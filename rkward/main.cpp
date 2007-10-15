/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002, 2005, 2006, 2007 by Thomas Friedrichsmeier 
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

#include <qstring.h>

#include "rkward.h"
#include "rkwardapplication.h"

#include "debug.h"

#include "../version.h"

int RK_Debug_Level = 0;
int RK_Debug_Flags = ALL;

static KCmdLineOptions options;

int main(int argc, char *argv[]) {
	options.add ("+[File]", ki18n ("file to open"), 0);
	options.add ("debug-level <level>", ki18n ("Verbosity of debug messages (0-5)"), "2");
	options.add ("debug-flags <flags>", ki18n ("Mask for components to debug (see debug.h)"), "8191");
	options.add ("debugger <command>", ki18n ("Debugger (enclose any debugger arguments in single quotes ('') together with the command)"), "");
	options.add ("disable-stack-check", ki18n ("Disable R C stack checking"), 0);

	KAboutData aboutData("rkward", QByteArray (), ki18n ("RKWard"), VERSION, ki18n ("Frontend to the R statistics language"), KAboutData::License_GPL, ki18n ("(c) 2002, 2004, 2005, 2006, 2007"), KLocalizedString (), "http://rkward.sf.net", "rkward-devel@lists.sourceforge.net");
	aboutData.addAuthor (ki18n ("%1").subs ("Thomas Friedrichsmeier"), ki18n ("Project leader / main developer"));
	aboutData.addAuthor (ki18n ("%1").subs ("Pierre Ecochard"), ki18n ("C++ coder since 0.2.9"));
	aboutData.addAuthor (ki18n ("%1").subs ("Stefan Roediger"), ki18n ("Many plugins, suggestions, marketing, translations"));
	aboutData.addAuthor (ki18n ("%1").subs ("Prasenjit Kapat"), ki18n ("Many plugins, suggestions"));
	aboutData.addCredit (ki18n ("Contributors in alphabetical order"));
	aboutData.addCredit (ki18n ("%1").subs ("Philippe Grosjean"), ki18n ("Several helpful comments and discussions"));
	aboutData.addCredit (ki18n ("%1").subs ("Adrien d'Hardemare"), ki18n ("Plugins and patches"));
	aboutData.addCredit (ki18n ("%1").subs ("Yves Jacolin"), ki18n ("New website"));
	aboutData.addCredit (ki18n ("%1").subs ("Marco Martin"), ki18n ("A cool icon"));
	aboutData.addCredit (ki18n ("%1").subs ("Daniele Medri"), ki18n ("RKWard logo, many suggestions, help on wording"));
	aboutData.addCredit (ki18n ("%1").subs ("David Sibai"), ki18n ("Several valuable comments, hints and patches"));
	aboutData.addCredit (ki18n ("%1").subs ("Ilias Soumpasis"), ki18n ("Translation, Suggestions, plugins"));
	aboutData.addCredit (ki18n ("%1").subs ("Ralf Tautenhahn"), ki18n ("Many comments, useful suggestions, and bug reports"));
	aboutData.addCredit (ki18n ("%1").subs ("Roland Vollgraf"), ki18n ("Some patches"));
	aboutData.addCredit (ki18n ("Many more people on rkward-devel@lists.sourceforge.net"), ki18n ("Sorry, if we forgot to list you. Please contact us to get added"));

	// before initializing the commandline args, remove the ".bin" from "rkward.bin".
	// This is so it prints "Usage rkward..." instead of "Usage rkward.bin...", etc.
	// it seems safest to keep a copy, since the shell still owns argv[0]
	char *argv_zero_copy = argv[0];
	argv[0] = qstrdup (QString (argv_zero_copy).remove (".bin").local8Bit ());
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	RK_Debug_Level = 5 - QString (args->getOption ("debug-level")).toInt ();
	RK_Debug_Flags = QString (args->getOption ("debug-flags")).toInt ();
	if (!args->getOption ("debugger").isEmpty ()) {
		RK_DO (qDebug ("--debugger option should have been handled by wrapper script. Ignoring."), ALL, DL_ERROR);
	}

	RKWardStartupOptions *stoptions = new RKWardStartupOptions;
	KUrl *open_url = 0;
	if (args->count ()) {
		open_url = new KUrl (args->makeURL (args->arg (0).latin1()));
	}
	stoptions->initial_url = open_url;
	stoptions->no_stack_check = args->isSet ("disable-stack-check");

	RKWardApplication app;
	if (app.isSessionRestored ()) {
		RESTORE(RKWardMainWindow);	// well, whatever this is supposed to do -> TODO
	} else {
		new RKWardMainWindow(stoptions);
	}
	args->clear();

	// do it!
	int status = app.exec ();
	// restore old argv[0] so the shell is happy
	argv[0] = argv_zero_copy;
	return status;
}
