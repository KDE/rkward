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

#ifndef RKRBACKENDPROTOCOL_BACKEND_H
#define RKRBACKENDPROTOCOL_BACKEND_H

#include "rkrbackendprotocol_shared.h"

class QThread;

class RKRBackendProtocolBackend {
public:
	static bool inRThread ();
	static QString dataDir () { return _instance->data_dir; };

	RKRBackendProtocolBackend (const QString &data_dir);
	~RKRBackendProtocolBackend ();
protected:
friend class RKRBackendProtocolFrontend;
friend class RKRBackend;
friend class RKRBackendThread;
	void sendRequest (RBackendRequest *request);
	static void msleep (int delay);
	static void interruptProcessing ();
	static RKRBackendProtocolBackend* instance () { return _instance; };
	QString data_dir;
private:
	static RKRBackendProtocolBackend* _instance;
	QThread *r_thread;
#ifndef Q_WS_WIN
	Qt::HANDLE r_thread_id;
#endif
};

#endif
