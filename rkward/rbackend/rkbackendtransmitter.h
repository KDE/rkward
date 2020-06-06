/***************************************************************************
                          rkbackendtransmitter  -  description
                             -------------------
    begin                : Thu Nov 18 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
public slots:
	void doExit(); // To be called from main thread via QMetaMethod::invoke
private:
	void timerEvent (QTimerEvent *event) override;
	void flushOutput (bool force);
	QList<RBackendRequest*> current_sync_requests;	// pointers to the request that we expect a reply for. Yes, internally, this can be several requests.
	QString servername;
	int flushtimerid;
};

#endif
