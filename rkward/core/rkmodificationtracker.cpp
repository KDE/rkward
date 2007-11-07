/***************************************************************************
                          rkmodificationtracker  -  description
                             -------------------
    begin                : Tue Aug 31 2004
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
#include "rkmodificationtracker.h"

#include <kmessagebox.h>
#include <klocale.h>

#include "../rkglobals.h"
#include "../dataeditor/rkeditor.h"
#include "../dataeditor/rkvareditmodel.h"
#include "rcontainerobject.h"
#include "robjectlist.h"
#include "../windows/rkworkplace.h"
#include "../misc/rkstandardicons.h"

#include "../debug.h"

RKModificationTracker::RKModificationTracker (QObject *parent) : RKObjectListModel (parent) {
	RK_TRACE (OBJECTS);

	updates_locked = 0;
}

RKModificationTracker::~RKModificationTracker () {
	RK_TRACE (OBJECTS);

	RK_ASSERT (updates_locked == 0);
}

void RKModificationTracker::lockUpdates (bool lock) {
	RK_TRACE (OBJECTS);

	if (lock) ++updates_locked;
	else {
		--updates_locked;
		RK_ASSERT (updates_locked >= 0);
	}
}

bool RKModificationTracker::removeObject (RObject *object, RKEditor *editor, bool removed_in_workspace) {
	RK_TRACE (OBJECTS);
// TODO: allow more than one editor per object
// WARNING: This does not work, if a sub-object is being edited!
	RKEditor *ed = objectEditor (object);
	RK_ASSERT (object);
	RK_ASSERT (!((editor) && (!ed)));
	RK_ASSERT (!(removed_in_workspace && editor));

	if (removed_in_workspace) {
		if (ed) {
			if (KMessageBox::questionYesNo (0, i18n ("The object '%1' was removed from workspace or changed to a different type of object, but is currently opened for editing. Do you want to restore it?", object->getFullName ()), i18n ("Restore object?")) == KMessageBox::Yes) {
				if (removed_in_workspace) ed->restoreObject (object);
				return false;
			}
		}
	} else {
		if (editor || ed) {
			if (KMessageBox::questionYesNo (0, i18n ("Do you really want to remove the object '%1'? The object is currently opened for editing, it will be removed in the editor, too. There's no way to get it back.", object->getFullName ()), i18n ("Remove object?")) != KMessageBox::Yes) {
				return false;
			}
		} else {
			// TODO: check for other editors editing this object
			if (KMessageBox::questionYesNo (0, i18n ("Do you really want to remove the object '%1'? There's no way to get it back.", object->getFullName ()), i18n ("Remove object?")) != KMessageBox::Yes) {
				return false;
			}
		}
	}

	RK_ASSERT (object);
	RK_ASSERT (object->getContainer ());

	if (!updates_locked) {
		QModelIndex object_index = indexFor (object->getContainer ());
		int object_row = object->getContainer ()->getIndexOf (object);
		RK_ASSERT (object_row >= 0);
		beginRemoveRows (object_index, object_row, object_row);
	}

	if (!updates_locked) sendListenerNotification (RObjectListener::ObjectRemoved, object, 0, 0, 0);

	object->remove (removed_in_workspace);

	if (!updates_locked) endRemoveRows ();

	return true;
}

void RKModificationTracker::moveObject (RContainerObject *parent, RObject* child, int old_index, int new_index) {
	RK_TRACE (OBJECTS);

	QModelIndex parent_index;

	if (!updates_locked) {
		parent_index = indexFor (parent->getContainer ());
		beginRemoveRows (parent_index, old_index, old_index);
	}
	RK_ASSERT (parent->findChildByIndex (old_index) == child);
	parent->removeChildNoDelete (child);
	if (!updates_locked) {
		endRemoveRows ();

		beginInsertRows (parent_index, new_index, new_index);
	}
	parent->insertChild (child, new_index);
	RK_ASSERT (parent->findChildByIndex (new_index) == child);
	if (!updates_locked) {
		endInsertRows ();
		sendListenerNotification (RObjectListener::ChildMoved, parent, old_index, new_index, 0);
	}
}

void RKModificationTracker::renameObject (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);

	object->rename (new_name);

	if (!updates_locked) {
		sendListenerNotification (RObjectListener::MetaChanged, object, 0, 0, 0);

		QModelIndex object_index = indexFor (object);
		emit (dataChanged (object_index, object_index));
	}
}

void RKModificationTracker::addObject (RObject *object, RContainerObject* parent, int position) {
	RK_TRACE (OBJECTS);

	if (!updates_locked) {
		QModelIndex parent_index = indexFor (parent);
		beginInsertRows (parent_index, position, position);
	}

	parent->insertChild (object, position);

	if (!updates_locked) {
		sendListenerNotification (RObjectListener::ChildAdded, parent, position, 0, 0);
		endInsertRows ();
	}
}

void RKModificationTracker::objectMetaChanged (RObject *object) {
	RK_TRACE (OBJECTS);

	if (!updates_locked) {
		sendListenerNotification (RObjectListener::MetaChanged, object, 0, 0, 0);

		QModelIndex object_index = indexFor (object);
		emit (dataChanged (object_index, object_index));
	}
}

void RKModificationTracker::objectDataChanged (RObject *object, RObject::ChangeSet *changes) {
	RK_TRACE (OBJECTS);

	if (!updates_locked) {
		sendListenerNotification (RObjectListener::DataChanged, object, 0, 0, changes);
		delete changes;

		QModelIndex object_index = indexFor (object);
		emit (dataChanged (object_index, object_index));
	}
}

void RKModificationTracker::addObjectListener (RObject* object, RObjectListener* listener) {
	RK_TRACE (OBJECTS);

	listeners.insert (object, listener);
#warning: probably we should create and check for an appropriate NotificationType, instead
	if (listener->listenerType () == RObjectListener::DataModel) object->beginEdit ();
}

void RKModificationTracker::removeObjectListener (RObject* object, RObjectListener* listener) {
	RK_TRACE (OBJECTS);

	listeners.remove (object, listener);
#warning: probably we should create and check for an appropriate NotificationType, instead
	if (listener->listenerType () == RObjectListener::DataModel) object->endEdit ();
}

void RKModificationTracker::sendListenerNotification (RObjectListener::NotificationType type, RObject* o, int index, int new_index, RObject::ChangeSet* changes) {
	RK_TRACE (OBJECTS);

	QList<RObjectListener*> obj_listeners = listeners.values (o);
	for (int i = obj_listeners.size () - 1; i >= 0; --i) {
		RObjectListener* listener = obj_listeners[i];
		if (!listener->wantsNotificationType (type)) continue;

		if (type == RObjectListener::ObjectRemoved) {
			listener->objectRemoved (o);
		} else if (type == RObjectListener::ChildAdded) {
			listener->childAdded (index, o);
		} else if (type == RObjectListener::ChildMoved) {
			listener->childMoved (index, new_index, o);
		} else if (type == RObjectListener::MetaChanged) {
			listener->objectMetaChanged (o);
		} else if (type == RObjectListener::DataChanged) {
			listener->objectDataChanged (o, changes);
		} else {
			RK_ASSERT (false);
		}
	}
}

RKEditor* RKModificationTracker::objectEditor (RObject* object) {
	RK_TRACE (OBJECTS);

	QList<RObjectListener*> obj_listeners = listeners.values (object);
	for (int i = obj_listeners.size () - 1; i >= 0; --i) {
		RObjectListener* listener = obj_listeners[i];
		if (!(listener->listenerType () == RObjectListener::DataModel)) continue;

		RKEditor* ed = static_cast<RKVarEditModel*> (listener)->getEditor ();
		if (ed) return ed;
	}

	return 0;
}

///////////////// RKObjectListModel ///////////////////////////

RKObjectListModel::RKObjectListModel (QObject *parent) : QAbstractItemModel (parent) {
	RK_TRACE (OBJECTS);
}

RKObjectListModel::~RKObjectListModel () {
	RK_TRACE (OBJECTS);
}

QModelIndex RKObjectListModel::index (int row, int column, const QModelIndex& parent) const {
	RK_TRACE (OBJECTS);
	if (!parent.isValid ()) {
		RK_ASSERT (row == 0);
		// must cast to RObject, here. Else casting to void* and back will confuse the hell out of GCC 4.2
		return (createIndex (row, column, static_cast<RObject *> (RObjectList::getObjectList ())));
	}
	RObject* parent_object = static_cast<RObject*> (parent.internalPointer ());

	RK_ASSERT (parent_object->isContainer ());
	RContainerObject* container = static_cast<RContainerObject*> (parent_object);
	RK_ASSERT (row < container->numChildren ());

	return (createIndex (row, column, container->findChildByIndex (row)));
}

QModelIndex RKObjectListModel::parent (const QModelIndex& index) const {
	RK_TRACE (OBJECTS);

	if (!index.isValid ()) return QModelIndex ();
	RObject* child = static_cast<RObject*> (index.internalPointer ());
	RK_ASSERT (child);
	return (indexFor (child->getContainer ()));
}

int RKObjectListModel::rowCount (const QModelIndex& parent) const {
	RK_TRACE (OBJECTS);

	RObject* parent_object = 0;
	if (parent.isValid ()) parent_object = static_cast<RObject*> (parent.internalPointer ());
	else return 1;		// the root item

	if (!(parent_object && parent_object->isContainer ())) return 0;

	return (static_cast<RContainerObject*> (parent_object)->numChildren ());
}

int RKObjectListModel::columnCount (const QModelIndex&) const {
	//RK_TRACE (OBJECTS); // no need to trace this

	return ColumnCount;
}

QVariant RKObjectListModel::data (const QModelIndex& index, int role) const {
	RK_TRACE (OBJECTS);

	int col = index.column ();
	RObject *object = static_cast<RObject*> (index.internalPointer ());

	if ((!object) || (col >= ColumnCount)) {
		RK_ASSERT (false);
		return QVariant ();
	}

	if (role == Qt::DisplayRole) {
		if (col == NameColumn) return object->getShortName ();
		if (col == LabelColumn) return object->getLabel ();
		if (col == TypeColumn) {
			if (object->isVariable ()) return RObject::typeToText (object->getDataType ());
			return QVariant ();
		}
		if (col == ClassColumn) return object->makeClassString ("; ");
		RK_ASSERT (false);
	} else if (role == Qt::DecorationRole) {
		if (col == NameColumn) return RKStandardIcons::iconForObject (object);
	} else if (role == Qt::ToolTipRole) {
		return object->getObjectDescription ();
	}

	return QVariant ();
}

QVariant RKObjectListModel::headerData (int section, Qt::Orientation orientation, int role) const {
	RK_TRACE (OBJECTS);

	if (orientation != Qt::Horizontal) return QVariant ();
	if (role != Qt::DisplayRole) return QVariant ();

	if (section == NameColumn) return i18n ("Name");
	if (section == LabelColumn) return i18n ("Label");
	if (section == TypeColumn) return i18n ("Type");
	if (section == ClassColumn) return i18n ("Class");

	RK_ASSERT (false);
	return QVariant ();
}

QModelIndex RKObjectListModel::indexFor (RObject *object) const {
	RK_TRACE (OBJECTS);

	if (!object) return QModelIndex ();

	RContainerObject *parent = object->getContainer ();
	// must cast to RObject, here. Else casting to void* and back will confuse the hell out of GCC 4.2
	if (!parent) return createIndex (0, 0, static_cast<RObject*> (RObjectList::getObjectList ()));

	int row = parent->getIndexOf (object);
	if (row < 0) {
		RK_ASSERT (false);
		return QModelIndex ();
	}

	return (createIndex (row, 0, object));
}


///////////////////// RObjectListener ////////////////////////

RObjectListener::RObjectListener (ListenerType type) {
	RK_TRACE (OBJECTS);

	RObjectListener::type = type;
	notifications = 0;
	num_watched_objects = 0;
}

RObjectListener::~RObjectListener () {
	RK_TRACE (OBJECTS);

	RK_ASSERT (num_watched_objects == 0);
}

void RObjectListener::objectRemoved (RObject*) {
	RK_ASSERT (false);	// listeners that receive this notification should have reimplemented this function
}

void RObjectListener::childAdded (int, RObject*) {
	RK_ASSERT (false);	// listeners that receive this notification should have reimplemented this function
}

void RObjectListener::childMoved (int, int, RObject*) {
	RK_ASSERT (false);	// listeners that receive this notification should have reimplemented this function
}

void RObjectListener::objectMetaChanged (RObject*) {
	RK_ASSERT (false);	// listeners that receive this notification should have reimplemented this function
}

void RObjectListener::objectDataChanged (RObject*, const RObject::ChangeSet *) {
	RK_ASSERT (false);	// listeners that receive this notification should have reimplemented this function
}

void RObjectListener::listenForObject (RObject* object) {
	RK_TRACE (OBJECTS);

	RKGlobals::tracker ()->addObjectListener (object, this);
	++num_watched_objects;
}

void RObjectListener::stopListenForObject (RObject* object) {
	RK_TRACE (OBJECTS);

	RKGlobals::tracker ()->removeObjectListener (object, this);
	--num_watched_objects;
}

#include "rkmodificationtracker.moc"
