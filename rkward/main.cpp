/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002-2014 by Thomas Friedrichsmeier 
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



/*!
**	\mainpage	RKWard
**	\author		Thomas Friedrichsmeier and the RKWard Team
**
**	\section	website	Website
**
**	<A HREF="http://rkward.kde.org">RKWard's project page</A>
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
#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <qstring.h>
#include <QMutex>
#include <QTemporaryFile>
#include <QDir>
#include <QThread>

#include <stdio.h>
#include <stdlib.h>
#include <QCommandLineParser>

#include "rkward.h"
#include "rkglobals.h"
#include "rkwardapplication.h"
#include "settings/rksettingsmoduledebug.h"
#include "windows/rkdebugmessagewindow.h"

#ifdef Q_WS_WIN
	// these are needed for the exit hack.
#	include <windows.h>
#endif

#include "debug.h"

#include "version.h"

int RK_Debug_Level = 0;
int RK_Debug_Flags = DEBUG_ALL;
int RK_Debug_CommandStep = 0;
QMutex RK_Debug_Mutex;

void RKDebugMessageOutput (QtMsgType type, const char *msg) {
	RK_Debug_Mutex.lock ();
	if (type == QtFatalMsg) {
		fprintf (stderr, "%s\n", msg);
	}
	RKSettingsModuleDebug::debug_file->write (msg);
	RKSettingsModuleDebug::debug_file->write ("\n");
	RKSettingsModuleDebug::debug_file->flush ();
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
	RKDebugMessageOutput (QtDebugMsg, buffer);
	if (QApplication::instance ()->thread () == QThread::currentThread ()) {
		// not safe to call from any other than the GUI thread
		RKDebugMessageWindow::newMessage (flags, level, QString (buffer));
	}
}

QString decodeArgument (const QString &input) {
	return (QUrl::fromPercentEncoding (input.toUtf8()));
}

int main (int argc, char *argv[]) {
	// before initializing the commandline args, remove the ".bin" from "rkward.bin".
	// This is so it prints "Usage rkward..." instead of "Usage rkward.bin...", etc.
	// it seems safest to keep a copy, since the shell still owns argv
	char *argv_copy[argc];
	argv_copy[0] = qstrdup (QString (argv[0]).remove (".frontend").replace (".exe", ".bat").toLocal8Bit ());
	for (int i = 1; i < argc; ++i) {
		argv_copy[i] = argv[i];
	}

	RKWardApplication app (argc, argv_copy);

	KAboutData aboutData ("rkward", i18n ("RKWard"), RKWARD_VERSION, i18n ("Frontend to the R statistics language"), KAboutLicense::GPL, i18n ("(c) 2002, 2004 - 2015"), QString (), "http://rkward.kde.org");
	aboutData.addAuthor (i18n ("Thomas Friedrichsmeier"), i18n ("Project leader / main developer"));
	aboutData.addAuthor (i18n ("Pierre Ecochard"), i18n ("C++ developer between 2004 and 2007"));
	aboutData.addAuthor (i18n ("Prasenjit Kapat"), i18n ("Many plugins, suggestions, plot history feature"));
	aboutData.addAuthor (i18n ("Meik Michalke"), i18n ("Many plugins, suggestions, rkwarddev package"));
	aboutData.addAuthor (i18n ("Stefan Roediger"), i18n ("Many plugins, suggestions, marketing, translations"));
	aboutData.addCredit (i18n ("Contributors in alphabetical order"));
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
	KAboutData::setApplicationData (aboutData);

	QCommandLineParser parser;
	parser.addVersionOption ();
	parser.addHelpOption ();
	parser.addOption (QCommandLineOption ("evaluate", i18n ("After starting (and after loading the specified workspace, if applicable), evaluate the given R code."), "Rcode", QString ()));
	parser.addOption (QCommandLineOption ("debug-level", i18n ("Verbosity of debug messages (0-5)"), "level", "2"));
	parser.addOption (QCommandLineOption ("debug-flags", i18n ("Mask for components to debug (see debug.h)"), "flags", QString::number (DEBUG_ALL)));
	parser.addOption (QCommandLineOption ("debugger", i18n ("Debugger for the frontend. Specify last, or add '--' after all debugger arguments"), "command and arguments", QString ()));
	parser.addOption (QCommandLineOption ("backend-debugger", i18n ("Debugger for the backend. (Enclose any debugger arguments in single quotes ('') together with the command. Make sure to re-direct stdout!)"), "command", QString ()));
	parser.addOption (QCommandLineOption ("r-executable", i18n ("Use specified R installation, instead of the one configured at compile time (note: rkward R library must be installed to that installation of R)"), "command", QString ()));
	parser.addOption (QCommandLineOption ("reuse", i18n ("Reuse a running RKWard instance (if available). If a running instance is reused, only the file arguments will be interpreted, all other options will be ignored.")));
	parser.addPositionalArgument ("files", i18n ("File or files to open, typically a workspace, or an R script file. When loading several things, you should specify the workspace, first."), "[Files...]");

	aboutData.setupCommandLine (&parser);
	parser.process (app);
	aboutData.processCommandLine (&parser);

	RK_Debug_Level = DL_FATAL - QString (parser.value ("debug-level")).toInt ();
	RK_Debug_Flags = QString (parser.value ("debug-flags")).toInt ();
	if (!parser.value ("debugger").isEmpty ()) {
		RK_DEBUG (DEBUG_ALL, DL_ERROR, "--debugger option should have been handled by wrapper script. Ignoring.");
	}

	QStringList url_args = parser.positionalArguments ();
	if (!url_args.isEmpty ()) {
		QVariantList urls_to_open;
		for (int i = 0; i < url_args.count (); ++i) {
			urls_to_open.append (QUrl::fromUserInput (url_args[i], QDir::currentPath(), QUrl::AssumeLocalFile));
		}
		RKGlobals::startup_options["initial_urls"] = urls_to_open;
	}
	RKGlobals::startup_options["evaluate"] = decodeArgument (parser.value ("evaluate"));
	RKGlobals::startup_options["backend-debugger"] = decodeArgument (parser.value ("backend-debugger"));

	// No, I do not really understand the point of separating KDE_LANG from LANGUAGE. We do honor it in so far as not
	// forcing LANGUAGE on the backend, though. Having language as LANGUAGE makes code in RKMessageCatalog much easier compared to KCatalog.
	// KF5 TODO: is this still needed at all? Does KDE_LANG still exist?
	qputenv ("LANGUAGE", QLocale ().bcp47Name ().section ('-', 0, 0).toAscii ());
	// install message handler *after* the componentData has been initialized
	RKSettingsModuleDebug::debug_file = new QTemporaryFile (QDir::tempPath () + "/rkward.frontend");
	RKSettingsModuleDebug::debug_file->setAutoRemove (false);
	if (RKSettingsModuleDebug::debug_file->open ()) {
		RK_DEBUG (APP, DL_INFO, "Full debug output is at %s", qPrintable (RKSettingsModuleDebug::debug_file->fileName ()));
		qInstallMsgHandler (RKDebugMessageOutput);
	}

	if (app.isSessionRestored ()) {
		RESTORE(RKWardMainWindow);	// well, whatever this is supposed to do -> TODO
	} else {
		new RKWardMainWindow ();
	}
	

	// Usually, KDE always adds the current directory to the list of prefixes.
	// However, since RKWard 0.5.6, the main binary is in KDE's libexec dir, which defies this mechanism. Therefore, RKWARD_ENSURE_PREFIX is set from the wrapper script.
	QByteArray add_path = qgetenv ("RKWARD_ENSURE_PREFIX");
	if (!add_path.isEmpty ()) KGlobal::dirs ()->addPrefix (QString::fromLocal8Bit (add_path));

	// do it!
	int status = app.exec ();

	qInstallMsgHandler (0);
	RKSettingsModuleDebug::debug_file->close ();

	return status;
}
