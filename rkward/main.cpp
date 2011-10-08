/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011 by Thomas Friedrichsmeier 
    email                : tfry@users.sourceforge.net
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
**	<A HREF="http://rkward.sourceforge.net">RKWard's project page</A>
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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qstring.h>
#include <QMutex>
#include <QTemporaryFile>
#include <QDir>

#include <stdio.h>
#include <stdlib.h>

#include "rkward.h"
#include "rkwardapplication.h"
#include "settings/rksettingsmoduledebug.h"

#ifdef Q_WS_WIN
	// these are needed for the exit hack.
#	include <windows.h>
#endif

#include "debug.h"

#include "version.h"

int RK_Debug_Level = 0;
int RK_Debug_Flags = ALL;
int RK_Debug_CommandStep = 0;
QMutex RK_Debug_Mutex;

static KCmdLineOptions options;

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

int main(int argc, char *argv[]) {
/* #ifdef Q_WS_X11
	This (along with the proper includes, of course) makes library (gWidgetsRGtk2) work on X11 on some systems.
	Unfortunately, on others, I causes lockups when plotting.

	XInitThreads ();
	XtToolkitThreadInitialize ();
#endif */
	options.add ("evaluate <Rcode>", ki18n ("After starting (and after loading the specified workspace, if applicable), evaluate the given R code."), 0);
	options.add ("debug-level <level>", ki18n ("Verbosity of debug messages (0-5)"), "2");
	options.add ("debug-flags <flags>", ki18n ("Mask for components to debug (see debug.h)"), "8191");
	options.add ("debugger <command>", ki18n ("Debugger (enclose any debugger arguments in single quotes ('') together with the command)"), "");
	options.add ("+[File]", ki18n ("R workspace file to open"), 0);

	KAboutData aboutData("rkward", QByteArray (), ki18n ("RKWard"), RKWARD_VERSION, ki18n ("Frontend to the R statistics language"), KAboutData::License_GPL, ki18n ("(c) 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010"), KLocalizedString (), "http://rkward.sf.net", "rkward-devel@lists.sourceforge.net");
	aboutData.addAuthor (ki18n ("%1").subs ("Thomas Friedrichsmeier"), ki18n ("Project leader / main developer"));
	aboutData.addAuthor (ki18n ("%1").subs ("Pierre Ecochard"), ki18n ("C++ coder since 0.2.9"));
	aboutData.addAuthor (ki18n ("%1").subs ("Stefan Roediger"), ki18n ("Many plugins, suggestions, marketing, translations"));
	aboutData.addAuthor (ki18n ("%1").subs ("Prasenjit Kapat"), ki18n ("Many plugins, suggestions"));
	aboutData.addCredit (ki18n ("Contributors in alphabetical order"));
	aboutData.addCredit (ki18n ("%1").subs ("Philippe Grosjean"), ki18n ("Several helpful comments and discussions"));
	aboutData.addCredit (ki18n ("%1").subs ("Adrien d'Hardemare"), ki18n ("Plugins and patches"));
	aboutData.addCredit (ki18n ("%1").subs ("Yves Jacolin"), ki18n ("New website"));
	aboutData.addCredit (ki18n ("%1").subs ("Germán Márquez Mejía"), ki18n ("HP filter plugin, spanish translation"), 0);
	aboutData.addCredit (ki18n ("%1").subs ("Marco Martin"), ki18n ("A cool icon"));
	aboutData.addCredit (ki18n ("%1").subs ("Daniele Medri"), ki18n ("RKWard logo, many suggestions, help on wording"));
	aboutData.addCredit (ki18n ("%1").subs ("David Sibai"), ki18n ("Several valuable comments, hints and patches"));
	aboutData.addCredit (ki18n ("%1").subs ("Ilias Soumpasis"), ki18n ("Translation, Suggestions, plugins"));
	aboutData.addCredit (ki18n ("%1").subs ("Ralf Tautenhahn"), ki18n ("Many comments, useful suggestions, and bug reports"));
	aboutData.addCredit (ki18n ("%1").subs ("Jannis Vajen"), ki18n ("German Translation, bug reports"));
	aboutData.addCredit (ki18n ("%1").subs ("Roland Vollgraf"), ki18n ("Some patches"));
	aboutData.addCredit (ki18n ("%1").subs ("Roy Qu"), ki18n ("patches and helpful comments"));
	aboutData.addCredit (ki18n ("%1").subs ("Meik Michalke"), ki18n ("Many plugins, suggestions"));
	aboutData.addCredit (ki18n ("Many more people on rkward-devel@lists.sourceforge.net"), ki18n ("Sorry, if we forgot to list you. Please contact us to get added"));

	// before initializing the commandline args, remove the ".bin" from "rkward.bin".
	// This is so it prints "Usage rkward..." instead of "Usage rkward.bin...", etc.
	// it seems safest to keep a copy, since the shell still owns argv
	char *argv_copy[argc];
	argv_copy[0] = qstrdup (QString (argv[0]).remove (".frontend").replace (".exe", ".bat").toLocal8Bit ());
	for (int i = 1; i < argc; ++i) {
		argv_copy[i] = argv[i];
	}
	KCmdLineArgs::init (argc, argv_copy, &aboutData);
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	RK_Debug_Level = DL_FATAL - QString (args->getOption ("debug-level")).toInt ();
	RK_Debug_Flags = QString (args->getOption ("debug-flags")).toInt ();
	if (!args->getOption ("debugger").isEmpty ()) {
		RK_DO (qDebug ("--debugger option should have been handled by wrapper script. Ignoring."), ALL, DL_ERROR);
	}

	RKWardStartupOptions *stoptions = new RKWardStartupOptions;
	if (args->count ()) {
		stoptions->initial_url = KUrl (args->makeURL (args->arg (0).toLatin1()));
	}
	stoptions->evaluate = args->getOption ("evaluate");

	RKWardApplication app;
	// install message handler *after* the componentData has been initialized
	RKSettingsModuleDebug::debug_file = new QTemporaryFile (QDir::tempPath () + "/rkward.frontend");
	RKSettingsModuleDebug::debug_file->setAutoRemove (false);
	if (RKSettingsModuleDebug::debug_file->open ()) {
		RK_DO (qDebug ("Full debug output is at %s", qPrintable (RKSettingsModuleDebug::debug_file->fileName ())), APP, DL_INFO);
		qInstallMsgHandler (RKDebugMessageOutput);
	}

	if (app.isSessionRestored ()) {
		RESTORE(RKWardMainWindow);	// well, whatever this is supposed to do -> TODO
	} else {
		new RKWardMainWindow(stoptions);
	}
	args->clear();

	// Usually, KDE always adds the current directory to the list of prefixes.
	// However, since RKWard 0.5.6, the main binary is in KDE's libexec dir, which defies this mechanism. Therefore, RKWARD_ENSURE_PREFIX is set from the wrapper script.
	char *add_path = getenv ("RKWARD_ENSURE_PREFIX");
	if (add_path) KGlobal::dirs ()->addPrefix (QString::fromLocal8Bit (add_path));

	// do it!
	int status = app.exec ();

	qInstallMsgHandler (0);
	RKSettingsModuleDebug::debug_file->close ();

#ifdef Q_WS_WIN
	// HACK: Somehow, if we created a windows graph-device during runtime (possibly also on other conditions), we just can't exit cleanly anymore.
	// We get out of the event loop, but once we return from main (including using _endthread(), _exit(), exit(), abort(), raise(SIGSEGV),
	// ExitProcess(), and I don't know, what else I tried), the process just continues to sit there, hogging up one CPU.
	// R 2.9.0, KDElibs 4.2.3, Qt 4.4.3, WinXP SP2
	// So what we do is
	// 1) Make extra sure, everything important is flushed
	// 2) Use TerminateProcess () to kill all threads
	// Alternatives: Use system ("taskkill ..."), but this command was not introduced before WinXP
	app.flush();
	app.processEvents();
	_flushall ();

	TerminateProcess (GetCurrentProcess (), 0);
#endif
	return status;
}
