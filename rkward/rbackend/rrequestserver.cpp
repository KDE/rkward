/***************************************************************************
                          rrequestserver  -  description
                             -------------------
    begin                : Fri Sep 3 2004
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
#include "rrequestserver.h"

#include <qhostaddress.h>
#include <qstring.h>

#include "rrequesthandler.h"

#include "../debug.h"

//static
bool RRequestServer::connected = false;

//0x7f000001 is 127.0.0.1
RRequestServer::RRequestServer (QObject *parent) : QServerSocket (QHostAddress (0x7f000001), 0, 1, parent) {
	RK_TRACE (RBACKEND);
	if (!ok ()) {
		RK_DO (qDebug ("Failed to set up RRequestServer!"), RBACKEND, DL_ERROR);
	} else {
		RK_DO (qDebug ("Started RRequestServer"), RBACKEND, DL_DEBUG);
	}
}

RRequestServer::~RRequestServer () {
	RK_TRACE (RBACKEND);
}

void RRequestServer::newConnection (int socket) {
	RK_TRACE (RBACKEND);
	RK_DO (qDebug ("Received new TCP connection on socket %d", socket), RBACKEND, DL_DEBUG);
	if (!connected) {
		new RRequestHandler (socket, this);
		connected = true;
	} else {
		new RRequestHandler (socket, this, true);
	}
}

#include "rrequestserver.moc"
