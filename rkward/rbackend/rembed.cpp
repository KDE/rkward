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

#include "../debug.h"

REmbed::REmbed() : REmbedInternal() {
	RK_TRACE (RBACKEND);
}

REmbed::~REmbed() {
	RK_TRACE (RBACKEND);
	outfile.close ();
	errfile.close ();
}

int REmbed::initialize () {
	RK_TRACE (RBACKEND);
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
	int status = 0;
	
	runCommandInternal ("library (\"rkward\")\n", &error);
	if (error) status |= LibLoadFail;
	runCommandInternal ("options (pager=\"" + RKSettingsModuleR::pagerApp () + "\")\n", &error);
	if (error) status |= OtherFail;
	runCommandInternal ("sink (\"" + RKSettingsModuleLogfiles::filesPath () + "/r_out\")\n", &error);
	if (error) status |= SinkFail;
	runCommandInternal ("sink (file (\"" +RKSettingsModuleLogfiles::filesPath () +"/r_err\", \"w\"), FALSE, \"message\")\n", &error);
	if (error) status |= SinkFail;
	runCommandInternal (".rk.socket <- socketConnection (port=4242)\n", &error);
	if (error) status |= OtherFail;
	
	outfile_offset = 0;
	errfile_offset = 0;
		
	outfile.setName (RKSettingsModuleLogfiles::filesPath () + "/r_out");
	if (!outfile.open (IO_ReadOnly)) {
		if (error) status |= SinkFail;
	}
	errfile.setName (RKSettingsModuleLogfiles::filesPath () + "/r_err");
	if (!errfile.open (IO_ReadOnly)) {
		if (error) status |= SinkFail;
	}
	
	return status;
}

void REmbed::runCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	
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
		RK_DO (qDebug ("Command failed: command was: '%s'", command->command ().latin1 ()), RBACKEND, DL_WARNING);
	} else {
		command->status = RCommand::WasTried;
	}

	if (command->type () & RCommand::DirectToOutput) {
		runCommandInternal ("sink ()\n", &error);
	}
	
	QString temp = "";
	while (!outfile.atEnd ()) {
		outfile.readLine (temp, 2048);
		command->_output.append (temp);
		command->status |= RCommand::HasOutput;
	}

	temp = "";
	while (!errfile.atEnd ()) {
		errfile.readLine (temp, 2048);
		command->_error.append (temp);
		command->status |= RCommand::HasError;
	}
	RK_DO (if (error) qDebug ("- error message was: '%s'", command->error ().latin1 ()), RBACKEND, DL_WARNING);
}
