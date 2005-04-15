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
#include "../rkglobals.h"
#include "rinterface.h"
#include "rthread.h"

#include "../debug.h"

REmbed::REmbed (RThread *thread) : REmbedInternal() {
	RK_TRACE (RBACKEND);
	RK_ASSERT (this_pointer == 0);
	this_pointer = this;
	RK_ASSERT (this_pointer);
	REmbed::thread = thread;
}

REmbed::~REmbed () {
	RK_TRACE (RBACKEND);
	outfile.close ();
	errfile.close ();
}

void REmbed::handleSubstackCall (char **call, int call_length) {
	RK_TRACE (RBACKEND);
	thread->doSubstack (call, call_length);
}
/*
char **REmbed::handleGetValueCall (char **call, int call_length, int *reply_length) {
	RK_TRACE (RBACKEND);
	return thread->fetchValue (call, call_length, reply_length);
}*/

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
	
	RKWardRError error;
	int status = 0;
	
	runCommandInternal ("library (\"rkward\")\n", &error);
	if (error) status |= LibLoadFail;
	runCommandInternal ("options (pager=\"" + RKSettingsModuleR::pagerApp () + "\")\n", &error);
	if (error) status |= OtherFail;
	runCommandInternal ("sink (\"" + RKSettingsModuleLogfiles::filesPath () + "/r_out\")\n", &error);
	if (error) status |= SinkFail;
	runCommandInternal ("sink (file (\"" +RKSettingsModuleLogfiles::filesPath () +"/r_err\", \"w\"), FALSE, \"message\")\n", &error);
	if (error) status |= SinkFail;
	
	runCommandInternal (".rk.output.file <- \"" +RKSettingsModuleLogfiles::filesPath() + "/rk_out.html\"", &error);
	runCommandInternal (".HTML.file <- \"" +RKSettingsModuleLogfiles::filesPath() + "/rk_out.html\"", &error);
	runCommandInternal (".rk.output.path <- \"" +RKSettingsModuleLogfiles::filesPath() + "\"", &error);
	
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
	
	if (command->status & RCommand::Canceled) return;
	
	RKWardRError error;
	
	int ctype = command->type ();
	const char *ccommand = command->command ().latin1 ();
	
	if (command->type () & RCommand::DirectToOutput) {
		runCommandInternal ("sink (\"" + RKSettingsModuleLogfiles::filesPath () + "/rk_out.html\", append=TRUE, split=TRUE)\n", &error);
	}

	MUTEX_UNLOCK;

	if (ctype & RCommand::GetStringVector) {
		command->string_data = getCommandAsStringVector (ccommand, &(command->string_count), &error);
	} else if (ctype & RCommand::GetRealVector) {
		command->real_data = getCommandAsRealVector (ccommand, &(command->real_count), &error);
	} else if (ctype & RCommand::GetIntVector) {
		command->integer_data = getCommandAsIntVector (ccommand, &(command->integer_count), &error);
	} else {
		runCommandInternal (ccommand, &error, ctype & RCommand::User);
	}

	MUTEX_LOCK;
	
	if (error != NoError) {
		command->status |= RCommand::WasTried | RCommand::Failed;
		if (error == Incomplete) {
			command->status |= RCommand::ErrorIncomplete;
			RK_DO (qDebug ("Command failed (incomplete)"), RBACKEND, DL_WARNING);
		} else if (error == SyntaxError) {
			command->status |= RCommand::ErrorSyntax;
			RK_DO (qDebug ("Command failed (syntax)"), RBACKEND, DL_WARNING);
		} else {
			command->status |= RCommand::ErrorOther;
			RK_DO (qDebug ("Command failed (other)"), RBACKEND, DL_WARNING);
		}
		RK_DO (qDebug ("failed command was: '%s'", command->command ().latin1 ()), RBACKEND, DL_WARNING);
	} else {
		command->status |= RCommand::WasTried;
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
