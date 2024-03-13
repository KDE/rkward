/*
rktransmitter - This file is part of the RKWard project. Created: Thu Nov 18 2010
SPDX-FileCopyrightText: 2010-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKTRANSMITTER_H
#define RKTRANSMITTER_H

#include "rkrbackendprotocol_shared.h"
#include "rkasyncdatastreamhelper.h"

#include <QThread>
#include <QByteArray>

/** functions for serialization / unserialization of communication between backend and frontend.
NOTE: This could really be a namespace, instead of a class, but "friending" a class is simply easier... */
class RKRBackendSerializer {
public:
	static void serialize (const RBackendRequest &request, QDataStream &buffer);
	static RBackendRequest *unserialize (QDataStream &buffer);

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

/** returns the magic token negotiated between frontend and backend (for validating incoming connections) */
	QString connectionToken () { return token; };
protected:
	RKAbstractTransmitter ();

	virtual void writeRequest (RBackendRequest *request) = 0;
	virtual void requestReceived (RBackendRequest *request) = 0;
	virtual void handleTransmissionError (const QString &message) = 0;

	void transmitRequest (RBackendRequest *request);
	void customEvent (QEvent *e) override;
	void setConnection (QLocalSocket *connection);
	QLocalSocket *connection;
	QString token;
private Q_SLOTS:
	/** Note: this blocks until a complete request has been received. Connected to the "readyRead"-signal of the connection. Calls requestReceived() once the request has been read. */
	void fetchTransmission ();
	void disconnected ();
private:
	static RKAbstractTransmitter* _instance;
	RKAsyncDataStreamHelper<quint64> streamer;
};

#endif
