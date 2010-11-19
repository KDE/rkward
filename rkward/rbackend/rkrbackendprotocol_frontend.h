/***************************************************************************
                          rkrbackendprotocol  -  description
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

#ifndef RKRBACKENDPROTOCOL_FRONTEND_H
#define RKRBACKENDPROTOCOL_FRONTEND_H

#include "rkrbackendprotocol_shared.h"

#include <QObject>

class RInterface;
class QThread;

class RKRBackendProtocolFrontend : public QObject {
public:
	RKRBackendProtocolFrontend (RInterface* parent);
	~RKRBackendProtocolFrontend ();

	static void setRequestCompleted (RBackendRequest *request);
	ROutputList flushOutput (bool force);
	void interruptProcessing ();
	void terminateBackend ();
	void setupBackend ();
	static RKRBackendProtocolFrontend* instance () { return _instance; };
protected:
/** needed to handle the QEvents, the R thread is sending (notifications on what's happening in the backend thread) */
	void customEvent (QEvent *e);
#ifndef RKWARD_THREADED
	QThread* main_thread;
#endif
private:
	static RKRBackendProtocolFrontend* _instance;
	RInterface *frontend;
};

#endif
