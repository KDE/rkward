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
#ifndef RKEDITOBJECTAGENT_H
#define RKEDITOBJECTAGENT_H

#include <qobject.h>
#include "../rbackend/rcommandreceiver.h"

#include <qstring.h>
#include <qstringlist.h>

/** This agent gets called, when an rk.edit() command was run in the backend. The purpose is to first update the structure information for the object(s), and then try to open it/them.

@author Thomas Friedrichsmeier
*/
class RKEditObjectAgent : public QObject, public RCommandReceiver {
	Q_OBJECT
public:
	RKEditObjectAgent (const QStringList &object_names, RCommandChain *chain);

	~RKEditObjectAgent ();
protected:
	void rCommandDone (RCommand *command);
private:
	QStringList object_names;
	int done_command_id;
};

#endif
