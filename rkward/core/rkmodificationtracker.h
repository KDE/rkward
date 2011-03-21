/***************************************************************************
                          rkmodificationtracker  -  description
                             -------------------
    begin                : Tue Aug 31 2004
    copyright            : (C) 2004, 2007, 2011 by Thomas Friedrichsmeier
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
#ifndef RKMODIFICATIONTRACKER_H
#define RKMODIFICATIONTRACKER_H

#include <qobject.h>
#include <qstring.h>
#include <QAbstractItemModel>
#include <QMultiHash>

#include "robject.h"

class RKEditor;
class RObject;
class RKModificationTracker;

/** Base class for classes that need to know when certain objects have been changed in some way. */
class RObjectListener {
public:
	enum NotificationType {
		ObjectRemoved=1,
		ChildAdded=2,
		ChildMoved=4,	/** < a child has changed its position (index) *within* the parent */
		MetaChanged=8,
		DataChanged=16
	};
	enum ListenerType {
		DataModel,	/** < listener is an RKVarEditModel */
		ObjectView,
		Other
	};

	ListenerType listenerType () const { return type; };
	bool wantsNotificationType (NotificationType type) const { return (notifications & type); };
protected:
	RObjectListener (ListenerType type);
	virtual ~RObjectListener ();

friend class RKModificationTracker;
	/** reimplement this, if you are listening for an object with notification type ObjectRemoved. The default implementation does nothing and raises an assert. This gets sent *before* the child is actually removed, so you can safely query it for information in this call, but directly after the pointer will become invalid. Make sure to call stopListenForObject(), when you receive this notification. */
	virtual void objectRemoved (RObject* removed);
	/** reimplement this, if you are listening for an object with notification type ChildAdded. The default implementation does nothing and raises an assert. This notification is sent *after* the child was added. */
	virtual void childAdded (int index, RObject* parent);
	/** reimplement this, if you are listening for an object with notification type ChildMoved. The default implementation does nothing and raises an assert. This notification is sent *after* the child was moved, so it is now at the new_index. */
	virtual void childMoved (int old_index, int new_index, RObject* parent);
	/** reimplement this, if you are listening for an object with notification type MetaChanged. The default implementation does nothing and raises an assert. This notification is sent *after* the object has changed. */
	virtual void objectMetaChanged (RObject* changed);
	/** reimplement this, if you are listening for an object with notification type DataChanged. The default implementation does nothing and raises an assert. This notification is sent *after* the object has changed. */
	virtual void objectDataChanged (RObject* object, const RObject::ChangeSet *changes);

	void listenForObject (RObject* object);
	void stopListenForObject (RObject* object);
	void addNotificationType (NotificationType type) { notifications |= type; };
private:
	ListenerType type;
	int notifications;
	int num_watched_objects;
};

/** An item model for the RObjectList . Technically this is the base class for RKModificationTracker. The two could be merged, fully, but this way, it's a little easier to see what belongs where, logically. */
class RKObjectListModel : public QAbstractItemModel {
public:
	enum Column {
		NameColumn=0,
		LabelColumn,
		TypeColumn,
		ClassColumn,
		ColumnCount = ClassColumn + 1
	};
protected:
	RKObjectListModel (QObject *parent);
	virtual ~RKObjectListModel ();
public:
	/** implements QAbstractItemModel::index() */
	QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex ()) const;
	/** implements QAbstractItemModel::parent() */
	QModelIndex parent (const QModelIndex& index) const;
	/** implements QAbstractItemModel::rowCount() */
	int rowCount (const QModelIndex& parent = QModelIndex ()) const;
	/** implements QAbstractItemModel::columnCount(). This is identical for all items */
	int columnCount (const QModelIndex& parent = QModelIndex ()) const;
	/** implements QAbstractItemModel::data() */
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
	/** reimplemented from  QAbstractItemModel::headerData() to provide column names */
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	/** reimplemented from  QAbstractItemModel::canFetchMore() */
	bool canFetchMore (const QModelIndex &parent) const;
	/** reimplemented from  QAbstractItemModel::fetchMore() */
	void fetchMore (const QModelIndex &parent);
	/** reimplemented from  QAbstractItemModel::hasChildren() */
	bool hasChildren (const QModelIndex &parent = QModelIndex ()) const; 

	/** convenience function to create an index for a particular object */
	QModelIndex indexFor (RObject *object) const;
};


/**
This class takes care of propagating object-modifications to all editors/variable-browsers etc. that need to know about them. For instance, if an object was removed in the R-workspace, the RObjectList will notify the RKModificationTracker. The modification tracker will then find out, whether there are editor(s) currently editing the removed object. If so, it will prompt the user what to do. Or, if an object is renamed in an editor, the RKModificationTracker will find out, whether the object is opened in further editors (not possible, yet) and tell those to update accordingly. It will further emit signals so the RObjectBrowser and RKVarselector(s) can update their object-lists.

@author Thomas Friedrichsmeier
*/
class RKModificationTracker : public RKObjectListModel {
public:
	RKModificationTracker (QObject *parent);

	~RKModificationTracker ();
	
/** the given object should be removed (either it was removed in the R-workspace, or the user requests removal of the object in an editor or the RObjectList). First, if the object is being edited somewhere, the user will get a chance to object to the removal. If the user does not object, the RKModificationTracker will remove the object and notify all interested listeners that the object really was removed. When calling from the RObjectList, you will likely set removed_in_workspace to true, to signal that the object-data is already gone in the workspace. */
	bool removeObject (RObject *object, RKEditor *editor=0, bool removed_in_workspace=false);
/** essentially like the above function, but requests a renaming of the object. Will also take care of finding out, whether the name is valid and promting for a different name otherwise. */
	void renameObject (RObject *object, const QString &new_name);
/** the object's meta data was modified. Tells all editors and lists containing the object to update accordingly. */
	void objectMetaChanged (RObject *object);
/** the object's data was modified. Tells all editors and lists containing the object to update accordingly. The ChangeSet given tells which parts of the data have to be updated. The ChangeSet will get deleted by the RKModificationTracker, when done. */
	void objectDataChanged (RObject *object, RObject::ChangeSet *changes);
/** recursive! */
	void lockUpdates (bool lock);
/** returns (the first) editor that is currently active for this object, or 0, if there is no editor */
	RKEditor* objectEditor (const RObject* object);
private:
	int updates_locked;
/** relay change notifications to connected listeners. This is not pretty, since the arguments change their meanings depending on the type of notification, but for now this is ok */
	void sendListenerNotification (RObjectListener::NotificationType type, RObject* o, int index, int new_index, RObject::ChangeSet* changes);

friend class RObjectListener;
	void addObjectListener (RObject* object, RObjectListener* listener);
	void removeObjectListener (RObject* object, RObjectListener* listener);
	QMultiHash<RObject*, RObjectListener*> listeners;

friend class RContainerObject;
friend class REnvironmentObject;
friend class RObject;
friend class RObjectList;
/** essentially like the above function(s). All objects listening for child additions on the parent will be notified */
	void beginAddObject (RObject *object, RObject* parent, int position);
	void endAddObject (RObject *object, RObject* parent, int position);
/** essentially like the above function(s). All objects listening for child position changed on the parent will be notified */
	void moveObject (RContainerObject *parent, RObject* child, int old_index, int new_index);
};

#endif
