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

static const char *description =
	I18N_NOOP("RKward");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("file to open"), 0 },
  { "debug-level <level>", I18N_NOOP("Verbosity of debug messages (0-5)"), "4"}, 
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

	KAboutData aboutData( "rkward", I18N_NOOP("RKWard"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2002, 2004", 0, 0, "");
	aboutData.addAuthor ("Thomas Friedrichsmeier",0, "");
	aboutData.setHomepage ("http://rkward.sf.net");
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
	
	KURL *open_url = 0;
	if (args->count ()) {
		open_url = new KURL (args->arg (0));
	}
	args->clear();

	new RKwardApp(open_url);
  }

  return app.exec();
}  
