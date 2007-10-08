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

static const char *version = I18N_NOOP(VERSION);
static const char *description = I18N_NOOP("RKWard");

static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP ("file to open"), 0 },
  { "debug-level <level>", I18N_NOOP ("Verbosity of debug messages (0-5)"), "2"}, 
  { "debug-flags <flags>", I18N_NOOP ("Mask for components to debug (see debug.h)"), "8191" }, 
  { "debugger <command>", I18N_NOOP ("Debugger (enclose any debugger arguments in single quotes ('') together with the command)"), ""}, 
  { "disable-stack-check", I18N_NOOP ("Disable R C stack checking"), 0 }, 
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[]) {
	KAboutData aboutData( "rkward", I18N_NOOP ("RKWard"), version, description, KAboutData::License_GPL, "(c) 2002, 2004, 2005, 2006, 2007", 0, "http://rkward.sf.net", "rkward-devel@lists.sourceforge.net");
	aboutData.addAuthor ("Thomas Friedrichsmeier", I18N_NOOP ("Project leader / main developer"), 0);
	aboutData.addAuthor ("Pierre Ecochard",  I18N_NOOP ("C++ coder since 0.2.9"), 0);
	aboutData.addAuthor ("Stefan Roediger", I18N_NOOP ("Many plugins, suggestions, marketing, translations"), 0);
	aboutData.addAuthor ("Prasenjit Kapat", I18N_NOOP ("Many plugins, suggestions"), 0);
	aboutData.addCredit ("Contributors in alphabetical order", 0, 0);
	aboutData.addCredit ("Philippe Grosjean", I18N_NOOP ("Several helpful comments and discussions"), 0);
	aboutData.addCredit ("Adrien d'Hardemare", I18N_NOOP ("Plugins and patches"), 0);
	aboutData.addCredit ("Yves Jacolin", I18N_NOOP ("New website"), 0);
	aboutData.addCredit ("Marco Martin", I18N_NOOP ("A cool icon"), 0);
	aboutData.addCredit ("Daniele Medri", I18N_NOOP ("RKWard logo, many suggestions, help on wording"), 0);
	aboutData.addCredit ("David Sibai", I18N_NOOP ("Several valuable comments, hints and patches"), 0);
	aboutData.addCredit ("Ilias Soumpasis", I18N_NOOP ("Translation, Suggestions, plugins"), 0);
	aboutData.addCredit ("Ralf Tautenhahn", I18N_NOOP ("Many comments, useful suggestions, and bug reports"), 0);
	aboutData.addCredit ("Roland Vollgraf", I18N_NOOP ("Some patches"), 0);
	aboutData.addCredit (I18N_NOOP ("Many more people on rkward-devel@lists.sourceforge.net"), I18N_NOOP ("Sorry, if we forgot to list you. Please contact us to get added"), 0);

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
	KURL *open_url = 0;
	if (args->count ()) {
		open_url = new KURL (args->makeURL (args->arg (0)));
	}
	stoptions->initial_url = open_url;
	stoptions->no_stack_check = args->isSet ("disable-stack-check");

	RKWardApplication app;
	if (app.isRestored ()) {
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
