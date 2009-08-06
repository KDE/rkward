/***************************************************************************
                          rkxmlguisyncer.cpp  -  description
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

#include "rkxmlguisyncer.h"

#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <kdirwatch.h>
#include <kapplication.h>

#include <QDir>

#include "../debug.h"

RKXMLGUISyncer* RKXMLGUISyncer::syncer = 0;

//static
RKXMLGUISyncer *RKXMLGUISyncer::self () {
	RK_TRACE (MISC);

	if (!syncer) syncer = new RKXMLGUISyncer ();
	return syncer;
}

RKXMLGUISyncer::RKXMLGUISyncer () : QObject () {
	RK_TRACE (MISC);

	file_watcher = KDirWatch::self ();
	connect (file_watcher, SIGNAL (dirty(const QString&)), this, SLOT (uiRcFileChanged(const QString&)));

	connect (&rebuild_guis_timer, SIGNAL (timeout()), this, SLOT (rebuildGUIs()));
	rebuild_guis_timer.setSingleShot (true);
}

RKXMLGUISyncer::~RKXMLGUISyncer () {
	RK_TRACE (MISC);
}

void RKXMLGUISyncer::watchXMLGUIClientUIrc (KXMLGUIClient *client, bool recursive) {
	RK_TRACE (MISC);

	KActionCollection *ac = client->actionCollection ();
	QString local_xml_file = client->localXMLFile ();

	// local_xml_file *can* be the name of a directory, if the client->xmlFile() is empty.
	if (ac && (!local_xml_file.isEmpty()) && (!QDir (local_xml_file).exists ())) {
		RK_ASSERT (ac->parentGUIClient () == client);

		if (!client_map.contains (local_xml_file, ac)) {
			if (!client_map.contains (local_xml_file)) {
				file_watcher->addFile (local_xml_file);
			}

			client_map.insertMulti (local_xml_file, ac);
			connect (ac, SIGNAL (destroyed(QObject*)), this, SLOT (actionCollectionDestroyed(QObject*)));
		} // we simply ignore attempts to watch the same client twice
	}

	if (recursive) {
		foreach (KXMLGUIClient *child, client->childClients ()) {
			watchXMLGUIClientUIrc (child, true);
		}
	}
}

void RKXMLGUISyncer::registerChangeListener (KXMLGUIClient *watched_client, QObject *receiver, const char *method) {
	RK_TRACE (MISC);

	KActionCollection *ac = watched_client->actionCollection ();

	RKXMLGUISyncerNotifier *notifier = new RKXMLGUISyncerNotifier (0);
	connect (notifier, SIGNAL (changed(KXMLGUIClient*)), receiver, method);

	notifier_map.insertMulti (ac, notifier);
}

void RKXMLGUISyncer::uiRcFileChanged (const QString &path)  {
	RK_TRACE (MISC);

	RK_ASSERT (client_map.contains (path));

	// find affected clients and reload them
	QMultiHash<QString, KActionCollection*>::const_iterator i = client_map.find(path);
	while (i != client_map.constEnd() && i.key() == path) {
		KXMLGUIClient *client = const_cast<KXMLGUIClient*> (i.value ()->parentGUIClient ());
		if (!client) {
			RK_ASSERT (false);
			continue;
		}
		RK_ASSERT (client->localXMLFile () == path);
		client->reloadXML ();
		RK_DO (qDebug ("reloaded client %p for file %s", client, qPrintable (path)), MISC, DL_DEBUG);
		if (client->factory ()) {
			affected_factories.insert (client->factory ());
			connect (client->factory (), SIGNAL (destroyed(QObject*)), this, SLOT (guiFactoryDestroyed(QObject*)));
		}

		// find notifiers listening for this client
		QMultiHash<KActionCollection*, RKXMLGUISyncerNotifier*>::const_iterator n = notifier_map.find(i.value ());
		while (n != notifier_map.constEnd() && n.key() == i.value ()) {
			n.value ()->emitChangeSignal (client);
			++n;
		}

		++i;
	}

	rebuild_guis_timer.start (0);
}

void RKXMLGUISyncer::rebuildGUIs () {
	RK_TRACE (MISC);

	while (!affected_factories.isEmpty ()) {
		KXMLGUIFactory *factory = *(affected_factories.begin ());
		affected_factories.remove (factory);

		RK_DO (qDebug ("rebuilding factory %p", factory), MISC, DL_DEBUG);
		QList<KXMLGUIClient*> clients = factory->clients ();
		for (int i = clients.size () - 1; i >= 0; --i) {
			factory->removeClient (clients[i]);
		}
		for (int i = 0; i < clients.size (); ++i) {
			factory->addClient (clients[i]);
		}
		RK_DO (qDebug ("done rebuilding factory"), MISC, DL_DEBUG);
	}
}

void RKXMLGUISyncer::actionCollectionDestroyed (QObject *object) {
	RK_TRACE (MISC);

	// warning: Do not call any methods on the ac. It is half-destroyed, already.
	KActionCollection *ac = static_cast<KActionCollection*> (object);

	// stop listening for the corresponding client
	QString path_key = client_map.key (static_cast<KActionCollection*> (object));
	client_map.remove (path_key, ac);

	// if there are no futher clients with this path, stop watching it.
	if (!client_map.contains (path_key)) {
		file_watcher->removeFile (path_key);
	}

	// remove any notifiers, too
	RKXMLGUISyncerNotifier* notifier;
	while ((notifier = notifier_map.take (ac))) {
		notifier->deleteLater ();
	}

	RK_DO (qDebug ("action collection destroyed. Still watch %d clients with %d notifiers", client_map.size (), notifier_map.size ()), MISC, DL_DEBUG);
}

void RKXMLGUISyncer::guiFactoryDestroyed (QObject *object) {
	RK_TRACE (MISC);

	affected_factories.remove (static_cast<KXMLGUIFactory*>(object));
}

#include "rkxmlguisyncer.moc"
