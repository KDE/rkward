/***************************************************************************
                          rkgraphicsdevice  -  description
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

#include <QAbstractSocket>
#include <QThread>
#include <QMutex>

/** This simple class is responsible for handling the backend side of transmitting data / requests for the RKGraphicsDevice
 Also it provides the namespace for some statics.
 As the protocol is really quite simple (only the backend send requests, only one request at a time), so is the transmitter. */
class RKGraphicsDeviceBackendTransmitter : public QThread {
	RKGraphicsDeviceBackendTransmitter (QAbstractSocket *connection);
	~RKGraphicsDeviceBackendTransmitter ();

	void kill ();
public:
	static QByteArray buffer;
	static QDataStream protocol;
	static QAbstractSocket* connection;
	static QMutex mutex;
private:
	bool alive;
	void run ();
};
