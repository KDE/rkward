/***************************************************************************
                          rkgraphicsdevice_frontendtransmitter  -  description
                             -------------------
    begin                : Mon Mar 18 20:06:08 CET 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier 
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

#ifndef RKGRAPHICSDEVICE_FRONTENDTRANSMITTER_H
#define RKGRAPHICSDEVICE_FRONTENDTRANSMITTER_H

#include "rkgraphicsdevice_protocol_shared.h"

class QIODevice;
class QLocalServer;

/** Handles the frontend side of RKWard Graphics Device transmissions. Since the
 * frontend has a running Qt event loop, We can use simple signals and slots, here. */
class RKGraphicsDeviceFrontendTransmitter : public QObject {
	Q_OBJECT
public:
	RKGraphicsDeviceFrontendTransmitter ();
	~RKGraphicsDeviceFrontendTransmitter ();
	QString serverName () const { return server_name; };
public slots:
	void newData ();
	void newConnection ();
private:
	void setupServer ();
	QString server_name;
	QIODevice *connection;
	QLocalServer *local_server;
	RKAsyncDataStreamHelper streamer;
};

#endif
