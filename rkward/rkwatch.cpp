/***************************************************************************
                          rkwatch.cpp  -  description
                             -------------------
    begin                : Sun Nov 3 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#include "rkwatch.h"

#include "rinterface.h"
#include "rcommand.h"

#include <qtextedit.h>
#include <qpushbutton.h>
#include <qfont.h>

RKwatch::RKwatch(RInterface *parent) {
	connect (submit, SIGNAL (clicked ()), this, SLOT (submitCommand ()));
	connect (clear_commands, SIGNAL (clicked ()), this, SLOT (clearCommand ()));
	r_inter = parent;
	watch->setReadOnly (true);

	// set a fixed width font
    QFont font ("Courier");
	watch->setCurrentFont (font);
	commands->setCurrentFont (font);
}

RKwatch::~RKwatch(){
}

void RKwatch::addInput (RCommand *command) {
// TODO: make colors/styles configurable
	if (command->type () & RCommand::User) {
		watch->setColor (Qt::red);
	} else if (command->type () & RCommand::Sync) {
		watch->setColor (Qt::gray);
	} else if (command->type () & RCommand::Plugin) {
		watch->setColor (Qt::blue);
	}

	watch->append ("---------------------------\n");
	watch->append ("Issueing command:\n");
	watch->setItalic (true);

	watch->append (command->command ());

	watch->setItalic (false);
}

void RKwatch::addOutput (RCommand *command) {
	watch->append ("---------------------------\n");
	watch->append ("Got reply:");
    watch->setBold (true);

	watch->append (command->reply ());

	watch->setBold (false);	
	watch->setColor (Qt::black);
}

void RKwatch::clearCommand () {
	commands->clear ();
}

void RKwatch::submitCommand () {
	r_inter->issueCommand (new RCommand (commands->text (), RCommand::User));
	commands->clear ();
}

void RKwatch::enableSubmit () {
	submit->setEnabled (true);
}

void RKwatch::disableSubmit () {
	submit->setEnabled (false);
}