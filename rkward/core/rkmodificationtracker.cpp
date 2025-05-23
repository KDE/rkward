/*
rkmodificationtracker - This file is part of RKWard (https://rkward.kde.org). Created: Tue Aug 31 2004
SPDX-FileCopyrightText: 2004-2017 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkmodificationtracker.h"

#include <KLocalizedString>
#include <kmessagebox.h>

#include "../dataeditor/rkeditor.h"
#include "../dataeditor/rkvareditmodel.h"
#include "../misc/rkstandardicons.h"
#include "../windows/rkworkplace.h"
#include "rcontainerobject.h"
#include "renvironmentobject.h"
#include "robjectlist.h"

#include "../debug.h"

RKModificationTracker *RKModificationTracker::_instance = nullptr;

RKModificationTracker::RKModificationTracker(QObject *parent) : RKObjectListModel(parent) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(_instance == nullptr); // singleton for now
	_instance = this;
	updates_locked = 0;
}

RKModificationTracker::~RKModificationTracker() {
	RK_TRACE(OBJECTS);

	RK_ASSERT(updates_locked == 0);
	RK_ASSERT(listeners.isEmpty());
}

void RKModificationTracker::lockUpdates(bool lock) {
	RK_TRACE(OBJECTS);

	if (lock) ++updates_locked;
	else {
		--updates_locked;
		RK_ASSERT(updates_locked >= 0);
	}
}

bool RKModificationTracker::removeObject(RObject *object, RKEditor *editor, bool removed_in_workspace) {
	RK_TRACE(OBJECTS);
	// WARNING: This does not work, if a sub-object is being edited!
	RKEditor *ed = objectEditors(object).value(0);
	RK_ASSERT(object);
	RK_ASSERT(!((editor) && (!ed)));
	RK_ASSERT(!(removed_in_workspace && editor));

	if (!object->isPseudoObject()) {
		if (removed_in_workspace) {
			if (ed && (ed->getObject() == object) && object->canWrite()) { // NOTE: do not allow restoring of columns in a data.frame this way. See https://mail.kde.org/pipermail/rkward-devel/2012-March/003225.html and replies.
				if (KMessageBox::questionTwoActions(nullptr, i18n("The object '%1' was removed from workspace or changed to a different type of object, but is currently opened for editing. Do you want to restore it?", object->getFullName()), i18n("Restore object?"), KStandardGuiItem::ok(), KStandardGuiItem::cancel()) == KMessageBox::PrimaryAction) {
					ed->restoreObject(object);
					/* TODO: It would make a lot of sense to allow restoring to a different name, and possibly different location. This may need some thinking. Probably something like:
					 * 	object->parentObject ()->removeChildNoDelete (parent);
					 * 	object->setParentObject (RObjectList::getGlobalEnv ());
					 * 	// make sure new_name is unique in new parent!
					 * 	RObjectList::getGlobalEnv ()->insertChild (object, -1);
					 * 	object->setName (new_name);
					 * along with proper begin/endAdd/RemoveRows().
					 * Oh, and listener notifications. That might be tricky.
					 * */
					return false;
				}
			}
		} else {
			if (editor || ed) {
				if (KMessageBox::questionTwoActions(nullptr, i18n("Do you really want to remove the object '%1'? The object is currently opened for editing, it will be removed in the editor, too. There's no way to get it back.", object->getFullName()), i18n("Remove object?"), KStandardGuiItem::remove(), KStandardGuiItem::cancel()) != KMessageBox::PrimaryAction) {
					return false;
				}
			} else {
				// TODO: check for other editors editing this object
				if (KMessageBox::questionTwoActions(nullptr, i18n("Do you really want to remove the object '%1'? There's no way to get it back.", object->getFullName()), i18n("Remove object?"), KStandardGuiItem::remove(), KStandardGuiItem::cancel()) != KMessageBox::PrimaryAction) {
					return false;
				}
			}
		}
	}

	RK_ASSERT(object);
	if (!object->parentObject()) {
		RK_DEBUG(OBJECTS, DL_ERROR, "Trying to remove root level object. Backend crashed?");
		return false;
	}
	bool view_update = !updates_locked && !object->isType(RObject::NonVisibleObject);

	if (view_update) {
		QModelIndex object_index = indexFor(object->parentObject());
		int object_row = object->parentObject()->getObjectModelIndexOf(object);
		RK_ASSERT(object_row >= 0);
		beginRemoveRows(object_index, object_row, object_row);
	}

	if (!(updates_locked || object->isPseudoObject())) sendListenerNotification(RObjectListener::ObjectRemoved, object, 0, 0, nullptr);

	object->remove(removed_in_workspace);

	if (view_update) endRemoveRows();

	return true;
}

void RKModificationTracker::moveObject(RContainerObject *parent, RObject *child, int old_index, int new_index) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(!child->isPseudoObject());

	QModelIndex parent_index;

	if (!updates_locked) {
		parent_index = indexFor(parent);
		beginRemoveRows(parent_index, old_index, old_index);
	}
	RK_ASSERT(parent->findChildByIndex(old_index) == child);
	parent->removeChildNoDelete(child);
	if (!updates_locked) {
		endRemoveRows();

		beginInsertRows(parent_index, new_index, new_index);
	}
	parent->insertChild(child, new_index);
	RK_ASSERT(parent->findChildByIndex(new_index) == child);
	if (!updates_locked) {
		endInsertRows();
		sendListenerNotification(RObjectListener::ChildMoved, parent, old_index, new_index, nullptr);
	}
}

void RKModificationTracker::renameObject(RObject *object, const QString &new_name) {
	RK_TRACE(OBJECTS);

	object->rename(new_name);

	if (!updates_locked) {
		sendListenerNotification(RObjectListener::MetaChanged, object, 0, 0, nullptr);

		QModelIndex object_index = indexFor(object);
		Q_EMIT dataChanged(object_index, object_index);
	}
}

void RKModificationTracker::beginAddObject(RObject *object, RObject *parent, int position) {
	RK_TRACE(OBJECTS);
	Q_UNUSED(object); // Kept for consistency of function signature

	if (!updates_locked) {
		QModelIndex parent_index = indexFor(parent);
		beginInsertRows(parent_index, position, position);
	}
}

void RKModificationTracker::endAddObject(RObject *object, RObject *parent, int position) {
	RK_TRACE(OBJECTS);

	if (!updates_locked) {
		if (!object->isPseudoObject()) sendListenerNotification(RObjectListener::ChildAdded, parent, position, 0, nullptr);
		endInsertRows();
	}
}

void RKModificationTracker::objectMetaChanged(RObject *object) {
	RK_TRACE(OBJECTS);

	if (!updates_locked) {
		sendListenerNotification(RObjectListener::MetaChanged, object, 0, 0, nullptr);

		QModelIndex object_index = indexFor(object);
		Q_EMIT dataChanged(object_index, object_index.sibling(object_index.row(), ColumnCount - 1));
	}
}

void RKModificationTracker::objectDataChanged(RObject *object, RObject::ChangeSet *changes) {
	RK_TRACE(OBJECTS);

	if (!updates_locked) {
		sendListenerNotification(RObjectListener::DataChanged, object, 0, 0, changes);
		delete changes;

		QModelIndex object_index = indexFor(object);
		Q_EMIT dataChanged(object_index, object_index.sibling(object_index.row(), ColumnCount - 1)); // might have changed dimensions, for instance
	}
}

void RKModificationTracker::addObjectListener(RObject *object, RObjectListener *listener) {
	RK_TRACE(OBJECTS);

	listeners.insert(object, listener);
	if (listener->listenerType() == RObjectListener::DataModel) object->beginEdit();
}

void RKModificationTracker::removeObjectListener(RObject *object, RObjectListener *listener) {
	RK_TRACE(OBJECTS);

	listeners.remove(object, listener);
	if (listener->listenerType() == RObjectListener::DataModel) object->endEdit();
}

void RKModificationTracker::sendListenerNotification(RObjectListener::NotificationType type, RObject *o, int index, int new_index, RObject::ChangeSet *changes) {
	RK_TRACE(OBJECTS);

	QList<RObjectListener *> obj_listeners = listeners.values(o);
	for (int i = obj_listeners.size() - 1; i >= 0; --i) {
		RObjectListener *listener = obj_listeners[i];
		if (!listener->wantsNotificationType(type)) continue;

		if (type == RObjectListener::ObjectRemoved) {
			listener->objectRemoved(o);
		} else if (type == RObjectListener::ChildAdded) {
			listener->childAdded(index, o);
		} else if (type == RObjectListener::ChildMoved) {
			listener->childMoved(index, new_index, o);
		} else if (type == RObjectListener::MetaChanged) {
			listener->objectMetaChanged(o);
		} else if (type == RObjectListener::DataChanged) {
			listener->objectDataChanged(o, changes);
		} else {
			RK_ASSERT(false);
		}
	}

	// when a container is removed, we need to send child notifications recursively, so listeners listening
	// for child objects will know the object is gone.
	if (type == RObjectListener::ObjectRemoved) {
		if (o->isContainer()) {
			RContainerObject *c = static_cast<RContainerObject *>(o);
			for (int i = c->numChildren() - 1; i >= 0; --i) {
				sendListenerNotification(RObjectListener::ObjectRemoved, c->findChildByIndex(i), 0, 0, nullptr);
			}
		}
	}
}

QList<RKEditor *> RKModificationTracker::objectEditors(const RObject *object) const {
	RK_TRACE(OBJECTS);

	QList<RKEditor *> ret;
	QList<RObjectListener *> obj_listeners = listeners.values(const_cast<RObject *>(object));
	for (int i = obj_listeners.size() - 1; i >= 0; --i) {
		RObjectListener *listener = obj_listeners[i];
		if (!(listener->listenerType() == RObjectListener::DataModel)) continue;

		RKEditor *ed = static_cast<RKVarEditModel *>(listener)->getEditor();
		if (ed) ret.append(ed);
	}

	return ret;
}

///////////////// RKObjectListModel ///////////////////////////

RKObjectListModel::RKObjectListModel(QObject *parent) : QAbstractItemModel(parent) {
	RK_TRACE(OBJECTS);
}

RKObjectListModel::~RKObjectListModel() {
	RK_TRACE(OBJECTS);
}

QModelIndex RKObjectListModel::index(int row, int column, const QModelIndex &parent) const {
	RK_TRACE(OBJECTS);
	if (!parent.isValid()) {
		RK_ASSERT(row < 2);
		// must cast to RObject, here. Else casting to void* and back will confuse the hell out of GCC 4.2
		if (row == 0) return (createIndex(0, column, static_cast<RObject *>(RObjectList::getGlobalEnv())));
		if (row == 1) return (createIndex(1, column, static_cast<RObject *>(RObjectList::getObjectList())));
		RK_ASSERT(false);
		return QModelIndex();
	}
	RObject *parent_object = static_cast<RObject *>(parent.internalPointer());

	RK_ASSERT(row < parent_object->numChildrenForObjectModel());

	return (createIndex(row, column, parent_object->findChildByObjectModelIndex(row)));
}

QModelIndex RKObjectListModel::parent(const QModelIndex &index) const {
	RK_TRACE(OBJECTS);

	if (!index.isValid()) return QModelIndex();
	RObject *child = static_cast<RObject *>(index.internalPointer());
	RK_ASSERT(child);
	if (child == RObjectList::getGlobalEnv()) return QModelIndex();
	return (indexFor(child->parentObject()));
}

int RKObjectListModel::rowCount(const QModelIndex &parent) const {
	RK_TRACE(OBJECTS);

	RObject *parent_object = nullptr;
	if (parent.isValid()) parent_object = static_cast<RObject *>(parent.internalPointer());
	else return 2; // the root item

	if (!parent_object) return 0;
	return (parent_object->numChildrenForObjectModel());
}

int RKObjectListModel::columnCount(const QModelIndex &) const {
	// RK_TRACE (OBJECTS); // no need to trace this

	return ColumnCount;
}

QVariant RKObjectListModel::data(const QModelIndex &index, int role) const {
	RK_TRACE(OBJECTS);

	int col = index.column();
	RObject *object = static_cast<RObject *>(index.internalPointer());

	if (!object) {
		RK_ASSERT(object);
		return QVariant();
	}

	if (role == Qt::DisplayRole) {
		if (col == NameColumn) return object->getShortName();
		if (col == LabelColumn) return object->getLabel();
		if (col == TypeColumn) {
			if (object->isVariable()) return RObject::typeToText(object->getDataType());
			return QVariant();
		}
		if ((col == ClassColumn) && (!object->isPseudoObject())) return object->makeClassString(QStringLiteral("; "));
	} else if (role == Qt::FontRole) {
		if (col == NameColumn && object->isPseudoObject()) {
			QFont font;
			font.setItalic(true);
			return (font);
		}
	} else if (role == Qt::DecorationRole) {
		if (col == NameColumn) return RKStandardIcons::iconForObject(object);
	} else if (role == Qt::ToolTipRole) {
		QString ret = u"<i>"_s + object->getShortName().replace(u'<', u"&lt;"_s) + u"</i><br>"_s + object->getObjectDescription();
		return ret;
	}

	RK_ASSERT(col < columnCount());
	return QVariant();
}

QVariant RKObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const {
	RK_TRACE(OBJECTS);

	if (orientation != Qt::Horizontal) return QVariant();
	if (role != Qt::DisplayRole) return QVariant();

	if (section == NameColumn) return i18n("Name");
	if (section == LabelColumn) return i18n("Label");
	if (section == TypeColumn) return i18n("Type");
	if (section == ClassColumn) return i18n("Class");

	RK_ASSERT(false);
	return QVariant();
}

bool RKObjectListModel::hasChildren(const QModelIndex &parent) const {
	RK_TRACE(OBJECTS);

	RObject *parent_object = nullptr;
	if (parent.isValid()) parent_object = static_cast<RObject *>(parent.internalPointer());
	else return true; // the root item

	if (!parent_object) return false;
	return (parent_object->isType(RObject::Incomplete) || parent_object->numChildrenForObjectModel());
}

bool RKObjectListModel::canFetchMore(const QModelIndex &parent) const {
	RK_TRACE(OBJECTS);

	RObject *object = static_cast<RObject *>(parent.internalPointer());
	return (object && object->isType(RObject::Incomplete));
}

void RKObjectListModel::fetchMore(const QModelIndex &parent) {
	RK_TRACE(OBJECTS);

	RObject *object = static_cast<RObject *>(parent.internalPointer());
	RK_ASSERT(object && object->isType(RObject::Incomplete));
	object->fetchMoreIfNeeded();
}

QModelIndex RKObjectListModel::indexFor(RObject *object) const {
	RK_TRACE(OBJECTS);

	if (!object) return QModelIndex();
	if (object->isType(RObject::NonVisibleObject)) return QModelIndex();

	RObject *parent = object->parentObject();
	// must cast to RObject, here. Else casting to void* and back will confuse the hell out of GCC 4.2
	if (!parent) {
		if (object == RObjectList::getObjectList()) {
			return createIndex(1, 0, static_cast<RObject *>(RObjectList::getObjectList()));
		} else {
			RK_ASSERT(object == RObjectList::getGlobalEnv());
			return createIndex(0, 0, static_cast<RObject *>(RObjectList::getGlobalEnv()));
		}
	}

	int row = parent->getObjectModelIndexOf(object);
	if (row < 0) {
		RK_ASSERT(false);
		return QModelIndex();
	}

	return (createIndex(row, 0, object));
}

///////////////////// RObjectListener ////////////////////////

RObjectListener::RObjectListener(ListenerType type) {
	RK_TRACE(OBJECTS);

	RObjectListener::type = type;
	notifications = 0;
	num_watched_objects = 0;
}

RObjectListener::~RObjectListener() {
	RK_TRACE(OBJECTS);

	RK_ASSERT(num_watched_objects == 0);
}

void RObjectListener::objectRemoved(RObject *) {
	RK_ASSERT(false); // listeners that receive this notification should have reimplemented this function
}

void RObjectListener::childAdded(int, RObject *) {
	RK_ASSERT(false); // listeners that receive this notification should have reimplemented this function
}

void RObjectListener::childMoved(int, int, RObject *) {
	RK_ASSERT(false); // listeners that receive this notification should have reimplemented this function
}

void RObjectListener::objectMetaChanged(RObject *) {
	RK_ASSERT(false); // listeners that receive this notification should have reimplemented this function
}

void RObjectListener::objectDataChanged(RObject *, const RObject::ChangeSet *) {
	RK_ASSERT(false); // listeners that receive this notification should have reimplemented this function
}

void RObjectListener::listenForObject(RObject *object) {
	RK_TRACE(OBJECTS);

	RKModificationTracker::instance()->addObjectListener(object, this);
	++num_watched_objects;
}

void RObjectListener::stopListenForObject(RObject *object) {
	RK_TRACE(OBJECTS);

	RKModificationTracker::instance()->removeObjectListener(object, this);
	--num_watched_objects;
}
