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
#ifndef RREQUESTSERVER_H
#define RREQUESTSERVER_H

#include <qserversocket.h>

/**
Requests from R to RKWard are handled over a TCP-socket connection. This class is responsible for handling an incoming connection request (probably there will only ever be one request per session?) from R, and creating an RRequestHandler to take care of the rest of the communication.

@author Thomas Friedrichsmeier
*/
class RRequestServer : public QServerSocket {
Q_OBJECT
public:
	RRequestServer (QObject *parent);

	~RRequestServer ();

	void newConnection (int socket);
private:
/// used to allow only a single connection
	static bool connected;
};

#endif
