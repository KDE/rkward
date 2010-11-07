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

class RKRBackendProtocolBackend {
public:
	static bool inRThread ();
protected:
friend class RKRBackendProtocolFrontend;
friend class RKRBackend;
friend class RKRBackendThread;
	RKRBackendProtocolBackend ();
	~RKRBackendProtocolBackend ();

	void sendRequest (RBackendRequest *request);
	static void msleep (int delay);
	static void interruptProcessing ();
	static RKRBackendProtocolBackend* instance () { return _instance; };
private:
	static RKRBackendProtocolBackend* _instance;
};

#endif
