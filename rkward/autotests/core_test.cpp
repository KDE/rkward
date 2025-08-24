/*
core_test - This file is part of RKWard (https://rkward.kde.org). Created: Thu Jun 09 2022
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTest>

#include <KAboutData>
#include <KActionCollection>
#include <KLocalizedString>

#include "../agents/rkquitagent.h"
#include "../core/renvironmentobject.h"
#include "../core/robject.h"
#include "../core/robjectlist.h"
#include "../debug.h"
#include "../misc/rkcommandlineargs.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkrapimenu.h"
#include "../misc/rkxmlguipreviewarea.h"
#include "../plugin/rkcomponentmap.h"
#include "../plugin/rkstandardcomponent.h"
#include "../rbackend/rkrinterface.h"
#include "../rbackend/rksessionvars.h"
#include "../rkconsole.h"
#include "../rkward.h"
#include "../settings/rksettings.h"
#include "../settings/rksettingsmodulekateplugins.h"
#include "../version.h"
#include "../windows/katepluginintegration.h"

QElapsedTimer _test_timer;

void testLog(const char *fmt, va_list args) {
	printf("%lld: ", _test_timer.elapsed());
	vprintf(fmt, args);
	printf("\n");
	fflush(stdout);
}

void testLog(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	testLog(fmt, ap);
	va_end(ap);
}

void RKDebug(int, int level, const char *fmt, ...) {
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
class RKWardCoreTest : public QObject {
	Q_OBJECT

	void runCommandWithTimeout(RCommand *command, RCommandChain *chain, std::function<void(RCommand *)> callback, int timeoutms = 1000) {
		static int done;
		QString ccopy = command->command();
		auto command_id = command->id();
		QElapsedTimer t;
		t.start();
		done = -1;
		int *_done = &done;
		connect(command->notifier(), &RCommandNotifier::commandFinished, this, [_done, command_id, callback](RCommand *command) { *_done = command_id; callback(command); });
		RInterface::issueCommand(command, chain);
		while ((done != command_id) && (t.elapsed() < timeoutms)) {
			qApp->processEvents(QEventLoop::AllEvents, 500);
		}
		if (done != command_id) {
			testLog("Command timed out: %s", qPrintable(ccopy));
			QFAIL("Command timed out");
		}
	}

	void runCommandAsync(RCommand *command, RCommandChain *chain, std::function<void(RCommand *)> callback) {
		command->whenFinished(this, callback);
		RInterface::issueCommand(command, chain);
	}

	void waitForAllFinished(int timeoutms = 2000) {
		runCommandWithTimeout(
		    new RCommand(QStringLiteral("# waitForAllFinished"), RCommand::App | RCommand::EmptyCommand | RCommand::Sync), nullptr, [](RCommand *) {}, timeoutms);
	}

	void cleanGlobalenv() {
		RInterface::issueCommand(new RCommand(QStringLiteral("rm(list=ls(all.names=TRUE))"), RCommand::User));
	}

	void listBackendLog() {
		testLog("Listing (new) contents of /tmp/rkward.rbackend");
		QByteArray output, oldoutput;
		QFile f(QDir::tempPath() + u"/rkward.rbackend"_s);
		if (f.open(QIODevice::ReadOnly)) {
			output = f.readAll();
			f.close();
		}

		QFile fl(QDir::tempPath() + u"/rkward.rbackend.listed"_s);
		if (fl.open(QIODevice::ReadOnly)) {
			oldoutput = fl.readAll();
			fl.close();
		}

		if (fl.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			fl.write(output);
			fl.close();
		}

		if (output.startsWith(oldoutput)) {
			output = output.sliced(oldoutput.length());
		}
		testLog("%s", qPrintable(QString::fromLocal8Bit(output)));
	}

	void waitForBackendStarted() {
		QElapsedTimer t;
		t.start();
		while (!(RInterface::instance()->backendIsDead() || RInterface::instance()->backendIsIdle())) {
			if (t.elapsed() > 40000) break;
			qApp->processEvents(QEventLoop::AllEvents, 500);
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
		if (RInterface::instance()->backendIsDead()) return QStringLiteral("dead");
		if (RInterface::instance()->backendIsIdle()) return QStringLiteral("idle");
		return QStringLiteral("busy");
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
		QStandardPaths::setTestModeEnabled(true);
		qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox"); // Allow test to be run as root, which, for some reason is being done on the SuSE CI.
		// qputenv("QT_LOGGING_RULES", "qt.qpa.windows.debug=true");  // Deliberately overwriting the rules set in the CI, as we are producing too much output, otherwise  -- TODO: does not appear to have any effect
		KLocalizedString::setApplicationDomain("rkward");
		KAboutData about(QStringLiteral("rkward"), QStringLiteral("RKWard"), QStringLiteral(RKWARD_VERSION), QStringLiteral("Frontend to the R statistics language"), KAboutLicense::GPL); // component name needed for .rc files to load
		KAboutData::setApplicationData(about);
		new RKCommandLineArgs(&about, qApp);
		RK_Debug::RK_Debug_Level = DL_DEBUG;
		testLog(R_EXECUTABLE);
		RKSessionVars::r_binary = QStringLiteral(R_EXECUTABLE);
		main_win = new RKWardMainWindow();
		main_win->testmode_suppress_dialogs = true;
		waitForBackendStarted();
	}

	void basicCheck() {
		// detect basic installation problems that are likely to cause (almost) everything else to fail
		QVERIFY(!RKCommonFunctions::getRKWardDataDir().isEmpty());
	}

	void getIntVector() {
		auto c = new RCommand(QStringLiteral("c(1, 2, 3)"), RCommand::GetIntVector | RCommand::App);
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
		QVERIFY(RObject::irregularShortName(u"0x"_s));
		QVERIFY(RObject::irregularShortName(u".1x"_s));
		QVERIFY(RObject::irregularShortName(u"_bla"_s));
		QVERIFY(RObject::irregularShortName(u"..."_s));
		QVERIFY(RObject::irregularShortName(u"b(la"_s));
		QVERIFY(!RObject::irregularShortName(u".x"_s));
		QVERIFY(!RObject::irregularShortName(u"..1x"_s));
		QVERIFY(!RObject::irregularShortName(u"x2"_s));
		QVERIFY(!RObject::irregularShortName(u"x_y"_s));
	}

	void objectListTest() {
		// check that resprentation a objects in backend is sane
		RInterface::issueCommand(QStringLiteral("a <- list(x1=c(1, 2, 3), x2=letters, x3=datasets::women); b <- a"), RCommand::User);
		RInterface::whenAllFinished(
		    this, []() {
			    auto a = RObjectList::getGlobalEnv()->findObject(QStringLiteral("a"));
			    QVERIFY(a != nullptr);
			    QVERIFY(a && a->isContainer());
			    auto ac = static_cast<RContainerObject *>(a);
			    QCOMPARE(ac->numChildren(), 3);
			    QCOMPARE(ac->findChildByIndex(0)->getDataType(), RObject::DataNumeric);
			    QCOMPARE(ac->findChildByIndex(1)->getDataType(), RObject::DataCharacter);
			    QVERIFY(ac->findChildByIndex(2)->isDataFrame());
		    },
		    nullptr);
		// check that changes are detected, and reflected, properly
		RInterface::issueCommand(QStringLiteral("rm(a); b <- 1; c <- letters; .d <- c"), RCommand::User);
		RInterface::whenAllFinished(
		    this, []() {
			    QVERIFY(RObjectList::getGlobalEnv()->findObject(u"a"_s) == nullptr);
			    QCOMPARE(RObjectList::getGlobalEnv()->findObject(u"b"_s)->getDataType(), RObject::DataNumeric);
			    QCOMPARE(RObjectList::getGlobalEnv()->findObject(u"c"_s)->getDataType(), RObject::DataCharacter);
			    QCOMPARE(RObjectList::getGlobalEnv()->findObject(u".d"_s)->getDimensions(), RObjectList::getGlobalEnv()->findObject(u"c"_s)->getDimensions());
		    },
		    nullptr);
		cleanGlobalenv();
		RInterface::whenAllFinished(this, [](RCommand *) {
			QCOMPARE(RObjectList::getGlobalEnv()->numChildren(), 0);
		});

		bool lock = true;
		runCommandAsync(new RCommand(QStringLiteral("dx <- data.frame(a=1:2, b=3:4)"), RCommand::User), nullptr, [this, &lock](RCommand *) {
			auto dx = RObjectList::getGlobalEnv()->findObject(QStringLiteral("dx"));
			QVERIFY(dx != nullptr);
			QVERIFY(dx && dx->isContainer());
			if (dx && dx->isContainer()) {
				auto dx_a = static_cast<RContainerObject *>(dx)->findChildByName(QStringLiteral("a"));
				QVERIFY(dx_a != nullptr);
				if (dx_a) {
					dx_a->rename(QStringLiteral("c"));
				}
				dx->rename(QStringLiteral("dy"));
			}
			auto c = new RCommand(QStringLiteral("dy$c"), RCommand::GetIntVector | RCommand::App);
			runCommandAsync(c, nullptr, [](RCommand *command) {
				QCOMPARE(command->getDataType(), RData::IntVector);
				QCOMPARE(command->getDataLength(), 2);
				QCOMPARE(command->intVector().value(1), 2);
			});
			lock = false;
		});
		while (lock)
			qApp->processEvents();

		// Saw a frontend crash on this idiom, once:
		RInterface::issueCommand(QStringLiteral("x <- list(NULL, 1)"), RCommand::User);
		RInterface::whenAllFinished(this, [](RCommand *) {
			QCOMPARE(RObjectList::getGlobalEnv()->findObject(u"x"_s)->getLength(), 2);
		});
		RInterface::issueCommand(QStringLiteral("x[[1]] <- NULL"), RCommand::User);
		RInterface::whenAllFinished(this, [](RCommand *) {
			QCOMPARE(RObjectList::getGlobalEnv()->findObject(u"x"_s)->getLength(), 1);
		});
		cleanGlobalenv();
	}

	void parseErrorTest() {
		runCommandWithTimeout(new RCommand(QStringLiteral("x <- "), RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(command->failed());
			QVERIFY(command->errorIncomplete());
		});
		runCommandWithTimeout(new RCommand(QStringLiteral("(}"), RCommand::App), nullptr, [](RCommand *command) {
			QVERIFY(command->failed());
			QVERIFY(command->errorSyntax());
		});
		runCommandWithTimeout(new RCommand(QStringLiteral("(}"), RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(command->failed());
			QEXPECT_FAIL("", "Syntax error detection for User commands known to be broken, but doesn't really matter", Continue);
			QVERIFY(command->errorSyntax());
		});
		runCommandWithTimeout(new RCommand(QStringLiteral("stop(\"123test\")"), RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(command->failed());
			QVERIFY(command->error().contains(u"123test"_s));
		});
		cleanGlobalenv();
	}

	void userCommandTest() {
		// Two commands submitted on one user line should both be run
		runCommandWithTimeout(new RCommand(QStringLiteral("print('first'); print('second')"), RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(!command->failed());
			QVERIFY(command->fullOutput().contains(u"first"_s));
			QVERIFY(command->fullOutput().contains(u"second"_s));
		});
		// Also, of course for commands on separate lines:
		runCommandWithTimeout(new RCommand(QStringLiteral("print('first')\nprint('second')"), RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(!command->failed());
			QVERIFY(command->fullOutput().contains(u"first"_s));
			QVERIFY(command->fullOutput().contains(u"second"_s));
		});
		// or multi-line commands:
		runCommandWithTimeout(new RCommand(QStringLiteral("{ print('first')\nprint('second') }"), RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(!command->failed());
			QVERIFY(command->fullOutput().contains(u"first"_s));
			QVERIFY(command->fullOutput().contains(u"second"_s));
		});
		// However, if a partial command fails, the next part should not get parsed:
		runCommandWithTimeout(new RCommand(QStringLiteral("stop('first'); print('second')"), RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(command->failed());
			QVERIFY(command->fullOutput().contains(u"first"_s));
			QVERIFY(!command->fullOutput().contains(u"second"_s));
		});
		// TODO: verify that calls to readline() and browser() are handled, correctly
	}

	void commandOrderAndOutputTest() {
		// commands shall run in the order 1, 3, 2, 5, 4, but also, of course, all different types of output shall be captured
		QStringList output;
		QRegularExpression extractnumber(QStringLiteral("\\d\\d\\d"));
		auto callback = [&output, extractnumber](RCommand *command) {
			auto res = extractnumber.match(command->fullOutput()); // clazy:exclude=use-static-qregularexpression - TODO: apparently false positive in clazy?
			QVERIFY(res.hasMatch());
			output.append(res.captured());
		};

		runCommandAsync(new RCommand(QStringLiteral("cat(\"111\\n\")"), RCommand::User), nullptr, callback);
		auto chain = RInterface::startChain();
		auto chain2 = RInterface::startChain(chain);
		runCommandAsync(new RCommand(QStringLiteral("message(\"222\\n\")"), RCommand::App), chain, callback);
		runCommandAsync(new RCommand(QStringLiteral("stop(\"333\\n\")"), RCommand::App), chain2, callback);
		runCommandAsync(new RCommand(QStringLiteral("warning(\"444\\n\")"), RCommand::User), nullptr, callback);
		runCommandAsync(new RCommand(QStringLiteral("if (.Platform$OS.type == \"unix\") system(\"echo 555\") else invisible(system(\"cmd /c echo 555\"))"), RCommand::App), chain, callback);
		RInterface::closeChain(chain);
		RInterface::closeChain(chain2);
		waitForAllFinished();

		QCOMPARE(output.size(), 5);
		QCOMPARE(output.value(0), u"111"_s);
		QCOMPARE(output.value(1), u"333"_s);
		QCOMPARE(output.value(2), u"222"_s);
		QCOMPARE(output.value(3), u"555"_s);
		QCOMPARE(output.value(4), u"444"_s);
	}

	void cancelCommandStressTest() {
		int cancelled_commands = 0;
		int commands_out = 0;
		for (int i = 0; i < 100; ++i) {
			runCommandAsync(new RCommand(QStringLiteral("Sys.sleep(.005)"), RCommand::User), nullptr, [&cancelled_commands, &commands_out](RCommand *command) {
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
		runCommandAsync(new RCommand(QStringLiteral("\ncat(\"sleeping\\n\"); Sys.sleep(5)"), RCommand::User), nullptr, [&priority_command_done](RCommand *command) {
			QVERIFY(priority_command_done);
			QVERIFY(command->failed());
			QVERIFY(command->wasCanceled());
		});
		auto priority_command = new RCommand(QStringLiteral("cat(\"priority\\n\")"), RCommand::PriorityCommand | RCommand::App);
		runCommandAsync(priority_command, nullptr, [&priority_command_done](RCommand *) {
			priority_command_done = true;
			RInterface::instance()->cancelAll();
		});
		// NOTE: The above two commands may or may not run in that order. Conceivably, the priority command gets handled, before the initial sleep command even started.
		//       The newline in the first command actually makes it a bit more likely for the priority command to go first (because parsing needs more iterations).
		//       We try to step on that interesting corner case, deliberately, at is has been causing failures in the past.
		waitForAllFinished();     // first wait with a short timeout: sleep should have been cancelled
		waitForAllFinished(5000); // fallbacck: priority_command_done must remain in scope until done (even if interrupting fails for some reason)
		                          // TODO: This test is still failing, occasionally, possibly, because the user command has not even been added to all_current_commands, yet
		                          //       (event processing in RKRBackend::handleRequest2())
	}

	void RKConsoleHistoryTest() {
		QTemporaryFile oldhist;
		QTemporaryFile emptyhist;
		emptyhist.open();
		emptyhist.close();
		RInterface::issueCommand(new RCommand(u"savehistory("_s + RObject::rQuote(oldhist.fileName()) + u"); loadhistory("_s + RObject::rQuote(emptyhist.fileName()) + u")"_s, RCommand::App));
		waitForAllFinished();

#define UNIQUE_STRING "unique_command_string"
		auto console = RKConsole::mainConsole();
		console->pipeUserCommand(QStringLiteral("if (FALSE) " UNIQUE_STRING "()"));
		runCommandWithTimeout(new RCommand(QStringLiteral("local({x <- tempfile(); savehistory(x); readLines(x)})"), RCommand::GetStringVector | RCommand::App), nullptr, [](RCommand *command) {
			QCOMPARE(command->stringVector().filter(QStringLiteral(UNIQUE_STRING)).size(), 1);
		});
		console->pipeUserCommand(QStringLiteral("timestamp(prefix=\"" UNIQUE_STRING "\")"));
		waitForAllFinished();
		QCOMPARE(console->commandHistory().filter(QStringLiteral(UNIQUE_STRING)).size(), 3);

		RInterface::issueCommand(new RCommand(u"loadhistory("_s + RObject::rQuote(oldhist.fileName()) + u")"_s, RCommand::App));
		waitForAllFinished();
	}

	void RKDeviceTest() {
		// Well, this test is sort of lame but should at least catch major breakage in RK() device
		runCommandAsync(new RCommand(QStringLiteral("demo(graphics, ask=FALSE)"), RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(!command->failed());
		});
		RInterface::issueCommand(new RCommand(QStringLiteral("dev.off()"), RCommand::User));
		waitForAllFinished(10000);
	}

	void HTMLWindowTest() {
		// this test, too, is extremely basic, but sometimes there are problems instantiating a QWebEnginePage
		runCommandAsync(new RCommand(QStringLiteral("?print"), RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(!command->failed());
			QCOMPARE(RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::HelpWindow).size(), 1);
			RKWorkplace::mainWorkplace()->closeAll(RKMDIWindow::HelpWindow);
			QCOMPARE(RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::HelpWindow).size(), 0);
		});
		RInterface::issueCommand(new RCommand(QStringLiteral("dev.off()"), RCommand::User));
		waitForAllFinished(5000);
	}

	void SettingsTest() {
		// Another very basic test. Essentially we check, whether the settings dialog can be created.
		QVERIFY(!RKSettings::settings_dialog);
		RKWorkplace::mainWorkplace()->openAnyUrl(QUrl(QStringLiteral("rkward://settings/plugins")));
		auto dialog = RKSettings::settings_dialog;
		QVERIFY(dialog);

		RKWorkplace::mainWorkplace()->openAnyUrl(QUrl(QStringLiteral("rkward://settings/graphics")));
		QVERIFY(dialog == RKSettings::settings_dialog); // shall be reused

		// Load and unload a bunch of plugins (settings dialog will be modified, but of course, also plugin loading/unloading is given another round
		// of testing.
		auto plugins = RKSettingsModuleKatePlugins::pluginsToLoad();
		QVERIFY(!plugins.isEmpty());
		for (int i = 0; i < plugins.size(); ++i) {
			auto plugins_b = plugins;
			plugins_b.removeAt(i);
			RKWardMainWindow::getMain()->katePluginIntegration()->loadPlugins(plugins_b);
			dialog->applyAll(); // to assertain there are no dead pointers in the dialog's internal bookkeeping
		}

		dialog->close();
		waitForAllFinished();
		QVERIFY(!RKSettings::settings_dialog);
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
		auto win = qobject_cast<RKCommandEditorWindow *>(wins[0]);
		QVERIFY(win == w);
		// opening the same url again shall re-use the window
		const auto w2 = RKWorkplace::mainWorkplace()->openScriptEditor(QUrl::fromLocalFile(f.fileName()));
		QVERIFY(win == w2);

		// pretty basic check: don't crash or assert on switching between previews
		// NOTE: first action is "no preview"
		auto modes = win->preview_modes->buttons();
		QVERIFY(modes.size() > 4);
		for (int i = modes.size() - 1; i >= 0; --i) {
			auto b = modes[i];
			if (b->isCheckable()) {
				qDebug("modebutton %s", qPrintable(b->text()));
				b->setChecked(true);
				QVERIFY(b->isChecked());
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
		RInterface::issueCommand(new RCommand(QStringLiteral("df <- data.frame('a'=letters, 'a'=letters, 'b'=letters, check.names=FALSE); df[[2, 'b']] <- list('x')"), RCommand::User));
		RInterface::issueCommand(new RCommand(QStringLiteral("rk.edit(df)"), RCommand::User));
		waitForAllFinished();

		// https://bugs.kde.org/show_bug.cgi?id=505955 : crash when changing type of edited column to unknown from R
		RInterface::issueCommand(new RCommand(QStringLiteral("df$c <- 1"), RCommand::User));
		waitForAllFinished();
		RInterface::issueCommand(new RCommand(QStringLiteral("df$c <- as.POSIXct.Date(1)"), RCommand::User)); // this one is current "unknown"
		waitForAllFinished();
		RInterface::issueCommand(new RCommand(QStringLiteral("df$c <- as.factor(\"a\")"), RCommand::User));
		waitForAllFinished();

		RKWardMainWindow::getMain()->slotCloseAllEditors();
	}

	void rkMenuTest() {
		const QStringList actionpath{u"analysis"_s, u"myaction"_s};
		RInterface::issueCommand(new RCommand(QStringLiteral("a <- rk.menu()"), RCommand::App));
		for (const auto &segment : actionpath) {
			RInterface::issueCommand(new RCommand(u"a <- a$item("_s + RObject::rQuote(segment) + u")"_s, RCommand::App));
		}
		RInterface::issueCommand(new RCommand(QStringLiteral("a$define('My Label', function() assign('x', 'actionval', envir=globalenv()))"), RCommand::User));
		waitForAllFinished();
		auto m = RKWardMainWindow::getMain()->rApiMenu();
		auto a = m->actionByPath(actionpath);
		QVERIFY(a);
		QVERIFY(a && a->text() == u"My Label"_s);
		QVERIFY(a && a->isEnabled());
		if (a) a->trigger();
		RInterface::issueCommand(new RCommand(QStringLiteral("a$enable(FALSE)"), RCommand::User));
		runCommandAsync(new RCommand(QStringLiteral("stopifnot(x == 'actionval')"), RCommand::App), nullptr, [](RCommand *c) {
			QVERIFY(c->succeeded());
		});
		waitForAllFinished();
		auto b = m->actionByPath(actionpath);
		QVERIFY(b);
		QVERIFY(b == a); // existing action should have been reused
		QVERIFY(b && (!b->isEnabled()));
		RInterface::issueCommand(new RCommand(QStringLiteral("a$remove()"), RCommand::App));
		RInterface::issueCommand(new RCommand(QStringLiteral("rm(x)"), RCommand::App));
		waitForAllFinished();
		QVERIFY(!m->actionByPath(actionpath));
	}

	void deviceHooksTest() {
		QString command = u"graphics.off()\n"_s
		                  "hook <- RK.addHook(\n"_s
		                  "  after.create=function(devnum, ...) {\n"_s
		                  "    print(paste0(\"New device \", devnum))\n"_s
		                  "  },\n"_s
		                  "  in.close=function(devnum, snapshot, ...) {\n"_s
		                  "    print(paste0(\"Closed device \", devnum))\n"_s
		                  "  },\n"_s
		                  "  in.blank=function(devnum, snapshot, ...) {\n"_s
		                  "    print(paste0(\"Blanking device \", devnum))\n"_s
		                  "  }\n"_s
		                  ")\n"_s

		                  "RK()\n"_s
		                  "plot(1, 1)\n"_s
		                  "plot(2, 2)\n"_s
		                  "rev <- RK.revision(dev.cur())\n"_s
		                  "title(\"Title\")\n"_s
		                  "if (RK.revision(dev.cur()) > rev) {\n"_s
		                  "  print(\"Plot was modified\")\n"_s
		                  "}\n"_s
		                  "dev.off()\n"_s
		                  "RK.removeHook(hook)\n"_s

		                  "plot(3, 3)\n"_s
		                  "dev.off()\n"_s;
		runCommandWithTimeout(new RCommand(command, RCommand::User), nullptr, [](RCommand *command) {
			QVERIFY(!command->failed());
			auto output = command->fullOutput();
			QCOMPARE(output.count(u"New device 2"_s), 1);
			QCOMPARE(output.count(u"Closed device 2"_s), 1);
			QCOMPARE(output.count(u"Plot was modified"_s), 1);
			QCOMPARE(output.count(u"Blanking device 2"_s), 2); // once each for the first two plots, but not for title()
		});
	}

	void restartRBackend() {
		RInterface::issueCommand(new RCommand(QStringLiteral("setwd(tempdir())"), RCommand::User)); // retart used to fail, if in non-existant directory
		RInterface::issueCommand(new RCommand(QStringLiteral("x <- 1"), RCommand::User));
		waitForAllFinished();
		QVERIFY(RObjectList::getGlobalEnv()->findObject(u"x"_s));

		QPointer<RInterface> oldiface = RInterface::instance();
		RKWardMainWindow::getMain()->triggerBackendRestart(false);
		QElapsedTimer t;
		t.start();
		while (oldiface) { // action may be delayed until next event processing
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
		QVERIFY(!RObjectList::getGlobalEnv()->findObject(u"x"_s));
		// but of course it should also be functional...
		RInterface::issueCommand(new RCommand(QStringLiteral("x <- 1"), RCommand::User));
		waitForAllFinished();
		QVERIFY(RObjectList::getGlobalEnv()->findObject(u"x"_s));
	}

	void switchComponentInterface() { // test for https://bugs.kde.org/show_bug.cgi?id=505364
		// this needs to be a plugin with both types of interface, *and* a preview defined from inside an embedded component
		auto handle = RKComponentMap::getComponentHandle(u"rkward::scatterplot"_s);
		QVERIFY(handle);
		auto ui = handle->invoke(nullptr, nullptr);
		QVERIFY(handle);
		ui->switchInterface(); // this used to crash
		ui->deleteLater();
	}

	void cleanupTestCase() {
		// at least the backend should exit properly, to avoid creating emergency save files
		RInterface::issueCommand(new RCommand(QStringLiteral("# Quit (test cleanup)"), RCommand::App | RCommand::EmptyCommand | RCommand::QuitCommand));
		RKWardMainWindow::getMain()->slotCloseAllWindows();
		while (!(RInterface::instance()->backendIsDead())) {
			qApp->processEvents();
		}
	}
};

QTEST_MAIN(RKWardCoreTest)

#include "core_test.moc"
