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

#include <stdlib.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "rksettingsmoduler.h"
#include "rksettingsmodulelogfiles.h"

REmbed::REmbed() : REmbedInternal() {
	QString r_home = RKSettingsModuleR::rHomeDir();
	
	char *env_r_home = getenv ("R_HOME");
	if (!env_r_home) {
		if (r_home == "") {
			qDebug ("guess");
			r_home = "/usr/lib/R/";		// guess
			KMessageBox::warningContinueCancel (0, i18n ("Could not find an R_HOME-environment variable and don't have a stored setting for that either. The R backend requires that variable - if it is not set correctly, the application will quit immediately. We'll try it with a guessed value (" + r_home + ") for now. If it works - fine. If not, you'll have to set the correct value using 'export R_HOME=...' prior to starting RkWard again."), "R_HOME not set");
		}
	} else {
		if (env_r_home != r_home) {
			qDebug ("conflict");
			if (KMessageBox::warningYesNo (0, i18n ("RKWard has a stored setting for the R_HOME-variable. However, in the environment, that variable is currently set to a different value. It's probably safe to assume the environment-setting is correct/more up to date. Using a wrong setting however will cause the application to quit immediately. Do you want to use the stored setting instead of the environment-setting?"), "Conflicting settings for R_HOME") != KMessageBox::Yes) {
				r_home = env_r_home;
			}
		}
	}

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
	
	runCommandInternal ("options (pager=\"" + RKSettingsModuleR::pagerApp () + "\")\n");
	runCommandInternal ("sink (\"" + RKSettingsModuleLogfiles::filesPath () + "/r_out\")\n");
	runCommandInternal ("sink (file (\"" +RKSettingsModuleLogfiles::filesPath () +"/r_err\", \"w\"), FALSE, \"message\")\n");

	// if we're still alive at this point, the setting for r_home must have been correct
	RKSettingsModuleR::r_home_dir = r_home;

	outfile_offset = 0;
	errfile_offset = 0;
	
	bool error = false;
	
	outfile.setName (RKSettingsModuleLogfiles::filesPath () + "/r_out");
	if (!outfile.open (IO_ReadOnly)) {
		error = true;
	}
	errfile.setName (RKSettingsModuleLogfiles::filesPath () + "/r_err");
	if (!errfile.open (IO_ReadOnly)) {
		error = true;
	}
	
	if (error) {
		KMessageBox::error (0, i18n ("There was a problem opening the files needed for communication with R. Most likely this is due to an incorrect setting for the location of these files. Check whether you have correctly configured the location of the log-files (Settings->Configure Settings->Logfiles) and restart RKWard."), i18n ("Error starting R"));
	}
}


REmbed::~REmbed() {
	outfile.close ();
	errfile.close ();
}

void REmbed::runCommand (RCommand *command) {
	qDebug ("TODO: REmbed::runCommand: Error-handling");

	QFile file(RKSettingsModuleLogfiles::filesPath () + "/r_in");
	if (file.open(IO_WriteOnly)) {
		QTextStream stream(&file);
		stream << command->command () << "\n";
		file.close();
	}
	
	if (!runCommandInternal ("source (\"" + RKSettingsModuleLogfiles::filesPath () + "/r_in\")")) {
		command->status = RCommand::WasTried | RCommand::Failed;
	} else {
		command->status = RCommand::WasTried;
	}
	
	command->output_offset = outfile_offset;
	command->error_offset = errfile_offset;;
	
	qDebug ("output");
	QString temp;
	while (!outfile.atEnd ()) {
		outfile.readLine (temp, 2048);
		qDebug (temp);
		command->_output.append (temp);
		command->status |= RCommand::HasOutput;
	}
	command->output_length = outfile.at () - command->output_offset;

	qDebug ("error");
	temp = "";
	while (!errfile.atEnd ()) {
		errfile.readLine (temp, 2048);
		qDebug (temp);
		command->_error.append (temp);
		command->status |= RCommand::HasError;
	}
	command->error_length = errfile.at () - command->error_offset;
}
