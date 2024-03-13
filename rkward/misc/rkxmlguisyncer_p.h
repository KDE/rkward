/*
rkxmlguisyncer_p.h - This file is part of the RKWard project. Created: Wed Aug 5 2009
SPDX-FileCopyrightText: 2009 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKXMLGUISYNCER_P_H
#define RKXMLGUISYNCER_P_H

#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <kdirwatch.h>

#include <QDir>
#include <QMultiHash>
#include <QSet>
#include <QTimer>

/** For internal use by RKXMLGUISyncer, only */
class RKXMLGUISyncerNotifier : public QObject {
Q_OBJECT
public:
	explicit RKXMLGUISyncerNotifier (QObject *parent) : QObject (parent) {};
	~RKXMLGUISyncerNotifier () {};

	void emitChangeSignal (KXMLGUIClient *client) { Q_EMIT changed(client); };
Q_SIGNALS:
	void changed (KXMLGUIClient *client);
};

class RKXMLGUISyncerPrivate : public QObject {
Q_OBJECT
public:
	RKXMLGUISyncerPrivate () {
		file_watcher = KDirWatch::self ();
		connect (file_watcher, &KDirWatch::dirty, this, &RKXMLGUISyncerPrivate::uiRcFileChanged);
	
		connect (&rebuild_guis_timer, &QTimer::timeout, this, &RKXMLGUISyncerPrivate::rebuildGUIs);
		rebuild_guis_timer.setSingleShot (true);
	}
	~RKXMLGUISyncerPrivate () {};

	/** Internally we store the actionCollection() of each KXMLGUIClient, instead of a pointer to the client, itself. This is because KXMLGUIClient is not a QObject, and so we cannot safely detect its destruction. */
	QMultiHash<QString, KActionCollection*> client_map;
	QMultiHash<KActionCollection*, RKXMLGUISyncerNotifier*> notifier_map;

	QSet<KXMLGUIFactory*> affected_factories;
	QTimer rebuild_guis_timer;

	KDirWatch *file_watcher;
public Q_SLOTS:
	void uiRcFileChanged (const QString &path);
	void actionCollectionDestroyed (QObject *object);
	void guiFactoryDestroyed (QObject *object);
	void rebuildGUIs ();
};

#endif

