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

#include "rkwatch.h"
#include "rcommand.h"
#include "rthread.h"
#include "rkward.h"

RInterface::RInterface(RKwardApp *parent){
	app = parent;
	
	r_thread = new RThread (this);
	r_thread->start ();
	
	watch = new RKwatch (this);
	watch->show ();
	watch->lower ();
}

RInterface::~RInterface(){
	delete watch;
}

void RInterface::customEvent (QCustomEvent *e) {
	if (e->type () == RCOMMAND_IN_EVENT) {
		watch->addInput (static_cast <RCommand *> (e->data ()));
	} else if (e->type () == RCOMMAND_OUT_EVENT) {
		RCommand *command = static_cast <RCommand *> (e->data ());
		watch->addOutput (command);
		command->finished ();
	} else if ((e->type () == RIDLE_EVENT)) {
		app->setRStatus (false);	
	} else if ((e->type () == RBUSY_EVENT)) {
		app->setRStatus (true);	
	}
}
