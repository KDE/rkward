#include <QObject>
#include <QTest>
#include <QApplication>
#include <QFile>
#include <QDir>

#include <KAboutData>

#include "../debug.h"
#include "../rkward.h"
#include "../agents/rkquitagent.h"
#include "../rbackend/rksessionvars.h"
#include "../rbackend/rkrinterface.h"

void RKDebug (int, int, const char*, ...) {
	// disabled for now
}

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
    
    QPointer<RKWardMainWindow> main_win;
private slots:
	void init() {
	}
	void initTestCase()
	{
		qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox"); // Allow test to be run as root, which, for some reason is being done on the SuSE CI.
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
			qApp->processEvents(QEventLoop::AllEvents);
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

	void cleanupTestCase()
	{
		// at least the backend should exit properly, to avoid creating emergency save files
		RInterface::issueCommand(new RCommand(QString(), RCommand::QuitCommand));
		while (!(RInterface::instance()->backendIsDead())) {
			qApp->processEvents();
		}
	}
};

QTEST_MAIN(RKWardCoreTest)

#include "core_test.moc"
