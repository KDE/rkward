/***************************************************************************
                          rembed  -  description
                             -------------------
    begin                : Mon Jul 26 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rembed.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettingsmodulelogfiles.h"

REmbed::REmbed() : REmbedInternal() {
}

REmbed::~REmbed() {
	outfile.close ();
	errfile.close ();
}

bool REmbed::initialize () {
	QString r_home = RKSettingsModuleR::rHomeDir();

	QStringList arglist = RKSettingsModuleR::getOptionList();
	char *argv[arglist.count ()];
	int argc = 0;
	QStringList::iterator it;
	for (it = arglist.begin (); it != arglist.end (); ++it) {
		argv[argc] = qstrdup ((*it).latin1 ());
		argc++;
	}
	
	startR (r_home, argc, argv);
	
	for (--argc; argc >= 0; --argc) {
		delete argv[argc];
	}
	
	bool error;
	
	runCommandInternal ("options (pager=\"" + RKSettingsModuleR::pagerApp () + "\")\n", &error);
	runCommandInternal ("sink (\"" + RKSettingsModuleLogfiles::filesPath () + "/r_out\")\n", &error);
	runCommandInternal ("sink (file (\"" +RKSettingsModuleLogfiles::filesPath () +"/r_err\", \"w\"), FALSE, \"message\")\n", &error);

	outfile_offset = 0;
	errfile_offset = 0;
		
	outfile.setName (RKSettingsModuleLogfiles::filesPath () + "/r_out");
	if (!outfile.open (IO_ReadOnly)) {
		error = true;
	}
	errfile.setName (RKSettingsModuleLogfiles::filesPath () + "/r_err");
	if (!errfile.open (IO_ReadOnly)) {
		error = true;
	}
	
	return error;
}

void REmbed::runCommand (RCommand *command) {
	
	if (command->type () & RCommand::EmptyCommand) return;
	
	bool error;
	
	if (command->type () & RCommand::DirectToOutput) {
		runCommandInternal ("sink (\"" + RKSettingsModuleLogfiles::filesPath () + "/rk_out.html\", append=TRUE, split=TRUE)\n", &error);
	}
	
	if (command->type () & RCommand::GetStringVector) {
		command->string_data = getCommandAsStringVector (command->command ().latin1 (), &(command->string_count), &error);
	} else if (command->type () & RCommand::GetRealVector) {
		command->real_data = getCommandAsRealVector (command->command ().latin1 (), &(command->real_count), &error);
	} else if (command->type () & RCommand::GetIntVector) {
		command->integer_data = getCommandAsIntVector (command->command ().latin1 (), &(command->integer_count), &error);
	} else {
		runCommandInternal (command->command ().latin1 (), &error);
	}
	
	if (error) {
		command->status = RCommand::WasTried | RCommand::Failed;
	} else {
		command->status = RCommand::WasTried;
	}

	if (command->type () & RCommand::DirectToOutput) {
		runCommandInternal ("sink ()\n", &error);
	}
	
	QString temp = "";
	while (!outfile.atEnd ()) {
		outfile.readLine (temp, 2048);
		qDebug (temp);
		command->_output.append (temp);
		command->status |= RCommand::HasOutput;
	}
	qDebug ("output: %s", temp.latin1 ());

	temp = "";
	while (!errfile.atEnd ()) {
		errfile.readLine (temp, 2048);
		qDebug (temp);
		command->_error.append (temp);
		command->status |= RCommand::HasError;
	}
	qDebug ("error: %s", temp.latin1 ());
}
