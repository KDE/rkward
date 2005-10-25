/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002 by 
    email                : 
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
#include "config.h"

#include "debug.h"

int RK_Debug_Level = 0;
int RK_Debug_Flags = ALL;

static const char *version = I18N_NOOP(VERSION);
static const char *description = I18N_NOOP("RKWard");

static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP ("file to open"), 0 },
  { "debug-level <level>", I18N_NOOP ("Verbosity of debug messages (0-5)"), "2"}, 
  { "debug-flags <flags>", I18N_NOOP ("Mask for components to debug as a binary number (see debug.h)"), "1111111111111" }, 
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[]) {
	KAboutData aboutData( "rkward", I18N_NOOP ("RKWard"), version, description, KAboutData::License_GPL, "(c) 2002, 2004, 2005", 0, "http://rkward.sf.net", QString::null);
	aboutData.addAuthor ("Thomas Friedrichsmeier", I18N_NOOP ("Project leader"), QString::null);
	aboutData.addAuthor ("Pierre Ecochard",  I18N_NOOP ("Core coder since 0.2.9"), QString::null);
	aboutData.addCredit ("Contributors in alphabetical order", QString::null, QString::null);
	aboutData.addCredit ("Philippe Grosjean", I18N_NOOP ("Several helpful comments and discussions"), QString::null);
	aboutData.addCredit ("Adrien d'Hardemare", I18N_NOOP ("Plugins and patches"), QString::null);
	aboutData.addCredit ("Yves Jacolin", I18N_NOOP ("New website"), QString::null);
	aboutData.addCredit ("Marco Martin", I18N_NOOP ("A cool icon"), QString::null);
	aboutData.addCredit ("Daniele Medri", I18N_NOOP ("RKWard logo, many suggestions, help on wording"), QString::null);
	aboutData.addCredit ("David Sibai", I18N_NOOP ("Several valuable comments, hints and patches"), QString::null);
	aboutData.addCredit (I18N_NOOP ("Many more people on rkward-devel@lists.sourceforge.net"), I18N_NOOP ("Sorry, I forgot to list you. Please contact me to get added"), QString::null);
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
	
  KApplication app;
 
  if (app.isRestored())
  {
    RESTORE(RKwardApp);
  }
  else 
  {
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	RK_Debug_Level = 5 - QString (args->getOption ("debug-level")).toInt ();
	RK_Debug_Flags = QString (args->getOption ("debug-flags")).toInt (0, 2);
	qDebug ("Debug-flags as decimal: %d", RK_Debug_Flags);
	
	KURL *open_url = 0;
	if (args->count ()) {
		open_url = new KURL (args->arg (0));
	}
	args->clear();

	new RKwardApp(open_url);
  }

  return app.exec();
}  
