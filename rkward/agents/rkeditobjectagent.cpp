/***************************************************************************
                          rkeditobjectagent  -  description
                             -------------------
    begin                : Fri Feb 16 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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
#include "rkeditobjectagent.h"

#include <klocale.h>
#include <kmessagebox.h>

#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../rbackend/rinterface.h"
#include "../rkward.h"
#include "../windows/rkworkplace.h"

#include "../debug.h"

RKEditObjectAgent::RKEditObjectAgent (const QStringList &object_names, RCommandChain *chain) {
	RK_TRACE (APP);

	RKEditObjectAgent::object_names = object_names;

	// first issue an empty command to trigger an update of the object list
	RKGlobals::rInterface ()->issueCommand (new RCommand (QString::null, RCommand::EmptyCommand | RCommand::ObjectListUpdate, QString::null, this), chain);

	// now add another empty command to find out, when the update is complete
	RCommand *command = new RCommand (QString::null, RCommand::EmptyCommand, QString::null, this);
	done_command_id = command->id ();
	RKGlobals::rInterface ()->issueCommand (command, chain);
}

RKEditObjectAgent::~RKEditObjectAgent () {
	RK_TRACE (APP);
}

void RKEditObjectAgent::rCommandDone (RCommand *command) {
	RK_TRACE (APP);

	if (command->id () == done_command_id) {	
		for (QStringList::const_iterator it = object_names.constBegin (); it != object_names.constEnd (); ++it) {
			QString object_name = *it;
			RObject *obj = RObjectList::getObjectList ()->findObject (object_name);
			if (!(obj && RKWorkplace::mainWorkplace()->editObject (obj))) {
				KMessageBox::information (0, i18n ("The object '%1', could not be opened for editing. Either it does not exist, or RKWard does not support editing this type of object, yet.", object_name), i18n ("Cannot edit '%1'", object_name));
			}
		}
		
		// we're done
		deleteLater ();
	}
}

#include "rkeditobjectagent.moc"
