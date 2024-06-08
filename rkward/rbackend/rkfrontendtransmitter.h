/*
rkfrontendtransmitter - This file is part of the RKWard project. Created: Thu Nov 04 2010
SPDX-FileCopyrightText: 2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKFRONTENDTRANSMITTER_H
#define RKFRONTENDTRANSMITTER_H

#include "rktransmitter.h"

class QProcess;
class QIODevice;
class QLocalServer;
class RKGraphicsDeviceFrontendTransmitter;
class RKRBackendProtocolFrontend;

class RKFrontendTransmitter : public RKAbstractTransmitter, public RKROutputBuffer {
Q_OBJECT
public:
	RKFrontendTransmitter(RKRBackendProtocolFrontend *frontend);
	~RKFrontendTransmitter ();

	void run () override;

	bool doMSleep (int delay) override {
		msleep (delay);
		return true;
	};
	void writeRequest (RBackendRequest *request) override;
	void requestReceived (RBackendRequest *request) override;
	/** Simple convenience function similar to QIODevice::waitForReadyRead(), but waiting for a full line to be available.
	    In particular on Windows, we often receive _less_ than a full line per chunk. */
	static QString waitReadLine (QIODevice *con, int msecs);
private Q_SLOTS:
	void connectAndEnterLoop ();
	void backendExit (int exitcode);
private:
	void handleTransmissionError (const QString &message) override;

	bool quirkmode;
	QProcess* backend;
	QLocalServer* server;
	RKRBackendProtocolFrontend* frontend;
	RKGraphicsDeviceFrontendTransmitter* rkd_transmitter;

	QString resolveRSpecOrFail(QString input);
	void detectAndCheckRBinary();
};

#endif

