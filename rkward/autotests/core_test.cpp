/*
core_test - This file is part of RKWard (https://rkward.kde.org). Created: Thu Jun 09 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
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

#include <KAboutData>

#include "../debug.h"
#include "../rkward.h"
#include "../agents/rkquitagent.h"
#include "../rbackend/rksessionvars.h"
#include "../rbackend/rkrinterface.h"
#include "../core/robject.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"

void RKDebug (int, int, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}

/** This test suite sets up a mostly complete application. That's a bit heavy, but arguably, a) modularity isn't ideal in RKWard, and b) many of the more interesting
 *  test involve passing commands to the R backend, and then verifying the expected state in the frontend. That alone requires a fairly extensive setup in the first place.
 *
 *  Since starting can still take several seconds, the plan, for now, is to run most tests most individual tests inside this single test suite. */
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
		while (!done && t.elapsed() < timeoutms) {
			qApp->processEvents();
		}
		if (!done) {
			qDebug("Command timed out: %s", qPrintable(ccopy));
			QFAIL("Command timed out");
		}
	}

	void runCommandAsync(RCommand *command, RCommandChain* chain, std::function<void(RCommand*)> callback) {
		command->whenFinished(this, callback);
		RInterface::issueCommand(command, chain);
	}

	void waitForAllFinished(int timeoutms = 1000) {
		runCommandWithTimeout(new RCommand(QString(), RCommand::App | RCommand::EmptyCommand), nullptr, [](RCommand*){}, timeoutms);
	}

	void cleanGlobalenv() {
		RInterface::issueCommand(new RCommand("rm(list=ls(all.names=TRUE))", RCommand::User));
	}
    
    QPointer<RKWardMainWindow> main_win;
private slots:
	void init() {
	}
	void cleanup() {
		waitForAllFinished();
	}
	void initTestCase()
	{
		qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox"); // Allow test to be run as root, which, for some reason is being done on the SuSE CI.
		QLoggingCategory::setFilterRules("qt.text.layout=false");  // Filter out some noise
		KAboutData::setApplicationData(KAboutData("rkward")); // needed for .rc files to load
		RK_Debug::RK_Debug_Level = DL_DEBUG;
		qDebug(R_EXECUTABLE);
		RKSessionVars::r_binary = R_EXECUTABLE;
		main_win = new RKWardMainWindow();
		main_win->testmode_suppress_dialogs = true;

		QElapsedTimer t;
		t.start();
		while (!(RInterface::instance()->backendIsDead() || RInterface::instance()->backendIsIdle())) {
			if (t.elapsed() > 20000) break;
			qApp->sendPostedEvents();
		}
		if (RInterface::instance()->backendIsIdle()) {
			qDebug("Startup completed");
		} else {
			qDebug("Startup failed. Listing contents of /tmp/rkward.rbackend");
			QFile f(QDir::tempPath() + "/rkward.rbackend");
			f.open(QIODevice::ReadOnly);
			auto output = f.readAll();
			qDebug("%s", output.data());
		}
	}

	void getIntVector() {
		auto c = new RCommand("c(1, 2, 3)", RCommand::GetIntVector);
		runCommandWithTimeout(c, nullptr, [](RCommand *command) {
			QCOMPARE(command->getDataType(), RData::IntVector);
			QCOMPARE(command->getDataLength(), 3);
			QCOMPARE(command->intVector().value(1), 2);
		});
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
			QVERIFY(a->isContainer());
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
			auto res = extractnumber.match(command->fullOutput());
			QVERIFY(res.hasMatch());
			output.append(res.captured());
		};

		runCommandAsync(new RCommand("cat(\"111\\n\")", RCommand::User), nullptr, callback);
		auto chain = RInterface::startChain();
		auto chain2 = RInterface::startChain(chain);
		runCommandAsync(new RCommand("message(\"222\\n\")", RCommand::App), chain, callback);
		runCommandAsync(new RCommand("stop(\"333\\n\")", RCommand::App), chain2, callback);
		runCommandAsync(new RCommand("warning(\"444\\n\")", RCommand::User), nullptr, callback);
		runCommandAsync(new RCommand("system(\"echo 555\")", RCommand::App), chain, callback);
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
		waitForAllFinished();  // priority_command_done must remain in scope until done
	}

	void cleanupTestCase()
	{
		// at least the backend should exit properly, to avoid creating emergency save files
		RInterface::issueCommand(new RCommand(QString(), RCommand::QuitCommand));
		RKWardMainWindow::getMain()->slotCloseAllWindows();
		while (!(RInterface::instance()->backendIsDead())) {
			qApp->processEvents();
		}
	}
};

QTEST_MAIN(RKWardCoreTest)

#include "core_test.moc"