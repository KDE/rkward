/***************************************************************************
                          rrequesthandler  -  description
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
#ifndef RREQUESTHANDLER_H
#define RREQUESTHANDLER_H

#include <qsocket.h>
#include <qstring.h>

/**
This class (created by RRequestServer) is responsible for handling requests sent from R to RKWard via a TCP socket connection.

@author Thomas Friedrichsmeier
*/
class RRequestHandler : public QSocket
{
Q_OBJECT
public:
	RRequestHandler (int socket, QObject *parent, bool reject=false);

	~RRequestHandler ();
public slots:
	void readFromR ();
	void connectionTerminated ();
private:
	QString eof_string;
};

#endif
