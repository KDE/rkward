/***************************************************************************
                          robjectlist  -  description
                             -------------------
    begin                : Wed Aug 18 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef ROBJECTLIST_H
#define ROBJECTLIST_H

#include <qobject.h>

#include <qstring.h>
#include <qmap.h>

#include <kurl.h>

#include "rcontainerobject.h"

class QTimer;
class RCommand;
class RCommandChain;
class RKEditor;

/**
This class is responsible for keeping and updating a list of objects in the R-workspace.
Acutally it kind of represents the R-workspace, including methods to save and load the workspace.
It acts as the "document".

@author Thomas Friedrichsmeier
*/
class RObjectList : public QObject, public RContainerObject {
  Q_OBJECT
public:
    RObjectList ();

    ~RObjectList ();
	void updateFromR ();
	
	void createFromR (RContainerObject *parent, const QString &cname);
	
	QString getFullName () { return ""; };
	QString makeChildName (const QString &short_child_name) { return short_child_name; };
	/** reimplemented from RContainerObject: do nothing. The object-list has no meta data. */
	void writeMetaData (RCommandChain *) {};
	
	RCommandChain *getUpdateCommandChain () { return update_chain; };
	
	void childUpdateComplete ();

	RObject *findObject (const QString &full_name);
	
	KURL getWorkspaceURL () { return current_url; };
public slots:
	void timeout ();
signals:
/// emitted when the list of objects is about to be updated
	void updateStarted ();
/// emitted when the list of objects has been updated
	void updateComplete ();
protected:
	void rCommandDone (RCommand *command);
/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	void renameChild (RObject *object, const QString &new_name);
/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	void removeChild (RObject *object, bool removed_in_workspace);
/// reimplemented from RContainerObject to emit a change signal
	void objectsChanged ();
private:
	friend class RKLoadAgent;
	void setWorkspaceURL (const KURL &url) { current_url = url; };
	QTimer *update_timer;
	
	struct PendingObject {
		QString name;
		RContainerObject *parent;
	};
	
	QMap<RCommand*, PendingObject*> pending_objects;
	
	RCommandChain *update_chain;

	KURL current_url;
};

#endif
