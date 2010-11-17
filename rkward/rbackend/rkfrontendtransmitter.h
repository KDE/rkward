/***************************************************************************
                          rkfrontendtransmitter  -  description
                             -------------------
    begin                : Thu Nov 04 2010
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

#ifndef RKFRONTENDTRANSMITTER_H
#define RKFRONTENDTRANSMITTER_H

#include "rkrbackendprotocol_shared.h"

#ifdef RKWARD_THREADED
#	error This file should only be compiled for split process backends!
#endif

#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QThread>

class RKFrontendTransmitter : public QThread, public RKROutputBuffer {
Q_OBJECT
public:
	RKFrontendTransmitter ();
	~RKFrontendTransmitter ();

	static RKFrontendTransmitter* instance () { return _instance; };

	void run ();

	bool doMSleep (int delay) {
		msleep (delay);
		return true;
	};
	void writeRequest (RBackendRequest *request);
	void customEvent (QEvent *e);
private slots:
	void connectAndEnterLoop ();
	void newProcessOutput ();
	void newConnectionData ();
	void backendExit (int exitcode, QProcess::ExitStatus exitstatus);
	void connectionStateChanged (QLocalSocket::LocalSocketState state);
private:
	void handleTransmitError (const QString &message);

	int current_request_length;
	QProcess backend;
	QLocalServer server;
	QLocalSocket* connection;
	static RKFrontendTransmitter *_instance;
};

#endif

