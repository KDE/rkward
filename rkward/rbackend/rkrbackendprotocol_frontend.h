/*
rkrbackendprotocol - This file is part of the RKWard project. Created: Thu Nov 04 2010
SPDX-FileCopyrightText: 2010-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKRBACKENDPROTOCOL_FRONTEND_H
#define RKRBACKENDPROTOCOL_FRONTEND_H

#include "rkrbackendprotocol_shared.h"

#include <QObject>

class RInterface;
class QThread;
class RCommandProxy;

class RKRBackendProtocolFrontend : public QObject {
public:
	explicit RKRBackendProtocolFrontend (RInterface* parent);
	~RKRBackendProtocolFrontend ();

	static void setRequestCompleted (RBackendRequest *request);
	ROutputList flushOutput (bool force);
	static void interruptCommand (int command_id);
	static void sendPriorityCommand (RCommandProxy *proxy);
	void terminateBackend ();
	void setupBackend ();
protected:
/** needed to handle the QEvents, the R thread is sending (notifications on what's happening in the backend thread) */
	void customEvent (QEvent *e) override;
	QThread* main_thread;
private:
	RInterface *frontend;
};

#endif
