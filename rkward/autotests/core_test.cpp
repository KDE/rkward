/*
core_test - This file is part of RKWard (https://rkward.kde.org). Created: Thu Jun 09 2022
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QObject>
#include <QTest>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QActionGroup>

#include <KAboutData>
#include <KLocalizedString>
#include <KActionCollection>

#include "../debug.h"
#include "../rkward.h"
#include "../rkconsole.h"
#include "../version.h"
#include "../agents/rkquitagent.h"
#include "../rbackend/rksessionvars.h"
#include "../rbackend/rkrinterface.h"
#include "../core/robject.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkcommandlineargs.h"
#include "../misc/rkxmlguipreviewarea.h"

QElapsedTimer _test_timer;

void testLog(const char* fmt, va_list args) {
	printf("%lld: ", _test_timer.elapsed());
	vprintf(fmt, args);
	printf("\n");
	fflush(stdout);
}

void testLog(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	testLog(fmt, ap);
	va_end(ap);
}

void RKDebug (int, int level, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	testLog(fmt, ap);
	if (level >= DL_ERROR) QFAIL("error message during test (see above)");
	va_end(ap);
}

/** This test suite sets up a mostly complete application. That's a bit heavy, but arguably, a) modularity isn't ideal in RKWard, and b) many of the more interesting
 *  tests involve passing commands to the R backend, and then verifying the expected state in the frontend. That alone requires a fairly extensive setup, anyway.
 *
 *  Since starting can still take several seconds, the plan, for now, is to run most individual tests inside this single test suite. */
class RKWardCoreTest: public QObject {
	Q_OBJECT

	void runCommandWithTimeout(RCommand *command, RCommandChain* chain, std::function<void(RCommand*)> callback, int timeoutms = 1000) {
		QString ccopy = command->command();
		QElapsedTimer t;
		t.start();
		bool done = false;
		bool *_done = &done;
		connect(command->notifier(), &RCommandNotifier::commandFinished, this, [_done, callback](RCommand *command) { *_done = true; callback(command); });
		RInterface::issueCommand(command, chain);
		while (!done && (t.elapsed() < timeoutms)) {
			qApp->processEvents(QEventLoop::AllEvents, 500);
		}
		if (!done) {
			testLog("Command timed out: %s", qPrintable(ccopy));
			QFAIL("Command timed out");
		}
	}

	void runCommandAsync(RCommand *command, RCommandChain* chain, std::function<void(RCommand*)> callback) {
		command->whenFinished(this, callback);
		RInterface::issueCommand(command, chain);
	}

	void waitForAllFinished(int timeoutms = 2000) {
		runCommandWithTimeout(new RCommand("# waitForAllFinished", RCommand::App | RCommand::EmptyCommand | RCommand::Sync), nullptr, [](RCommand*){}, timeoutms);
	}

	void cleanGlobalenv() {
		RInterface::issueCommand(new RCommand("rm(list=ls(all.names=TRUE))", RCommand::User));
	}

	void listBackendLog() {
		testLog("Listing (new) contents of /tmp/rkward.rbackend");
		QByteArray output, oldoutput;
		QFile f(QDir::tempPath() + "/rkward.rbackend");
		if (f.open(QIODevice::ReadOnly)) {
			output = f.readAll();
			f.close();
		}

		QFile fl(QDir::tempPath() + "/rkward.rbackend.listed");
		if (fl.open(QIODevice::ReadOnly)) {
			oldoutput = fl.readAll();
			fl.close();
		}

		if (fl.open(QIODevice::ReadWrite | QIODevice::Truncate) ) {
			fl.write(output);
			fl.close();
		}

		if (output.startsWith(oldoutput)) {
			output = output.sliced(oldoutput.length());
		}
		testLog("%s", qPrintable(output.data()));
	}

	void waitForBackendStarted() {
		QElapsedTimer t;
		t.start();
		while (!(RInterface::instance()->backendIsDead() || RInterface::instance()->backendIsIdle())) {
			if (t.elapsed() > 40000) break;
			qApp->sendPostedEvents();
		}
		if (RInterface::instance()->backendIsIdle()) {
			testLog("Backend startup completed");
		} else {
			testLog("Backend startup failed");
			listBackendLog();
		}
	}

	QString backendStatus() {
		if (RInterface::instance()->backendIsDead()) return "dead";
		if (RInterface::instance()->backendIsIdle()) return "idle";
		return "busy";
	}
    
	QPointer<RKWardMainWindow> main_win;
private Q_SLOTS:
	void init() {
		testLog("Starting next test");
	}

	void cleanup() {
		testLog("Cleanup. Backend status: %s", qPrintable(backendStatus()));
		waitForAllFinished();
		listBackendLog();
		testLog("Cleanup done. Backend status: %s", qPrintable(backendStatus()));
	}

	void initTestCase() {
		_test_timer.start();
		qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox"); // Allow test to be run as root, which, for some reason is being done on the SuSE CI.
		// qputenv("QT_LOGGING_RULES", "qt.qpa.windows.debug=true");  // Deliberately overwriting the rules set in the CI, as we are producing too much output, otherwise  -- TODO: does not appear to have any effect
		KLocalizedString::setApplicationDomain("rkward");
		KAboutData about("rkward", "RKWard", RKWARD_VERSION, "Frontend to the R statistics language", KAboutLicense::GPL); // component name needed for .rc files to load
		KAboutData::setApplicationData(about);
		new RKCommandLineArgs(&about, qApp);
		RK_Debug::RK_Debug_Level = DL_DEBUG;
		testLog(R_EXECUTABLE);
		RKSessionVars::r_binary = R_EXECUTABLE;
		main_win = new RKWardMainWindow();
		main_win->testmode_suppress_dialogs = true;
		waitForBackendStarted();
	}

	void basicCheck() {
		// detect basic installation problems that are likely to cause (almost) everything else to fail
		QVERIFY(!RKCommonFunctions::getRKWardDataDir().isEmpty());
	}

	void getIntVector() {
		auto c = new RCommand("c(1, 2, 3)", RCommand::GetIntVector | RCommand::App);
		runCommandWithTimeout(c, nullptr, [](RCommand *command) {
			QCOMPARE(command->getDataType(), RData::IntVector);
			QCOMPARE(command->getDataLength(), 3);
			QCOMPARE(command->intVector().value(1), 2);
		});
	}

	void encodingTest() {
		QString test_string = QStringLiteral("français");
		RInterface::issueCommand(QStringLiteral("x <- ") + RObject::rQuote(test_string), RCommand::User);
		auto c = new RCommand(QStringLiteral("cat(x); y <- \"fran\\xE7ais\"; Encoding(y) <- \"latin1\"; list(x==y, x, y)"), RCommand::GetStructuredData | RCommand::App);
		runCommandWithTimeout(c, nullptr, [test_string](RCommand *command) {
			QCOMPARE(command->output(), test_string);
			QCOMPARE(command->getDataType(), RData::StructureVector);
			QCOMPARE(command->getDataLength(), 3);
			QVERIFY(command->structureVector().value(0)->intVector().value(0, 0) > 0);
			QCOMPARE(command->structureVector().value(1)->stringVector().value(0), test_string);
			QCOMPARE(command->structureVector().value(2)->stringVector().value(0), test_string);
		});
		// This one is to test that we don't screw up on strings exceeding the conversion buffer size
		c = new RCommand(QStringLiteral("x <- paste0(rep(x, 2000), collapse=\",\"); cat(x); x"), RCommand::GetStringVector | RCommand::App);
		runCommandWithTimeout(c, nullptr, [test_string](RCommand *command) {
			QCOMPARE(command->output().count(test_string), 2000);
			QCOMPARE(command->getDataType(), RData::StringVector);
			QCOMPARE(command->getDataLength(), 1);
			QCOMPARE(command->stringVector().value(0).count(test_string), 2000);
		});
		RInterface::issueCommand(QStringLiteral("rm(x); rm(y)"), RCommand::User);
	}

	void irregularShortNameTest() {
		QVERIFY(RObject::irregularShortName("0x"));
		QVERIFY(RObject::irregularShortName(".1x"));
		QVERIFY(RObject::irregularShortName("_bla"));
		QVERIFY(RObject::irregularShortName("..."));
		QVERIFY(RObject::irregularShortName("b(la"));
		QVERIFY(!RObject::irregularShortName(".x"));
		QVERIFY(!RObject::irregularShortName("..1x"));
		QVERIFY(!RObject::irregularShortName("x2"));
		QVERIFY(!RObject::irregularShortName("x_y"));
	}

	void objectListTest() {
		// check that resprentation a objects in backend is sane
		RInterface::issueCommand("a <- list(x1=c(1, 2, 3), x2=letters, x3=datasets::women); b <- a", RCommand::User);
		RInterface::whenAllFinished(this, []() {
			auto a = RObjectList::getGlobalEnv()->findObject("a");
			QVERIFY(a != nullptr);
			QVERIFY(a && a->isContainer());
			auto ac = static_cast<RContainerObject*>(a);
			QCOMPARE(ac->numChildren(), 3);
			QCOMPARE(ac->findChildByIndex(0)->getDataType(), RObject::DataNumeric);
			QCOMPARE(ac->findChildByIndex(1)->getDataType(), RObject::DataCharacter);
			QVERIFY(ac->findChildByIndex(2)->isDataFrame());
		}, nullptr);
		// check that changes are detected, and reflected, properly
		RInterface::issueCommand("rm(a); b <- 1; c <- letters; .d <- c", RCommand::User);
		RInterface::whenAllFinished(this, []() {
			QVERIFY(RObjectList::getGlobalEnv()->findObject("a") == nullptr);
			QCOMPARE(RObjectList::getGlobalEnv()->findObject("b")->getDataType(), RObject::DataNumeric);
			QCOMPARE(RObjectList::getGlobalEnv()->findObject("c")->getDataType(), RObject::DataCharacter);
			QCOMPARE(RObjectList::getGlobalEnv()->findObject(".d")->getDimensions(), RObjectList::getGlobalEnv()->findObject("c")->getDimensions());
		}, nullptr);
		cleanGlobalenv();
		RInterface::whenAllFinished(this, [](RCommand*) {
			QCOMPARE(RObjectList::getGlobalEnv()->numChildren(), 0);
		});

		bool lock = true;
		runCommandAsync(new RCommand("dx <- data.frame(a=1:2, b=3:4)", RCommand::User), nullptr, [this, &lock](RCommand *) {
			auto dx = RObjectList::getGlobalEnv()->findObject("dx");
			QVERIFY(dx != nullptr);
			QVERIFY(dx && dx->isContainer());
			if (dx && dx->isContainer()) {
			    auto dx_a = static_cast<RContainerObject*>(dx)->findChildByName("a");
			    QVERIFY(dx_a != nullptr);
			    if (dx_a) {
				dx_a->rename("c");
			    }
			    dx->rename("dy");
			}
			auto c = new RCommand("dy$c", RCommand::GetIntVector | RCommand::App);
			runCommandAsync(c, nullptr, [](RCommand *command) {
			    QCOMPARE(command->getDataType(), RData::IntVector);
			    QCOMPARE(command->getDataLength(), 2);
			    QCOMPARE(command->intVector().value(1), 2);
			});
			lock = false;
		});
		while(lock) qApp->processEvents();

		// Saw a frontend crash on this idiom, once:
		RInterface::issueCommand("x <- list(NULL, 1)", RCommand::User);
		RInterface::whenAllFinished(this, [](RCommand*) {
			QCOMPARE(RObjectList::getGlobalEnv()->findObject("x")->getLength(), 2);
		});
		RInterface::issueCommand("x[[1]] <- NULL", RCommand::User);
		RInterface::whenAllFinished(this, [](RCommand*) {
			QCOMPARE(RObjectList::getGlobalEnv()->findObject("x")->getLength(), 1);
		});
		cleanGlobalenv();
	}

	void parseErrorTest() {
		runCommandWithTimeout(new RCommand("x <- ", RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(command->failed());
			QVERIFY(command->errorIncomplete());
		});
		runCommandWithTimeout(new RCommand("(}", RCommand::App), nullptr, [](RCommand *command) {
			QVERIFY(command->failed());
			QVERIFY(command->errorSyntax());
		});
		runCommandWithTimeout(new RCommand("(}", RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(command->failed());
			QEXPECT_FAIL("", "Syntax error detection for User commands known to be broken, but doesn't really matter", Continue);
			QVERIFY(command->errorSyntax());
		});
		runCommandWithTimeout(new RCommand("stop(\"123test\")", RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(command->failed());
			QVERIFY(command->error().contains("123test"));
		});
		cleanGlobalenv();
	}

	void commandOrderAndOutputTest() {
		// commands shall run in the order 1, 3, 2, 5, 4, but also, of course, all different types of output shall be captured
		QStringList output;
		QRegularExpression extractnumber("\\d\\d\\d");
		auto callback = [&output, extractnumber](RCommand *command) {
			auto res = extractnumber.match(command->fullOutput()); // clazy:exclude=use-static-qregularexpression - TODO: apparently false positive in clazy?
			QVERIFY(res.hasMatch());
			output.append(res.captured());
		};

		runCommandAsync(new RCommand("cat(\"111\\n\")", RCommand::User), nullptr, callback);
		auto chain = RInterface::startChain();
		auto chain2 = RInterface::startChain(chain);
		runCommandAsync(new RCommand("message(\"222\\n\")", RCommand::App), chain, callback);
		runCommandAsync(new RCommand("stop(\"333\\n\")", RCommand::App), chain2, callback);
		runCommandAsync(new RCommand("warning(\"444\\n\")", RCommand::User), nullptr, callback);
		runCommandAsync(new RCommand("if (.Platform$OS.type == \"unix\") system(\"echo 555\") else invisible(system(\"cmd /c echo 555\"))", RCommand::App), chain, callback);
		RInterface::closeChain(chain);
		RInterface::closeChain(chain2);
		waitForAllFinished();

		QCOMPARE(output.size(), 5);
		QCOMPARE(output.value(0), "111");
		QCOMPARE(output.value(1), "333");
		QCOMPARE(output.value(2), "222");
		QCOMPARE(output.value(3), "555");
		QCOMPARE(output.value(4), "444");
	}

	void cancelCommandStressTest() {
		int cancelled_commands = 0;
		int commands_out = 0;
		for (int i = 0; i < 100; ++i) {
			runCommandAsync(new RCommand("Sys.sleep(.005)", RCommand::User), nullptr, [&cancelled_commands, &commands_out](RCommand *command) {
				if (command->wasCanceled()) cancelled_commands++;
				commands_out++;
			});
			// We want to cover various cases, here, including cancelling commands before and after they have been sent to the backend, but also at least some commands that finish
			// without being effictively cancelled.
			if (i % 4 == 0) {
				RInterface::instance()->cancelAll();
			} else if (i % 4 == 1) {
				QElapsedTimer t;
				t.start();
				while (commands_out <= i) {
					qApp->processEvents(QEventLoop::AllEvents, 500);
					if (t.elapsed() > 10000) {
						testLog("Timeout waiting for backend");
						listBackendLog();
						QFAIL("Timeout waiting for backend");
						break;
					}
				}
			} else if (i % 4 == 2) {
				qApp->processEvents();
			}
		}
		waitForAllFinished();
		// The point of this test case is to make sure, we do not get into a deadlock, however, the QVERIFYs below are to make sure the test itself behaves as expected.
		// There needs to be some wiggle room, however, as this is inherently prone to race-conditions. (Commands finish running before getting cancelled, or they don't).
		QVERIFY(cancelled_commands >= 25);
		QVERIFY(cancelled_commands <= 75);
		testLog("%d out of %d commands were actually cancelled", cancelled_commands, commands_out);
	}

	void priorityCommandTest() {
		bool priority_command_done = false;
		runCommandAsync(new RCommand("Sys.sleep(5)", RCommand::User), nullptr, [&priority_command_done](RCommand *command) {
			QVERIFY(priority_command_done);
			QVERIFY(command->failed());
			QVERIFY(command->wasCanceled());
		});
		auto priority_command = new RCommand("cat(\"something\\n\")", RCommand::PriorityCommand | RCommand::App);
		runCommandAsync(priority_command, nullptr, [&priority_command_done](RCommand *) {
			priority_command_done = true;
			RInterface::instance()->cancelAll();
		});
		waitForAllFinished();
		waitForAllFinished(4000);  // priority_command_done must remain in scope until done (even if interrupting fails for some reason)
	}

	void RKConsoleHistoryTest() {
		QTemporaryFile oldhist;
		QTemporaryFile emptyhist;
		emptyhist.open();
		emptyhist.close();
		RInterface::issueCommand(new RCommand("savehistory(" + RObject::rQuote(oldhist.fileName()) + "); loadhistory(" + RObject::rQuote(emptyhist.fileName()) + ")", RCommand::App));
		waitForAllFinished();

#		define UNIQUE_STRING "unique_command_string"
		auto console = RKConsole::mainConsole();
		console->pipeUserCommand("if (FALSE) " UNIQUE_STRING "()");
		runCommandWithTimeout(new RCommand("local({x <- tempfile(); savehistory(x); readLines(x)})", RCommand::GetStringVector | RCommand::App), nullptr, [](RCommand *command) {
			QCOMPARE(command->stringVector().filter(UNIQUE_STRING).size(), 1);
		});
		console->pipeUserCommand("timestamp(prefix=\"" UNIQUE_STRING "\")");
		waitForAllFinished();
		QCOMPARE(console->commandHistory().filter(UNIQUE_STRING).size(), 3);

		RInterface::issueCommand(new RCommand("loadhistory(" + RObject::rQuote(oldhist.fileName()) + ")", RCommand::App));
		waitForAllFinished();
	}

	void RKDeviceTest() {
		// Well, this test is sort of lame but should at least catch major breakage in RK() device
		runCommandAsync(new RCommand("demo(graphics, ask=FALSE)", RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(!command->failed());
		});
		RInterface::issueCommand(new RCommand("dev.off()", RCommand::User));
		waitForAllFinished(10000);
	}

	void HTMLWindowTest() {
		// this test, too, is extremely basic, but sometimes there are problems instantiating a QWebEnginePage
		runCommandAsync(new RCommand("?print", RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(!command->failed());
			QCOMPARE(RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::HelpWindow).size(), 1);
			RKWorkplace::mainWorkplace()->closeAll(RKMDIWindow::HelpWindow);
			QCOMPARE(RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::HelpWindow).size(), 0);
		});
		RInterface::issueCommand(new RCommand("dev.off()", RCommand::User));
		waitForAllFinished(5000);  // priority_command_done must remain in scope until done
	}

	void ScriptWindowTest() {
		QCOMPARE(RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow).size(), 0);

		// pretty basic check: don't crash or assert on opening script window
		QTemporaryFile f;
		f.open();
		f.write("plot(1,1)\n"); // Using a plot(), here is interesting in that it a) allows a plot preview b) a plot will also be generated, and
		                        // immediately discarded for R console previews, which used to be prone to crashing
		f.close();
		const auto w = RKWorkplace::mainWorkplace()->openScriptEditor(QUrl::fromLocalFile(f.fileName()));
		QVERIFY(w != nullptr);
		auto wins = RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow);
		QCOMPARE(wins.size(), 1);
		auto win = qobject_cast<RKCommandEditorWindow*>(wins[0]);
		QVERIFY(win == w);
		// opening the same url again shall re-use the window
		const auto w2 = RKWorkplace::mainWorkplace()->openScriptEditor(QUrl::fromLocalFile(f.fileName()));
		QVERIFY(win == w2);

		// pretty basic check: don't crash or assert on switching between previews
		// NOTE: first action is "no preview"
		auto actions = win->preview_modes->actions();
		QVERIFY(actions.size() > 4);
		for (int i = actions.size() - 1; i >= 0; --i) {
			auto a = actions[i];
			if (a->isCheckable()) {
				qDebug("action %s", qPrintable(a->text()));
				a->trigger();  // NOTE: Using setChecked(true), here, would not emit the require QActionGroup::triggered() inside RKCommandEditorWindow
				QVERIFY(a->isChecked());
				win->doRenderPreview(); // don't wait for debounce timeout
				waitForAllFinished(8000);
				// TODO: check that a preview was actually generated
			}
		}
		win->close(RKMDIWindow::NoAskSaveModified);
		waitForAllFinished();
	}

	void dataEditorTest() {
		// Create a quirky data.frame, intentionally. Goal is not to crash ;-)
		RInterface::issueCommand(new RCommand("df <- data.frame('a'=letters, 'a'=letters, 'b'=letters, check.names=FALSE); df[[2, 'b']] <- list('x')", RCommand::User));
		RInterface::issueCommand(new RCommand("rk.edit(df)", RCommand::User));
		waitForAllFinished();
		RKWardMainWindow::getMain()->slotCloseAllEditors();
	}

	void restartRBackend() {
		RInterface::issueCommand(new RCommand("setwd(tempdir())", RCommand::User)); // retart used to fail, if in non-existant directory
		RInterface::issueCommand(new RCommand("x <- 1", RCommand::User));
		waitForAllFinished();
		QVERIFY(RObjectList::getGlobalEnv()->findObject("x"));

		QPointer<RInterface> oldiface = RInterface::instance();
		RKWardMainWindow::getMain()->triggerBackendRestart(false);
		QElapsedTimer t;
		t.start();
		while (oldiface) {  // action may be delayed until next event processing
			qApp->processEvents(QEventLoop::AllEvents, 500);
			if (t.elapsed() > 30000) {
				testLog("Backend shutdown timed out");
				auto m = QApplication::activeModalWidget();
				testLog("Active modal window (if any): %p (%s)", m, qPrintable(m ? m->windowTitle() : QString()));
				break;
			}
		}
		testLog("Backend is restarting");
		waitForBackendStarted();

		// backend should be clean after restart
		QVERIFY(!RObjectList::getGlobalEnv()->findObject("x"));
		// but of course it should also be functional...
		RInterface::issueCommand(new RCommand("x <- 1", RCommand::User));
		waitForAllFinished();
		QVERIFY(RObjectList::getGlobalEnv()->findObject("x"));
	}

	void cleanupTestCase()
	{
		// at least the backend should exit properly, to avoid creating emergency save files
		RInterface::issueCommand(new RCommand(QString("# Quit (test cleanup)"), RCommand::App | RCommand::EmptyCommand | RCommand::QuitCommand));
		RKWardMainWindow::getMain()->slotCloseAllWindows();
		while (!(RInterface::instance()->backendIsDead())) {
			qApp->processEvents();
		}
	}
};

QTEST_MAIN(RKWardCoreTest)

#include "core_test.moc"
