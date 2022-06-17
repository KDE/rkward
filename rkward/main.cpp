/*
main.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Tue Oct 29 2002
SPDX-FileCopyrightText: 2002-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/



/*!
**	\mainpage	RKWard
**	\author		Thomas Friedrichsmeier and the RKWard Team
**
**	\section	website	Website
**
**	<A HREF="https://rkward.kde.org">RKWard's project page</A>
**
**	\section	description Description
**
** RKWard is meant to become an easy to use, transparent frontend to the R-language, a very powerful, yet hard-to-get-into 
** scripting-language with a strong focus on statistic functions. It will not only provide a convenient user-interface, however, but also 
** take care of seamless integration with an office-suite. Practical statistics is not just about calculating, after all, but also about 
** documenting and ultimately publishing the results.
**
** RKWard then is (will be) something like a free replacement for commercial statistical packages.
**
** \section docOverview Getting started with the documentation
**
** The following sections of the API-documentation provide useful entry-points:
** 
** - \ref UsingTheInterfaceToR
** - \ref RKComponentProperties
**
**	\section	copyright	Copyright
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
*/


#include <kaboutdata.h>
#include <KLocalizedString>
#include <KUrlAuthorized>
#include <KMessageBox>
#ifdef WITH_KCRASH
#	include <KCrash>
#endif

#include <QString>
#include <QMutex>
#include <QTemporaryFile>
#include <QDir>
#include <QThread>
#include <QApplication>
#include <QUrl>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTime>
#include <QSettings>
#include <QStandardPaths>
#if defined(Q_OS_MACOS) || defined(Q_OS_WINDOWS)
#	include <QVersionNumber>
#endif

#ifdef Q_OS_MACOS
	// Needed to allow execution of launchctl
#	include <QProcess>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "rkward.h"
#include "settings/rksettingsmoduledebug.h"
#include "settings/rksettingsmodulegeneral.h"
#include "rbackend/rksessionvars.h"
#include "windows/rkdebugmessagewindow.h"
#include "misc/rkdbusapi.h"
#include "misc/rkcommonfunctions.h"

#ifdef Q_OS_WIN
	// these are needed for the exit hack.
#	include <windows.h>
#endif

#include "debug.h"

#include "version.h"

#ifndef R_EXECUTABLE
#error config problem
#endif

#ifdef Q_OS_WIN
#	define PATH_VAR_SEP ';'
#else
#	define PATH_VAR_SEP ':'
#endif

QString findExeAtPath (const QString &appname, const QString &path) {
	QDir dir (path);
	dir.makeAbsolute ();
	if (QFileInfo (dir.filePath (appname)).isExecutable ()) return dir.filePath (appname);
#ifdef Q_OS_WIN
	if (QFileInfo (dir.filePath (appname + ".exe")).isExecutable ()) return dir.filePath (appname + ".exe");
	if (QFileInfo (dir.filePath (appname + ".com")).isExecutable ()) return dir.filePath (appname + ".com");
#endif
	return QString ();
}

bool RK_Debug_Terminal = true;
QMutex RK_Debug_Mutex;

void RKDebugMessageOutput (QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
	RK_Debug_Mutex.lock ();
	if (type == QtFatalMsg) {
		fprintf (stderr, "%s\n", qPrintable (msg));
	}

	if (RK_Debug_Terminal) {
#ifdef QT_MESSAGELOGCONTEXT
		fprintf (stderr, "%s, %s: %s", ctx.file, ctx.function, qPrintable (msg));
#else
		Q_UNUSED (ctx);
		fprintf (stderr, "%s", qPrintable (msg));
#endif
		fprintf (stderr, "\n");
	} else {
#ifdef QT_MESSAGELOGCONTEXT
		RK_Debug::debug_file->write (ctx.file);
		RK_Debug::debug_file->write (ctx.function);
#endif
		RK_Debug::debug_file->write (qPrintable (msg));
		RK_Debug::debug_file->write ("\n");
		RK_Debug::debug_file->flush ();
	}
	RK_Debug_Mutex.unlock ();
}

/** The point of this redirect (to be called via the RK_DEBUG() macro) is to separate RKWard specific debug messages from
 * any other noise, coming from Qt / kdelibs. Also it allows us to retain info on flags and level, and to show messages
 * in a tool window, esp. for debugging plugins. */
void RKDebug (int flags, int level, const char *fmt, ...) {
	const int bufsize = 1024*8;
	char buffer[bufsize];

	va_list ap;
	va_start (ap, fmt);
	vsnprintf (buffer, bufsize-1, fmt, ap);
	va_end (ap);
	RKDebugMessageOutput (QtDebugMsg, QMessageLogContext (), buffer);
	if (QApplication::instance ()->thread () == QThread::currentThread ()) {
		// not safe to call from any other than the GUI thread
		RKDebugMessageWindow::newMessage (flags, level, QString (buffer));
	}
}

/** Check if the given path to R (or "auto") is executable, and fail with an appropriate message, otherwise. If "auto" is given as input, try to auto-locate an R installation at the standard
installation path(s) for this platform. */
QString resolveRSpecOrFail (QString input, const QString &message) {
	if (input == QLatin1String ("auto")) {
		QString ret;
#ifdef Q_OS_MACOS
		QString instroot ("/Library/Frameworks/R.framework/Versions");
		if (QFileInfo (instroot).isReadable ()) {
			QDir dir (instroot);
			QStringList candidates = dir.entryList ();
			QVersionNumber highest (0, 0, 0);
			for (int i = candidates.count () - 1; i >= 0; --i) {
				QString found = findExeAtPath ("Resources/bin/R", dir.absoluteFilePath (candidates[i]));
				if (!found.isNull()) {
					QVersionNumber version = QVersionNumber::fromString (candidates[i]);
					if (version > highest) {
						ret = found;
					}
				}
			}
		}
#endif
#ifdef Q_OS_WIN
		QString instroot = QString (getenv ("PROGRAMFILES")) + "/R";
		if (!QFileInfo (instroot).isReadable ()) instroot = QString (getenv ("PROGRAMFILES(x86)")) + "/R";
		if (QFileInfo (instroot).isReadable ()) {
			QDir dir (instroot);
			QStringList candidates = dir.entryList (QStringList ("R-*"), QDir::Dirs);
			QVersionNumber highest (0, 0, 0);
			for (int i = candidates.count () - 1; i >= 0; --i) {
				QString found = findExeAtPath ("bin/R", dir.absoluteFilePath (candidates[i]));
				if (!found.isNull()) {
					QVersionNumber version = QVersionNumber::fromString (candidates[i].mid (2));
					if (version > highest) {
						ret = found;
					}
				}
			}
		}
#endif
		// On Unix, but also, if R was not found in the default locations on Windows or Mac,
		// try to find R in the system path.
		if (ret.isNull ()) ret = QStandardPaths::findExecutable ("R");

		if (ret.isNull() || !QFileInfo (ret).isExecutable()) {
			QMessageBox::critical (0, i18n ("Unable to detect R installation"), i18n ("RKWard failed to detect an R installation on this system. Either R is not installed, or not at one of the standard installation locations. You can use the command line parameter '--r-executable <i>auto / PATH_TO_R</i>', or supply an rkward.ini file to specify a non-standard location."));
			exit (1);
		}

		RK_DEBUG (APP, DL_DEBUG, "Using auto-detected R at %s", qPrintable (ret));
		return ret;
	} else {
		if (QFileInfo (input).isExecutable ()) {
			return input;
		}

		// TODO, while fixing krazy2 warnings: KMessageBox layout for static messages is quirky in that it has squeezed caption, and does not allow resize -> Submit a patch.
		//KMessageBox::error (0, QString ("The R executable specified on the command line (%1) does not exist or is not executable.").arg (r_exe), "Specified R executable does not exist");
		QMessageBox::critical (0, i18n ("Specified R executable does not exist"), message);
		exit(1);
	}
	return QString(); // not reached
}

#ifndef NO_QT_WEBENGINE
#include <QWebEngineUrlScheme>
#endif

int main (int argc, char *argv[]) {
	RK_Debug::RK_Debug_Level = DL_WARNING;
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#ifndef NO_QT_WEBENGINE
	// annoyingly, QWebEngineUrlSchemes have to be registered before creating the app.
	QWebEngineUrlScheme scheme("help");
	scheme.setSyntax (QWebEngineUrlScheme::Syntax::Path);
	scheme.setFlags (QWebEngineUrlScheme::LocalScheme|QWebEngineUrlScheme::LocalAccessAllowed);
	QWebEngineUrlScheme::registerScheme (scheme);
#endif
	QApplication app (argc, argv);
#ifdef WITH_KCRASH
	KCrash::setDrKonqiEnabled (true);
#endif
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
	// Follow the example of kate, and use breeze theme on Windows and Mac, which appears to work best
	QApplication::setStyle(QStringLiteral("breeze"));
#endif
	// Don't complain when linking rkward://-pages from Rd pages
	KUrlAuthorized::allowUrlAction ("redirect", QUrl("http://"), QUrl ("rkward://"));
	// Don't complain when trying to open help pages
	KUrlAuthorized::allowUrlAction ("redirect", QUrl("rkward://"), QUrl ("help:"));

	KLocalizedString::setApplicationDomain ("rkward");
	KAboutData aboutData ("rkward", i18n ("RKWard"), RKWARD_VERSION, i18n ("Frontend to the R statistics language"), KAboutLicense::GPL, i18n ("(c) 2002 - 2022"), QString (), "https://rkward.kde.org");
	aboutData.addAuthor (i18n ("Thomas Friedrichsmeier"), i18n ("Project leader / main developer"));
	aboutData.addAuthor (i18n ("Pierre Ecochard"), i18n ("C++ developer between 2004 and 2007"));
	aboutData.addAuthor (i18n ("Prasenjit Kapat"), i18n ("Many plugins, suggestions, plot history feature"));
	aboutData.addAuthor (i18n ("Meik Michalke"), i18n ("Many plugins, suggestions, rkwarddev package"));
	aboutData.addAuthor (i18n ("Stefan Roediger"), i18n ("Many plugins, suggestions, marketing, translations"));
	aboutData.addCredit (i18n ("Contributors in alphabetical order"));
	aboutData.addCredit (i18n ("Björn Balazs"), i18n ("Extensive usability feedback"));
	aboutData.addCredit (i18n ("Aaron Batty"), i18n ("Wealth of feedback, hardware donations"));
	aboutData.addCredit (i18n ("Jan Dittrich"), i18n ("Extensive usability feedback"));
	aboutData.addCredit (i18n ("Philippe Grosjean"), i18n ("Several helpful comments and discussions"));
	aboutData.addCredit (i18n ("Adrien d'Hardemare"), i18n ("Plugins and patches"));
	aboutData.addCredit (i18n ("Yves Jacolin"), i18n ("New website"));
	aboutData.addCredit (i18n ("Germán Márquez Mejía"), i18n ("HP filter plugin, spanish translation"), 0);
	aboutData.addCredit (i18n ("Marco Martin"), i18n ("A cool icon"));
	aboutData.addCredit (i18n ("Daniele Medri"), i18n ("RKWard logo, many suggestions, help on wording"));
	aboutData.addCredit (i18n ("David Sibai"), i18n ("Several valuable comments, hints and patches"));
	aboutData.addCredit (i18n ("Ilias Soumpasis"), i18n ("Translation, Suggestions, plugins"));
	aboutData.addCredit (i18n ("Ralf Tautenhahn"), i18n ("Many comments, useful suggestions, and bug reports"));
	aboutData.addCredit (i18n ("Jannis Vajen"), i18n ("German Translation, bug reports"));
	aboutData.addCredit (i18n ("Roland Vollgraf"), i18n ("Some patches"));
	aboutData.addCredit (i18n ("Roy Qu"), i18n ("patches and helpful comments"));
	aboutData.addCredit (i18n ("Many more people on rkward-devel@kde.org"), i18n ("Sorry, if we forgot to list you. Please contact us to get added"));
	aboutData.setOtherText(QString("<p><b>%1</b></p><ul><li><a href=\"http://dev1.jstatsoft.org/article/view/v049i09\">%2</a></li><li>Friedrichsmeier, T. &amp; the RKWard Team (%3). RKWard: %4. Version %5. %6</li></ul>").arg(i18n("How to cite:"), i18n("Peer-reviewed article in the Journal of Statistical Software"), aboutData.copyrightStatement().right(4), aboutData.shortDescription(), aboutData.version(), aboutData.homepage()));
	KAboutData::setApplicationData (aboutData);

	QCommandLineParser parser;
	parser.addVersionOption ();
	parser.addHelpOption ();
	parser.addOption (QCommandLineOption ("evaluate", i18n ("After starting (and after loading the specified workspace, if applicable), evaluate the given R code."), "Rcode", QString ()));
	parser.addOption (QCommandLineOption ("debug-level", i18n ("Verbosity of debug messages (0-5)"), "level", "2"));
	parser.addOption (QCommandLineOption ("debug-flags", i18n ("Mask for components to debug (see debug.h)"), "flags", QString::number (DEBUG_ALL)));
	parser.addOption (QCommandLineOption ("debug-output", i18n ("Where to send debug message (file|terminal)"), "where", "file"));
	parser.addOption (QCommandLineOption ("backend-debugger", i18n ("Debugger for the backend. (Enclose any debugger arguments in single quotes ('') together with the command. Make sure to re-direct stdout!)"), "command", QString ()));
	parser.addOption (QCommandLineOption ("r-executable", i18n ("Use specified R installation, instead of the one configured at compile time (note: rkward R library must be installed to that installation of R)"), "command", QString ()));
	parser.addOption (QCommandLineOption ("reuse", i18n ("Reuse a running RKWard instance (if available). If a running instance is reused, only the file arguments will be interpreted, all other options will be ignored.")));
	parser.addOption (QCommandLineOption ("autoreuse", i18n ("Behaves like --reuse, if any file arguments are also given, starts a new instance, otherwise. Intended for use in the .desktop file.")));
	parser.addOption (QCommandLineOption ("nowarn-external", i18n ("When used in conjunction with rkward://runplugin/-URLs specified on the command line, suppresses the warning about application-external (untrusted) links.")));
	parser.addPositionalArgument ("files", i18n ("File or files to open, typically a workspace, or an R script file. When loading several things, you should specify the workspace, first."), "[Files...]");

	aboutData.setupCommandLine (&parser);
	parser.process (app);
	aboutData.processCommandLine (&parser);

	// Set up debugging
	RK_Debug::RK_Debug_Level = DL_FATAL - QString (parser.value ("debug-level")).toInt ();
	RK_Debug::RK_Debug_Flags = QString (parser.value ("debug-flags")).toInt ();
	RK_Debug_Terminal = QString (parser.value ("debug-output")) == "terminal";
	if (RK_Debug::setupLogFile (QDir::tempPath () + "/rkward.frontend")) {
		RK_DEBUG (APP, DL_INFO, "Full debug output is at %s", qPrintable (RK_Debug::debug_file->fileName ()));
	} else {
		RK_Debug_Terminal = true;
		RK_DEBUG (APP, DL_INFO, "Failed to open debug file %s", qPrintable (RK_Debug::debug_file->fileName ()));
	}
	qInstallMessageHandler (RKDebugMessageOutput);

	// handle positional (file) arguments, first
	QStringList url_args = parser.positionalArguments ();
	if (!url_args.isEmpty ()) {
		for (int i = 0; i < url_args.size (); ++i) {
			url_args[i] = QUrl::fromUserInput (url_args[i], QDir::currentPath (), QUrl::AssumeLocalFile).toString ();
		}
		RKSettingsModuleGeneral::setStartupOption("initial_urls", url_args);
		RKSettingsModuleGeneral::setStartupOption("warn_external", !parser.isSet("nowarn-external"));
	}
	RKSettingsModuleGeneral::setStartupOption("evaluate", parser.value("evaluate"));
	RKSettingsModuleGeneral::setStartupOption("backend-debugger", parser.value("backend-debugger"));

	// MacOS may need some path adjustments, first
#ifdef Q_OS_MACOS
	QString oldpath = qgetenv ("PATH");
	if (!oldpath.contains (INSTALL_PATH)) {
		//ensure that PATH is set to include what we deliver with the bundle
		qputenv ("PATH", QString ("%1/bin:%1/sbin:%2").arg (INSTALL_PATH).arg (oldpath).toLocal8Bit ());
		if (RK_Debug::RK_Debug_Level > 3) qDebug ("Adjusting system path to %s", qPrintable (qgetenv ("PATH")));
	}
	if (!qEnvironmentVariableIsSet ("DBUS_LAUNCHD_SESSION_BUS_SOCKET")) {
		// try to ensure that DBus is running before trying to connect
		QProcess::execute ("launchctl", QStringList () << "load" << "/Library/LaunchAgents/org.freedesktop.dbus-session.plist");
	}
#endif
	// This is _not_ the same path adjustment as above: Make sure to add the current dir to the path, before launching R and backend.
	QStringList syspaths = QString (qgetenv ("PATH")).split (PATH_VAR_SEP);
	if (!syspaths.contains (app.applicationDirPath ())) {
		syspaths.prepend (RKCommonFunctions::windowsShellScriptSafeCommand (app.applicationDirPath ()));
		qputenv ("PATH", syspaths.join (PATH_VAR_SEP).toLocal8Bit ());
	}

	// Handle --reuse option, by placing a dbus-call to existing RKWard process (if any) and exiting
	if (parser.isSet ("reuse") || (parser.isSet ("autoreuse") && !url_args.isEmpty ())) {
		if (!QDBusConnection::sessionBus ().isConnected ()) {
			RK_DEBUG (DEBUG_ALL, DL_WARNING, "Could not connect to session dbus");
		} else {
			QDBusInterface iface (RKDBUS_SERVICENAME, "/", "", QDBusConnection::sessionBus ());
			if (iface.isValid ()) {
				QDBusReply<void> reply = iface.call ("openAnyUrl", url_args, !parser.isSet ("nowarn-external"));
				if (!reply.isValid ()) {
					RK_DEBUG (DEBUG_ALL, DL_ERROR, "Error while placing dbus call: %s", qPrintable (reply.error ().message ()));
					return 1;
				}
				return 0;
			}
		}
	}

	// Look for R:
	//- command line parameter
	//- Specified in cfg file next to rkward executable
	//- compile-time default
	QString r_exe = parser.value ("r-executable");
	if (!r_exe.isNull ()) {
		r_exe = resolveRSpecOrFail (r_exe, i18n ("The R executable specified on the command line (%1) does not exist or is not executable.", r_exe));
		RK_DEBUG (APP, DL_DEBUG, "Using R specified on command line");
	} else {
		QDir frontend_path = app.applicationDirPath ();
		QFileInfo rkward_ini_file (frontend_path.absoluteFilePath ("rkward.ini"));
		if (rkward_ini_file.isReadable ()) {
			QSettings rkward_ini (rkward_ini_file.absoluteFilePath (), QSettings::IniFormat);
			r_exe = rkward_ini.value ("R executable").toString ();
			if (!r_exe.isNull ()) {
				if (QDir::isRelativePath (r_exe) && r_exe != QStringLiteral ("auto")) {
					r_exe = frontend_path.absoluteFilePath (r_exe);
				}
				r_exe = resolveRSpecOrFail (r_exe, i18n ("The R executable (%1) specified in the rkward.ini file (%2) does not exist or is not executable.", r_exe, rkward_ini_file.absoluteFilePath ()));
			}
			RK_DEBUG (APP, DL_DEBUG, "Using R as configured in config file %s", qPrintable (rkward_ini_file.absoluteFilePath ()));
		}
		if (r_exe.isNull ()) {
			r_exe = resolveRSpecOrFail (R_EXECUTABLE, i18n ("The R executable specified at compile time (%1) does not exist or is not executable. Probably the installation of R has moved. You can use the command line parameter '--r-executable <i>auto / PATH_TO_R</i>', or supply an rkward.ini file to specify the new location.", QString (R_EXECUTABLE)));
			RK_DEBUG (APP, DL_DEBUG, "Using R as configured at compile time");
		}
	}
	RKSessionVars::r_binary = r_exe;

	if (app.isSessionRestored ()) {
		RESTORE(RKWardMainWindow);	// well, whatever this is supposed to do -> TODO
	} else {
		new RKWardMainWindow ();
	}

	// do it!
	int status = app.exec ();

	qInstallMessageHandler (0);
	RK_Debug::debug_file->close ();

	return status;
}
