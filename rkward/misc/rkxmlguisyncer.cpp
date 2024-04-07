/*
rkxmlguisyncer.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Wed Aug 5 2009
SPDX-FileCopyrightText: 2009 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkxmlguisyncer.h"
#include "rkxmlguisyncer_p.h"

#include "../debug.h"

RKXMLGUISyncer* RKXMLGUISyncer::syncer = nullptr;

//static
RKXMLGUISyncer *RKXMLGUISyncer::self () {
	RK_TRACE (MISC);

	if (!syncer) syncer = new RKXMLGUISyncer ();
	return syncer;
}

RKXMLGUISyncer::RKXMLGUISyncer () : d (new RKXMLGUISyncerPrivate) {
	RK_TRACE (MISC);
}

RKXMLGUISyncer::~RKXMLGUISyncer () {
	RK_TRACE (MISC);
	delete d;
}

void RKXMLGUISyncer::watchXMLGUIClientUIrc (KXMLGUIClient *client, bool recursive) {
	RK_TRACE (MISC);

	KActionCollection *ac = client->actionCollection ();
	QString local_xml_file = client->localXMLFile ();

	// local_xml_file *can* be the name of a directory, if the client->xmlFile() is empty.
	if (ac && (!local_xml_file.isEmpty()) && (!QDir (local_xml_file).exists ())) {
		RK_ASSERT (ac->parentGUIClient () == client);

		if (!d->client_map.contains (local_xml_file, ac)) {
			if (!d->client_map.contains (local_xml_file)) {
				d->file_watcher->addFile (local_xml_file);
			}

			d->client_map.insert(local_xml_file, ac);
			d->connect (ac, &KActionCollection::destroyed, d, &RKXMLGUISyncerPrivate::actionCollectionDestroyed);
		} // we simply ignore attempts to watch the same client twice
	}

	if (recursive) {
		const auto children = client->childClients();
		for (KXMLGUIClient *child : children) {
			watchXMLGUIClientUIrc(child, true);
		}
	}
}

void RKXMLGUISyncer::registerChangeListener (KXMLGUIClient *watched_client, QObject *receiver, const char *method) {
	RK_TRACE (MISC);

	KActionCollection *ac = watched_client->actionCollection ();

	RKXMLGUISyncerNotifier *notifier = new RKXMLGUISyncerNotifier(nullptr);
	d->connect (notifier, SIGNAL (changed(KXMLGUIClient*)), receiver, method);

	d->notifier_map.insert(ac, notifier);
}



void RKXMLGUISyncerPrivate::uiRcFileChanged (const QString &path)  {
	RK_TRACE (MISC);

	RK_ASSERT (client_map.contains (path));

	// find affected clients and reload them
	QMultiHash<QString, KActionCollection*>::const_iterator i = client_map.constFind(path);
	while (i != client_map.constEnd() && i.key() == path) {
		KXMLGUIClient *client = const_cast<KXMLGUIClient*> (i.value ()->parentGUIClient ());
		if (!client) {
			RK_ASSERT (false);
			continue;
		}
		RK_ASSERT (client->localXMLFile () == path);
		client->reloadXML ();
		RK_DEBUG (MISC, DL_DEBUG, "reloaded client %p for file %s", client, qPrintable (path));
		if (client->factory ()) {
			affected_factories.insert (client->factory ());
			connect (client->factory (), &KXMLGUIFactory::destroyed, this, &RKXMLGUISyncerPrivate::guiFactoryDestroyed);
		}

		// find notifiers listening for this client
		QMultiHash<KActionCollection*, RKXMLGUISyncerNotifier*>::const_iterator n = notifier_map.constFind(i.value ());
		while (n != notifier_map.constEnd() && n.key() == i.value ()) {
			n.value ()->emitChangeSignal (client);
			++n;
		}

		++i;
	}

	rebuild_guis_timer.start (0);
}

void RKXMLGUISyncerPrivate::rebuildGUIs () {
	RK_TRACE (MISC);

	while (!affected_factories.isEmpty ()) {
		KXMLGUIFactory *factory = *(affected_factories.begin ());
		affected_factories.remove (factory);

		RK_DEBUG (MISC, DL_DEBUG, "rebuilding factory %p", factory);
		QList<KXMLGUIClient*> clients = factory->clients ();
		for (int i = clients.size () - 1; i >= 0; --i) {
			factory->removeClient (clients[i]);
		}
		for (int i = 0; i < clients.size (); ++i) {
			factory->addClient (clients[i]);
			if (clients[i]->xmlFile ().isEmpty ()) {
				// somehow config-based settings get lost above, so re-read them, now.
				clients[i]->actionCollection ()->readSettings();
			}
		}
		RK_DEBUG (MISC, DL_DEBUG, "done rebuilding factory");
	}
}

void RKXMLGUISyncerPrivate::actionCollectionDestroyed (QObject *object) {
	RK_TRACE (MISC);

	// warning: Do not call any methods on the ac. It is half-destroyed, already.
	KActionCollection *ac = static_cast<KActionCollection*> (object);

	// stop listening for the corresponding client
	QString path_key = client_map.key (static_cast<KActionCollection*> (object));
	client_map.remove (path_key, ac);

	// if there are no further clients with this path, stop watching it.
	if (!client_map.contains (path_key)) {
		file_watcher->removeFile (path_key);
	}

	// remove any notifiers, too
	RKXMLGUISyncerNotifier* notifier;
	while ((notifier = notifier_map.take (ac))) {
		notifier->deleteLater ();
	}

	RK_DEBUG (MISC, DL_DEBUG, "action collection destroyed. Still watch %d clients with %d notifiers", client_map.size (), notifier_map.size ());
}

void RKXMLGUISyncerPrivate::guiFactoryDestroyed (QObject *object) {
	RK_TRACE (MISC);

	affected_factories.remove (static_cast<KXMLGUIFactory*>(object));
}

