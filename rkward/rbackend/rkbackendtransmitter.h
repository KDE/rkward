/*
rkbackendtransmitter - This file is part of the RKWard project. Created: Thu Nov 18 2010
SPDX-FileCopyrightText: 2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKBACKENDTRANSMITTER_H
#define RKBACKENDTRANSMITTER_H

#include "rktransmitter.h"

/** Private class used by the RKRBackendProtocol, in case of the backend running in a split process.
This will be used as the secondary thread, and takes care of serializing, sending, receiving, and unserializing requests. */
class RKRBackendTransmitter : public RKAbstractTransmitter {
Q_OBJECT
public:
	RKRBackendTransmitter (const QString &servername, const QString &token);
	~RKRBackendTransmitter ();

	void publicmsleep (int delay) { msleep (delay); };

	void run () override;

	void writeRequest (RBackendRequest *request) override;
	void requestReceived (RBackendRequest *request) override;
	void handleTransmissionError (const QString &message) override;
public Q_SLOTS:
	void doExit(); // To be called from main thread via QMetaMethod::invoke
private:
	void timerEvent (QTimerEvent *event) override;
	void flushOutput (bool force);
	QList<RBackendRequest*> current_sync_requests;	// pointers to the request that we expect a reply for. Yes, internally, this can be several requests.
	QString servername;
	int flushtimerid;
};

#endif
