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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include <qstring.h>

#include "rkward.h"

#include "debug.h"

int RK_Debug_Level = 0;
int RK_Debug_Flags = ALL;

static const char *version =
       I18N_NOOP("0.3.0");

static const char *description =
	I18N_NOOP("RKWard");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP ("file to open"), 0 },
  { "debug-level <level>", I18N_NOOP ("Verbosity of debug messages (0-5)"), "4"}, 
  { "debug-flags <flags>", I18N_NOOP ("Mask for components to debug as a binary number (see debug.h)"), "111111111111" }, 
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

	KAboutData aboutData( "rkward", I18N_NOOP ("RKWard"), version, description, KAboutData::License_GPL, "(c) 2002, 2004", 0, "http://rkward.sf.net", "");
	aboutData.addAuthor ("Thomas Friedrichsmeier", "Project leader", "");
	aboutData.addAuthor ("Pierre Ecochard", "Contributor", "");
	aboutData.addAuthor ("Daniele Medri", "Contributor", "");
	aboutData.addCredit ("David Sibai", "Several valuable comments, hints and patches", "");
	aboutData.addCredit ("Daniele Medri", "RKWard logo, many suggestions", "");
	aboutData.addCredit ("Philippe Grosjean", "Several helpful comments and discussions", "");
	aboutData.addCredit ("Many more people on rkward-devel@lists.sourceforge.net", "Sorry, I forgot to list you. Please contact me to get added", "");
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
