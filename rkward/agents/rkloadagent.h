/***************************************************************************
                          rkloadagent  -  description
                             -------------------
    begin                : Sun Sep 5 2004
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
#ifndef RKLOADAGENT_H
#define RKLOADAGENT_H

#include <qobject.h>
#include "../rbackend/rcommandreceiver.h"

#include <qstring.h>
#include <kurl.h>

/** The RKLoadAgent is really a rather simple agent. All it needs to do is display an error message, if loading fails. No further action is required. Like all
agents, the RKLoadAgent self-destructs when done.
@author Thomas Friedrichsmeier
*/
class RKLoadAgent : public QObject, public RCommandReceiver {
	Q_OBJECT
public:
	RKLoadAgent (const KURL &url, bool merge=false);

	~RKLoadAgent ();
protected:
	void rCommandDone (RCommand *command);
private:
/// needed if file to be loaded is remote
	QString tmpfile;
};

#endif
