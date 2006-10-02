/***************************************************************************
                          robjectlist  -  description
                             -------------------
    begin                : Wed Aug 18 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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
class REnvironmentObject;

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
	
	QString getFullName () { return QString::null; };
	QString makeChildName (const QString &short_child_name) { return short_child_name; };
	/** reimplemented from RContainerObject: do nothing. The object-list has no meta data. */
	void writeMetaData (RCommandChain *) {};
	
	RCommandChain *getUpdateCommandChain () { return update_chain; };

	/** reimplemented from RContainerObject to search the environments in search order */
	RObject *findObject (const QString &name, bool canonified=false);

	KURL getWorkspaceURL () { return current_url; };
public slots:
	void timeout ();
signals:
/// emitted when the list of objects is about to be updated	// TODO: remove me
	void updateStarted ();
/// emitted when the list of objects has been updated	// TODO: remove me
	void updateComplete ();
protected:
/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString removeChildCommand (RObject *object);
	void removeChild (RObject *object, bool removed_in_workspace);
/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString renameChildCommand (RObject *object, const QString &new_name);
/// reimplemented from RContainerObject to emit a change signal
	void objectsChanged ();
	bool updateStructure (RData *new_data);
	void rCommandDone (RCommand *command);
	void updateEnvironments (QString *env_names, unsigned int env_count);
private:
	friend class RKLoadAgent;
	friend class RKSaveAgent;
	void setWorkspaceURL (const KURL &url) { current_url = url; };
	QTimer *update_timer;
	
	RCommandChain *update_chain;

	REnvironmentObject **toplevel_environments;
	unsigned int num_toplevel_environments;

	KURL current_url;
};

/**
\page RepresentationOfRObjectsInRKWard Representation of R objects in RKWard
\brief How objects in R space are represented in RKWard

Due to primarily two reasons, RKWard needs to keep it's own list of objects in the R workspace. The first, and most important reason is threading: R objects might be modified or even removed in the R backend, while the GUI thread is trying to access them. Since we have no control over what's going on inside R, this cannot be solved with a simple mutex. So rather, we copy a representation into memory accessed by the GUI thread only (in the future, maybe the backend thread will get access to this representation for more efficient updating, but still a representation separate from that kept in R itself is needed).

The second reason is that R and Qt includes clash, and we cannot easily use R SEXPs directly in Qt code.

RKWard then uses an own specialized description of R objects. This is slightly more abstracted than objects in R, but stores the most important information about each object, and of course the hierarchical organization of objects.

TODO: write me!
	
@see RObject
@see RKVariable
@see RContainerObject
@see RObjectList
@see RKModificationTracker
 
 */
 
#endif
