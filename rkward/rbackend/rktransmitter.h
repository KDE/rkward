/***************************************************************************
                          rktransmitter  -  description
                             -------------------
    begin                : Thu Nov 18 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#ifndef RKTRANSMITTER_H
#define RKTRANSMITTER_H

#include "rkrbackendprotocol_shared.h"

#include <QThread>
#include <QByteArray>

#ifndef RKWARD_SPLIT_PROCESS
#	error This should only be compiled for split process backends.
#endif

/** functions for serialization / unserialization of communication between backend and frontend.
NOTE: This could really be a namespace, instead of a class, but "friending" a class is simply easier... */
class RKRBackendSerializer {
public:
	static QByteArray serialize (const RBackendRequest &request);
	static RBackendRequest *unserialize (const QByteArray &buffer);

private:
	static void serializeOutput (const ROutputList &list, QDataStream &stream);
	static void serializeData (const RData &data, QDataStream &stream);
	static void serializeProxy (const RCommandProxy &proxy, QDataStream &stream);
	static ROutputList* unserializeOutput (QDataStream &stream);
	static RData* unserializeData (QDataStream &stream);
	static RCommandProxy* unserializeProxy (QDataStream &stream);
};

class QLocalSocket;
/** The base class for the frontend- and backend transmitters */
class RKAbstractTransmitter : public QThread {
Q_OBJECT
public:
	static RKAbstractTransmitter* instance () { return _instance; };
	virtual ~RKAbstractTransmitter ();
protected:
	RKAbstractTransmitter ();

	virtual void writeRequest (RBackendRequest *request) = 0;
	virtual void requestReceived (RBackendRequest *request) = 0;
	virtual void handleTransmissionError (const QString &message) = 0;

	void transmitRequest (RBackendRequest *request);
	void customEvent (QEvent *e);
	void setConnection (QLocalSocket *connection);
	QLocalSocket *connection;
private slots:
	/** Note: this blocks until a compelete request has been received. Connected to the "readyRead"-signal of the connection. Calls requestReceived() once the request has been read. */
	void fetchTransmission ();
	void disconnected ();
private:
static RKAbstractTransmitter* _instance;
};

#endif
