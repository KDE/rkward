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
	RKEditor *ed = object->objectOpened ();
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

	internalRemoveObject (object, removed_in_workspace, true);

	return true;
}

void RKModificationTracker::internalRemoveObject (RObject *object, bool removed_in_workspace, bool delete_obj) {
	RK_TRACE (OBJECTS);

	RK_ASSERT (object);
	RK_ASSERT (object->getContainer ());

	if (!updates_locked) {
		QModelIndex object_index = indexFor (object->getContainer ());
		int object_row = object->getContainer ()->getIndexOf (object);
		RK_ASSERT (object_row >= 0);
		beginRemoveRows (object_index, object_row, object_row);
	}

// TODO: allow more than one editor per object
// WARNING: This does not work, if a sub-object is being edited!
	RKEditor *ed = object->objectOpened ();

	if (ed) ed->removeObject (object);		// READ: delete ed
/* What's this? A child of a removed complex object may be edited somewhere, but not the whole object. In this case, the editor has no chance of restoring the object, but it still needs to be closed. We search all editors for the removed object */
	if (object->isContainer ()) {
		RKWorkplace::RKWorkplaceObjectList list = RKWorkplace::mainWorkplace ()->getObjectList (RKMDIWindow::DataEditorWindow);
		for (RKWorkplace::RKWorkplaceObjectList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
			RKEditor *subed = static_cast<RKEditor *> (*it);
			RObject *subedobj = subed->getObject ();
			if (static_cast<RContainerObject *> (object)->isParentOf (subedobj, true)) {
				subed->removeObject (subedobj);
			}
		}
	}

	if (!updates_locked) emit (objectRemoved (object));

	if (delete_obj) object->remove (removed_in_workspace);
	else object->getContainer ()->removeChildNoDelete (object);

	if (!updates_locked) endRemoveRows ();
}

void RKModificationTracker::renameObject (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);
// TODO: allow more than one editor per object
// TODO: find out, whether new object-name is valid
	RKEditor *ed = object->objectOpened ();

	object->rename (new_name);

// since we may end up with a different name that originally requested, we propagate the change also to the original editor
	if (ed) ed->renameObject (object);

	if (!updates_locked) {
		emit (objectPropertiesChanged (object));

		QModelIndex object_index = indexFor (object);
		emit (dataChanged (object_index, object_index));
	}
}

void RKModificationTracker::addObject (RObject *object, RContainerObject* parent, int position, RKEditor *editor) {
	RK_TRACE (OBJECTS);

	if (!updates_locked) {
		QModelIndex parent_index = indexFor (parent);
		beginInsertRows (parent_index, position, position);
	}

	parent->insertChild (object, position);

// TODO: allow more than one editor per object
	RKEditor *ed = 0;
	if (object->getContainer ()) ed = object->getContainer ()->objectOpened ();
	RK_ASSERT (!((editor) && (!ed)));
	
	if (ed) {
		if (ed != editor) {
			ed->addObject (object);
		}
	}

	if (!updates_locked) {
		emit (objectAdded (object));
		endInsertRows ();
	}
}

void RKModificationTracker::objectMetaChanged (RObject *object) {
	RK_TRACE (OBJECTS);
// TODO: allow more than one editor per object
	RKEditor *ed = object->objectOpened ();
	
	if (ed) {
		ed->updateObjectMeta (object);
	}

	if (!updates_locked) {
		emit (objectPropertiesChanged (object));

		QModelIndex object_index = indexFor (object);
		emit (dataChanged (object_index, object_index));
	}
}

void RKModificationTracker::objectDataChanged (RObject *object, RObject::ChangeSet *changes) {
	RK_TRACE (OBJECTS);
// TODO: allow more than one editor per object
	RKEditor *ed = object->objectOpened ();

	if (ed) {
		ed->updateObjectData (object, changes);
	}

	delete changes;

	if (!updates_locked) {
		QModelIndex object_index = indexFor (object);
		emit (dataChanged (object_index, object_index));
	}
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

#include "rkmodificationtracker.moc"
