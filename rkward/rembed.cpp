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

REmbed::REmbed(const char* r_home) : REmbedInternal() {
	startR (r_home);
	outfile_offset = 0;
	errfile_offset = 0;
	
	outfile.setName ("r_out");
	if (!outfile.open (IO_ReadOnly)) {
		qDebug ("can't open outfile! will lock up soon");
	}
	errfile.setName ("r_err");
	if (!errfile.open (IO_ReadOnly)) {
		qDebug ("can't open errfile! will lock up soon");
	}
}


REmbed::~REmbed() {
	outfile.close ();
	errfile.close ();
}

void REmbed::runCommand (RCommand *command) {
	qDebug ("TODO: REmbed::runCommand: Error-handling");

	QFile file("r_in");
	if (file.open(IO_WriteOnly)) {
		QTextStream stream(&file);
		stream << command->command () << "\n";
		file.close();
	}
	
	if (!runCommandInternal ("source (\"r_in\")")) {
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
