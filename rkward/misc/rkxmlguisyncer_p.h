/***************************************************************************
                          rkxmlguisyncer_p.h  -  description
                             -------------------
    begin                : Wed Aug 5 2009
    copyright            : (C) 2009 by Thomas Friedrichsmeier
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

#ifndef RKXMLGUISYNCER_P_H
#define RKXMLGUISYNCER_P_H

#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <kdirwatch.h>
#include <kapplication.h>

#include <QDir>
#include <QMultiHash>
#include <QSet>
#include <QTimer>

/** For internal use by RKXMLGUISyncer, only */
class RKXMLGUISyncerNotifier : public QObject {
Q_OBJECT
public:
	RKXMLGUISyncerNotifier (QObject *parent) : QObject (parent) {};
	~RKXMLGUISyncerNotifier () {};

	void emitChangeSignal (KXMLGUIClient *client) { changed (client); };
signals:
	void changed (KXMLGUIClient *client);
};

class RKXMLGUISyncerPrivate : public QObject {
Q_OBJECT
public:
	RKXMLGUISyncerPrivate () {
		file_watcher = KDirWatch::self ();
		connect (file_watcher, SIGNAL (dirty(const QString&)), this, SLOT (uiRcFileChanged(const QString&)));
	
		connect (&rebuild_guis_timer, SIGNAL (timeout()), this, SLOT (rebuildGUIs()));
		rebuild_guis_timer.setSingleShot (true);
	}
	~RKXMLGUISyncerPrivate () {};

	/** Internally we store the actionCollection() of each KXMLGUIClient, instead of a pointer to the client, itself. This is because KXMLGUIClient is not a QObject, and so we cannot safely detect its destruction. */
	QMultiHash<QString, KActionCollection*> client_map;
	QMultiHash<KActionCollection*, RKXMLGUISyncerNotifier*> notifier_map;

	QSet<KXMLGUIFactory*> affected_factories;
	QTimer rebuild_guis_timer;

	KDirWatch *file_watcher;
public slots:
	void uiRcFileChanged (const QString &path);
	void actionCollectionDestroyed (QObject *object);
	void guiFactoryDestroyed (QObject *object);
	void rebuildGUIs ();
};

#endif

