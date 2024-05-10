/*
rkrbackendprotocol - This file is part of the RKWard project. Created: Thu Nov 04 2010
SPDX-FileCopyrightText: 2010-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKRBACKENDPROTOCOL_BACKEND_H
#define RKRBACKENDPROTOCOL_BACKEND_H

#include "rkrbackendprotocol_shared.h"

class QThread;
class RKRBackendTransmitter;

extern "C"
#ifdef Q_OS_WIN
	__declspec(dllexport)
#else
	__attribute__((__visibility__("default")))
#endif
int do_main(int, char**, void*, void* (*)(void*, const char*));

class RKRBackendProtocolBackend {
public:
	static bool inRThread ();
	static QString dataDir () { return _instance->data_dir; };
	static QString rkdServerName () { return _instance->rkd_server_name; };
	static QString backendDebugFile ();
	static void doExit();

	RKRBackendProtocolBackend (const QString &data_dir, const QString &rkd_server_name);
	~RKRBackendProtocolBackend ();
protected:
friend class RKRBackendProtocolFrontend;
friend class RKRBackend;
friend class RKRBackendThread;
friend class RKRBackendTransmitter;
friend int main(int, char**);
friend int do_main(int, char**, void*, void* (*)(void*, const char*));
	void sendRequest (RBackendRequest *request);
	static void msleep (int delay);
	static RKRBackendProtocolBackend* instance () { return _instance; };
	QString data_dir;
private:
	QString rkd_server_name;
	static RKRBackendProtocolBackend* _instance;
	static RKRBackendTransmitter* p_transmitter;
	QThread *r_thread;
#ifndef Q_OS_WIN
	friend void completeForkChild ();
	Qt::HANDLE r_thread_id;
#endif
};

#endif
