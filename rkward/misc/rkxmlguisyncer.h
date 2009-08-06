/***************************************************************************
                          rkxmlguisyncer.h  -  description
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

#ifndef RKXMLGUISYNCER_H
#define RKXMLGUISYNCER_H

#include <QObject>
#include <QMultiHash>
#include <QSet>
#include <QTimer>

class KXMLGUIClient;
class KActionCollection;
class KDirWatch;
class KXMLGUIFactory;

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

/** This class listens for changes in the XMLGUI-configuration files of registered KXMLGUIClients. It then takes care of updating those KXMLGUIClients. */
class RKXMLGUISyncer : public QObject {
Q_OBJECT
public:
	/** Returns the single static instance of the syncer. If the instance did not exit, yet, it is created, now. */
	static RKXMLGUISyncer *self ();

	/** start watching for changes in the given KXMLGUIClient's local ui.rc .
	@param client The client to monitor
	@param recursive Also monitor the client's child clients? */
	void watchXMLGUIClientUIrc (KXMLGUIClient *client, bool recursive=true);

	/** You can use this function to receive a signal, when an KXMLGUIClient's ui.rc file has been reloaded (after the reload, before the factory is told to rebuild).
	@param watched_client The client to monitor. This needs to be registered using watchXMLGUIClientUIrc, first.
	@param receiver QObject to receive notification
	@param method slot to recieve notification. This needs to have the signature
	\code
		void someSlotName (KXMLGUIClient *changed_client);
	\endcode
	*/
	void registerChangeListener (KXMLGUIClient *watched_client, QObject *receiver, const char *method);
private slots:
	void uiRcFileChanged (const QString &path);
	void actionCollectionDestroyed (QObject *object);
	void guiFactoryDestroyed (QObject *object);
	void rebuildGUIs ();
protected:
	RKXMLGUISyncer ();
	~RKXMLGUISyncer ();
private:
	/** Internally we store the actionCollection() of each KXMLGUIClient, instead of a pointer to the client, directly. This is because KXMLGUIClient is not a QObject, and so we cannot safely detect its destruction. */
	QMultiHash<QString, KActionCollection*> client_map;
	QMultiHash<KActionCollection*, RKXMLGUISyncerNotifier*> notifier_map;

	QSet<KXMLGUIFactory*> affected_factories;
	QTimer rebuild_guis_timer;

	KDirWatch *file_watcher;
	static RKXMLGUISyncer *syncer;
};

#endif
