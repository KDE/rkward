/***************************************************************************
                          rinterface.cpp  -  description
                             -------------------
    begin                : Fri Nov 1 2002
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

#include "rinterface.h"

#include <qcstring.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "rkwatch.h"
#include "rcommand.h"
#include "rembed.h"

RInterface::RInterface(){
	embeddedR = new REmbed ();
	watch = new RKwatch (this);
	watch->show ();
	command_stack.setAutoDelete (true);
}

RInterface::~RInterface(){
	delete watch;
}

void RInterface::shutdown () {
}

bool RInterface::startR (QStrList &commandline) {
	qDebug ("TODO: cleanup startR!");
}

void RInterface::issueCommand (RCommand *command) {
	command_stack.append (command);
	tryNextCommand ();
}

void RInterface::tryNextCommand () {
	if (command_stack.count ()) {
		write (command_stack.first ());
	}
}

void RInterface::write (RCommand *command) {
	QString output;
	
	embeddedR->runCommand (command);
	watch->addInput (command);
	qDebug ("ran command %d", command->id ());
	
	watch->addOutput (command);
	command->finished ();
	command_stack.removeFirst ();
	
	tryNextCommand ();
}

