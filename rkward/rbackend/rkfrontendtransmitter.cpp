/*
rkfrontendtransmitter - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 04 2010
SPDX-FileCopyrightText: 2010-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkfrontendtransmitter.h"

#include "../misc/rkcommandlineargs.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettingsmoduler.h"
#include "rkrbackendprotocol_frontend.h"
#include "rksessionvars.h"
#include "rkwarddevice/rkgraphicsdevice_frontendtransmitter.h"

#include <KLocalizedString>
#include <krandom.h>

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "../debug.h"
#include "../version.h"

QString findBackendAtPath(const QString &path) {
	QDir dir(path);
	dir.makeAbsolute();
#ifdef Q_OS_WIN
	QString ret = dir.filePath(u"rkward.rbackend.exe"_s);
#else
	QString ret = dir.filePath(u"rkward.rbackend"_s);
#endif
	RK_DEBUG(RBACKEND, DL_DEBUG, "Looking for backend at %s", qPrintable(ret));
	QFileInfo fi(ret);
	if (fi.exists() && fi.isExecutable()) return ret;
	return QString();
}

#if defined(RK_DLOPEN_LIBRSO)
QString findBackendLibAtPath(const QString &path) {
	QDir dir(path);
	dir.makeAbsolute();
#	ifdef Q_OS_WIN
	QString ret = dir.filePath(u"rkward.rbackend.lib.dll"_s);
#	elif defined(Q_OS_MACOS)
	QString ret = dir.filePath(u"librkward.rbackend.lib.dylib"_s);
#	else
	QString ret = dir.filePath(u"librkward.rbackend.lib.so"_s);
#	endif
	RK_DEBUG(RBACKEND, DL_DEBUG, "Looking for backend lib at %s", qPrintable(ret));
	if (QFileInfo::exists(ret)) return ret;
	return QString();
}
#endif

bool pathIsChildOf(const QString &parent, const QString &child) {
	return QFileInfo(child).canonicalFilePath().startsWith(QFileInfo(parent).canonicalFilePath());
}

void removeFromPathList(const char *varname, bool (*shouldRemove)(const QString &path)) {
#ifdef Q_OS_WIN
#	define PATH_VAR_SEP u';'
#else
#	define PATH_VAR_SEP u':'
#endif
	auto var = qgetenv(varname);
	if (var.isEmpty()) return;

	const auto list = QString::fromLocal8Bit(var).split(PATH_VAR_SEP);
	QStringList newlist;
	for (const auto &str : list) {
		if (shouldRemove(str)) {
			RK_DEBUG(RBACKEND, DL_DEBUG, "Removing path %s from $%s", qPrintable(str), varname);
		} else {
			newlist.append(str);
		}
	}
	qputenv(varname, newlist.join(PATH_VAR_SEP).toLocal8Bit());
}

RKFrontendTransmitter::RKFrontendTransmitter(RKRBackendProtocolFrontend *frontend) : RKAbstractTransmitter(), frontend(frontend) {
	RK_TRACE(RBACKEND);

	rkd_transmitter = new RKGraphicsDeviceFrontendTransmitter();
	quirkmode = false;
	start();
}

RKFrontendTransmitter::~RKFrontendTransmitter() {
	RK_TRACE(RBACKEND);

	delete rkd_transmitter;
}

QString localeDir() {
	// adapted from KCatalog::catalogLocaleDir()
	QString relpath = QStringLiteral("%1/LC_MESSAGES/rkward.mo").arg(QLocale().name().section(u'_', 0, 0));
	QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("locale/") + relpath);
	if (file.isEmpty()) return QString();
	return QFileInfo(file.left(file.size() - relpath.size())).absolutePath();
}

/** Check if the given path to R (or "auto") is executable, and fail with an appropriate message, otherwise. If "auto" is given as input, try to auto-locate an R installation at the standard
installation path(s) for this platform. */
QString RKFrontendTransmitter::resolveRSpecOrFail(QString input) {
	if (input == QLatin1String("auto")) {
		QString ret = RKSessionVars::findRInstallations().value(0);

		if (ret.isNull() || !QFileInfo(ret).isExecutable()) {
			handleTransmissionError(i18n("RKWard failed to detect an R installation on this system. Either R is not installed, or not at one of the standard installation locations."));
		}
		RK_DEBUG(APP, DL_DEBUG, "Using auto-detected R at %s", qPrintable(ret));
		return ret;
	}

	if (!QFileInfo(input).isExecutable()) {
		handleTransmissionError(i18n("The configured R installation at <b>%1</b> does not exist or is not an executable.", input));
	}
	return input;
}

void RKFrontendTransmitter::detectAndCheckRBinary() {
	if (!RKSessionVars::RBinary().isEmpty()) return;

	// Look for R:
	//- command line parameter
	//- Specified in cfg file next to rkward executable
	//- compile-time default
	QString r_exe = RKCommandLineArgs::get(RKCommandLineArgs::RExecutable).toString();
	if (!r_exe.isEmpty()) {
		RK_DEBUG(APP, DL_DEBUG, "Using R as specified on command line");
	} else if (!RKSettingsModuleR::userConfiguredRBinary().isEmpty()) {
		RK_DEBUG(APP, DL_DEBUG, "Using R as previously user-configured");
		r_exe = RKSettingsModuleR::userConfiguredRBinary();
	} else {
		QDir frontend_path = qApp->applicationDirPath();
		QFileInfo rkward_ini_file(frontend_path.absoluteFilePath(QStringLiteral("rkward.ini")));
		if (rkward_ini_file.isReadable()) {
			QSettings rkward_ini(rkward_ini_file.absoluteFilePath(), QSettings::IniFormat);
			r_exe = rkward_ini.value("R executable").toString();
			if (!r_exe.isNull()) {
				if (QDir::isRelativePath(r_exe) && r_exe != QStringLiteral("auto")) {
					r_exe = frontend_path.absoluteFilePath(r_exe);
				}
			}
			RK_DEBUG(APP, DL_DEBUG, "Using R as configured in config file %s", qPrintable(rkward_ini_file.absoluteFilePath()));
		} else {
			if (QFileInfo::exists(QStringLiteral(R_EXECUTABLE))) {
				RK_DEBUG(APP, DL_DEBUG, "Using R as configured at compile time");
				r_exe = QStringLiteral(R_EXECUTABLE);
			}
		}
	}
	if (r_exe.isEmpty()) {
		RK_DEBUG(APP, DL_DEBUG, "Falling back to auto-detection of R binary");
		r_exe = QStringLiteral("auto");
	}

	RKSessionVars::r_binary = resolveRSpecOrFail(r_exe);
}

void RKFrontendTransmitter::run() {
	RK_TRACE(RBACKEND);

	quirkmode = RKCommandLineArgs::get(RKCommandLineArgs::QuirkMode).toBool();
	detectAndCheckRBinary();

	// start server
	server = new QLocalServer(this);
	// we add a bit of randomness to the servername, as in general the servername must be unique
	// there could be conflicts with concurrent or with previous crashed rkward sessions.
	if (!server->listen(u"rkward"_s + KRandom::randomString(8))) handleTransmissionError(u"Failure to start frontend server: "_s + server->errorString());
	connect(server, &QLocalServer::newConnection, this, &RKFrontendTransmitter::connectAndEnterLoop, Qt::QueuedConnection);
	if (quirkmode) server->setSocketOptions(QLocalServer::UserAccessOption);
	// start backend
	backend = new QProcess(this);
	connect(backend, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &RKFrontendTransmitter::backendExit);

	QStringList env = QProcess::systemEnvironment();
	// Try to synchronize language selection in frontend and backend
	int index = env.indexOf(QRegularExpression(QStringLiteral("^LANGUAGE=.*"), QRegularExpression::CaseInsensitiveOption)); // clazy:exclude=use-static-qregularexpression ; it'll only be craeted once
	if (index >= 0) env.removeAt(index);
	env.append(u"LANGUAGE="_s + QLocale().name().section(u'_', 0, 0));

	if (RKSessionVars::runningInAppImage() && RKSessionVars::isPathInAppImage(RKSessionVars::RBinary())) {
		RK_DEBUG(RBACKEND, DL_DEBUG, "Detected running from AppImage with external R.");
		removeFromPathList("LD_LIBRARY_PATH", RKSessionVars::isPathInAppImage);
		removeFromPathList("PATH", RKSessionVars::isPathInAppImage);
	}

	QStringList args;
	args.append(QStringLiteral("CMD"));
	QString debugger = RKCommandLineArgs::get(RKCommandLineArgs::BackendDebugger).toString();
	if (!debugger.isEmpty()) {
		args += debugger.split(u' ');
	}

	QString backend_executable = findBackendAtPath(QCoreApplication::applicationDirPath());
#ifdef Q_OS_MACOS
	if (backend_executable.isEmpty())
		backend_executable =
		    findBackendAtPath(QCoreApplication::applicationDirPath() + u"/../Resources"_s); // an appropriate location in a standalone app-bundle
#endif
	if (backend_executable.isEmpty())
		backend_executable = findBackendAtPath(QCoreApplication::applicationDirPath() + u"/rbackend"_s); // for running directly from the build-dir
	if (backend_executable.isEmpty())
		backend_executable = findBackendAtPath(QCoreApplication::applicationDirPath() + u"/../rbackend"_s); // for running directly from the build-test-dir
	if (backend_executable.isEmpty())
		backend_executable =
		    findBackendAtPath(QCoreApplication::applicationDirPath() + u"/"_s + QStringLiteral(REL_PATH_TO_LIBEXEC)); // as calculated from cmake
#ifdef Q_OS_MACOS
	if (backend_executable.isEmpty()) backend_executable = findBackendAtPath(QCoreApplication::applicationDirPath() + u"/../../../rbackend"_s);
	if (backend_executable.isEmpty())
		backend_executable =
		    findBackendAtPath(QCoreApplication::applicationDirPath() + u"/../Frameworks/libexec"_s); // For running from .dmg created by craft --package rkward
#endif
	if (backend_executable.isEmpty()) backend_executable = findBackendAtPath(QStringLiteral(RKWARD_BACKEND_PATH));
	if (backend_executable.isEmpty()) {
		handleTransmissionError(i18n("The backend executable could not be found. This is likely to be a problem with your installation."));
		exec(); // To actually show the transmission error
		return;
	}

#if defined(RK_DLOPEN_LIBRSO)
	/** NOTE: For a description of the rationale for this involved loading procedure rkapi.h ! */
	QString backend_lib = findBackendLibAtPath(QCoreApplication::applicationDirPath()); // for running directly from the build tree, but also covers windows
	if (backend_lib.isEmpty())
		backend_lib = findBackendLibAtPath(QCoreApplication::applicationDirPath() + u"/"_s + QStringLiteral(REL_PATH_TO_LIB)); // regular installation; rel path between bin and lib dir is calculated in cmake
	if (backend_lib.isEmpty())
		backend_lib = findBackendLibAtPath(QFileInfo(backend_executable).absolutePath()); // backend and lib both installed in libexec or similar
#	if defined(Q_OS_MACOS)
	if (backend_lib.isEmpty())
		backend_lib = findBackendLibAtPath(QCoreApplication::applicationDirPath() + u"/../Frameworks"_s); // MacOS bundle: rkward in /MacOS, lib in /Framweorks
#	endif
#	if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	env.append(QStringLiteral("RK_BACKEND_LIB=") + backend_lib);
#	else
	env.append(QStringLiteral("RK_BACKEND_LIB=") + QFileInfo(backend_lib).fileName());
	QTemporaryDir rkward_only_dir(QDir::tempPath() + u"/rkward_only"_s);
	QFile(QFileInfo(backend_lib).absolutePath()).link(rkward_only_dir.filePath(QStringLiteral("_rkward_only_dlpath")));
	env.append(QStringLiteral("RK_ADD_LDPATH=./_rkward_only_dlpath"));
	env.append(QStringLiteral("RK_LD_CWD=") + rkward_only_dir.path());
#	endif
#endif
	backend->setEnvironment(env);

#ifndef Q_OS_MACOS
	// Needed on for paths with spaces. R CMD is too simple to deal with those, even if we provide proper quoting.
	// So rather we need to work from a relative path with all spaces eliminated.
	// However, also, we might start in directory that no longer exists, or will cease to exist, in a second
	// (e.g. the tempdir() of the previous R session). So we set something known to exist, here.
	// On MacOS, we cd to R_HOME, instead, below
	QFileInfo bfi(backend_executable);
	backend->setWorkingDirectory(bfi.absolutePath());
#	ifdef Q_OS_WIN
	args.append(bfi.fileName());
#	else
	args.append(u"./"_s + bfi.fileName());
#	endif
#else
	args.append(backend_executable);
#endif

	if (RKSettingsModuleGeneral::rkwardVersionChanged()) args.append(QStringLiteral("--setup"));
	args.append(QStringLiteral("--debug-level=") + QString::number(RK_Debug::RK_Debug_Level));
	// NOTE: QProcess quotes its arguments, *but* properly passing all spaces and quotes through the R CMD wrapper, seems near(?) impossible on Windows.
	// Instead, we use percent encoding, internally.
	args.append(QStringLiteral("--server-name=") + QString::fromLatin1(server->fullServerName().toUtf8().toPercentEncoding()));
	args.append(QStringLiteral("--rkd-server-name=") + QString::fromLatin1(rkd_transmitter->serverName().toUtf8().toPercentEncoding()));
	args.append(QStringLiteral("--data-dir=") + QString::fromLatin1(RKSettingsModuleGeneral::filesPath().toUtf8().toPercentEncoding()));
	args.append(QStringLiteral("--locale-dir=") + QString::fromLatin1(localeDir().toUtf8().toPercentEncoding()));
	RK_DO(
	    {
		    RK_DEBUG(RBACKEND, DL_DEBUG, "R binary: %s", qPrintable(RKSessionVars::RBinary()));
		    RK_DEBUG(RBACKEND, DL_DEBUG, "%s", qPrintable(args.join(u"\n"_s)));
	    },
	    RBACKEND, DL_DEBUG);

#ifdef Q_OS_MACOS
	// Resolving libR.dylib and friends is a pain on MacOS, and running through R CMD does not always seem to be enough.
	// (Apparently DYLIB_FALLBACK_LIBRARY_PATH is ignored on newer versions of MacOS). Safest best seems to be to start in the lib directory, itself.
	QProcess dummy;
	dummy.start(RKSessionVars::RBinary(), QStringList() << u"--slave"_s
	                                                    << u"--no-save"_s
	                                                    << u"--no-init-file"_s
	                                                    << u"-e"_s
	                                                    << u"cat(R.home('lib'))"_s);
	dummy.waitForFinished();
	QString r_home = QString::fromLocal8Bit(dummy.readAllStandardOutput());
	RK_DEBUG(RBACKEND, DL_INFO, "Setting working directory to %s", qPrintable(r_home));
	backend->setWorkingDirectory(r_home);
#endif
#if defined(Q_OS_WIN)
	// On some windows systems, the _first_ invocation of the backend seems to fail as somehow process output from the backend (the token) never arrives.
	// What appears to function as a workaround is start a dummy process, before that.
	QProcess dummy;
	QStringList dummyargs = args;
	dummy.setWorkingDirectory(backend->workingDirectory());
	dummyargs.removeAt(dummyargs.size() - 4); // the --server-name. With this empty, the backend will exit
	dummy.start(RKSessionVars::RBinary(), dummyargs, QIODevice::ReadOnly);
	dummy.waitForFinished();
	dummy.readAllStandardOutput();
#endif
	RK_DEBUG(RBACKEND, DL_DEBUG, "Starting backend. Timestamp %d", QDateTime::currentMSecsSinceEpoch(), token.length());
	if (quirkmode) {
		backend->setProgram(RKSessionVars::RBinary());
		backend->setArguments(args);
		backend->startDetached();
	} else {
		backend->start(RKSessionVars::RBinary(), args, QIODevice::ReadOnly);

		if (!backend->waitForStarted()) {
			handleTransmissionError(i18n("The backend executable could not be started. Error message was: %1", backend->errorString()));
		} else {
			token = waitReadLine(backend, 5000).trimmed();
			RK_DEBUG(RBACKEND, DL_DEBUG, "Now closing stdio channels");
			backend->closeReadChannel(QProcess::StandardError);
			backend->closeReadChannel(QProcess::StandardOutput);
		}
	}
	RK_DEBUG(RBACKEND, DL_DEBUG, "Startup done at %d. Received token length was %d", QDateTime::currentMSecsSinceEpoch(), token.length());

	exec();

	if (!quirkmode) {
		// It's ok to only give backend a short time to finish. We only get here, after QuitCommand has been handled by the backend
		if (!backend->waitForFinished(1000)) backend->close();
	}

	if (!connection) {
		RK_ASSERT(false);
	}
	RK_ASSERT(!server->isListening());
	delete server;
	delete backend;
}

QString RKFrontendTransmitter::waitReadLine(QIODevice *con, int msecs) {
	RK_TRACE(RBACKEND);

	// NOTE: On Qt5+Windows, readyReady may actually come in char by char, so calling waitForReadyRead() does not guarantee we will
	//       see the full line, at all. But also, of course, we want to put some cap on trying. Using a time threshold for this.
	QElapsedTimer time;
	time.start();
	QByteArray ret;
	do {
		ret.append(con->readLine());
		if (ret.contains('\n')) break;
		con->waitForReadyRead(500);
	} while (time.elapsed() < msecs);
	return QString::fromLocal8Bit(ret);
}

void RKFrontendTransmitter::connectAndEnterLoop() {
	RK_TRACE(RBACKEND);
	RK_ASSERT(server->hasPendingConnections());

	QLocalSocket *con = server->nextPendingConnection();
	server->close();

	// handshake
	QString token_c = waitReadLine(con, 1000).trimmed();
	if (quirkmode) {
		token = token_c;
	} else {
		if (token_c != token)
			handleTransmissionError(i18n("Error during handshake with backend process. Expected token '%1', received token '%2'", token, token_c));
	}
	QString version_c = waitReadLine(con, 1000).trimmed();
	if (version_c != QStringLiteral(RKWARD_VERSION))
		handleTransmissionError(
		    i18n("Version mismatch during handshake with backend process. Frontend is version '%1' while backend is '%2'.\nPlease fix your installation.",
		         QStringLiteral(RKWARD_VERSION), version_c));

	setConnection(con);
}

void RKFrontendTransmitter::requestReceived(RBackendRequest *request) {
	RK_TRACE(RBACKEND);

	if (request->type == RBackendRequest::Output) {
		ROutputList *list = request->output;
		for (int i = 0; i < list->size(); ++i) {
			ROutput *out = (*list)[i];

			if (handleOutput(out->output, out->output.length(), out->type)) {
				RKRBackendEvent *event = new RKRBackendEvent(new RBackendRequest(false, RBackendRequest::OutputStartedNotification));
				qApp->postEvent(frontend, event);
			}

			delete (out);
		}
		delete list;
		request->output = nullptr;
		RK_ASSERT(request->synchronous);
		writeRequest(request); // to tell the backend, that we are keeping up. Also deletes the request.
		return;
	}

	RKRBackendEvent *event = new RKRBackendEvent(request);
	qApp->postEvent(frontend, event);
}

void RKFrontendTransmitter::backendExit(int exitcode) {
	RK_TRACE(RBACKEND);

	if (!exitcode && token.isEmpty()) handleTransmissionError(i18n("The backend process could not be started. Please check your installation."));
	else if (token.isEmpty()) handleTransmissionError(i18n("The backend process failed to start with exit code %1, message: '%2'.", exitcode, QString::fromLocal8Bit(backend->readAllStandardError().replace('\n', "<br>"))));
	else handleTransmissionError(i18n("Backend process has exited with code %1, message: '%2'.", exitcode, QString::fromLocal8Bit(backend->readAllStandardError().replace('\n', "<br>"))));
}

void RKFrontendTransmitter::writeRequest(RBackendRequest *request) {
	RK_TRACE(RBACKEND);

	transmitRequest(request);
	connection->flush();
	delete request;
}

void RKFrontendTransmitter::handleTransmissionError(const QString &message) {
	RK_TRACE(RBACKEND);

	if (connection) connection->close();
	RBackendRequest *req = new RBackendRequest(false, RBackendRequest::BackendExit);
	req->params[QStringLiteral("message")] = message;
	RKRBackendEvent *event = new RKRBackendEvent(req);
	qApp->postEvent(frontend, event);

	exit();
}
