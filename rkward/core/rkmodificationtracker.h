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
#ifndef RKMODIFICATIONTRACKER_H
#define RKMODIFICATIONTRACKER_H

#include <qobject.h>
#include <qstring.h>
#include <QAbstractItemModel>

#include "robject.h"

class RKEditor;
class RObject;

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

	/** convenience function to create an index for a particular object */
	QModelIndex indexFor (RObject *object) const;
};


/**
This class takes care of propagating object-modifications to all editors/variable-browsers etc. that need to know about them. For instance, if an object was removed in the R-workspace, the RObjectList will notify the RKModificationTracker. The modification tracker will then find out, whether there are editor(s) currently editing the removed object. If so, it will prompt the user what to do. Or, if an object is renamed in an editor, the RKModificationTracker will find out, whether the object is opened in further editors (not possible, yet) and tell those to update accordingly. It will further emit signals so the RObjectBrowser and RKVarselector(s) can update their object-lists.

@author Thomas Friedrichsmeier
*/
class RKModificationTracker : public RKObjectListModel {
Q_OBJECT
public:
	RKModificationTracker (QObject *parent);

	~RKModificationTracker ();
	
/** the given object should be removed (either it was removed in the R-workspace, or the user requests removal of the object in an editor or the RObjectList). First, if the object is being edited somewhere, the user will get a chance to object to the removal. If the user does not object, the RKModificationTracker will remove the object and notify all editors/objectlists that the object really was removed. When calling from the RObjectList, you will likely set removed_in_workspace to true, to signal that the object-data is already gone in the workspace. */
	bool removeObject (RObject *object, RKEditor *editor=0, bool removed_in_workspace=false);
/** essentially like the above function, but requests a renaming of the object. Will also take care of finding out, whether the name is valid and promting for a different name otherwise. */
	void renameObject (RObject *object, const QString &new_name);
/** essentially like the above function(s). All objects editing a parent of the new objects are notified of the addition. */
	void addObject (RObject *object, RContainerObject* parent, int position, RKEditor *editor=0);
/** the object's meta data was modified. Tells all editors and lists containing the object to update accordingly. */
	void objectMetaChanged (RObject *object);
/** the object's data was modified. Tells all editors and lists containing the object to update accordingly. The ChangeSet given tells which parts of the data have to be updated. The ChangeSet will get deleted by the RKModificationTracker, when done. */
	void objectDataChanged (RObject *object, RObject::ChangeSet *changes);
/** recursive! */
	void lockUpdates (bool lock);
signals:
/** classes which are not RKEditor(s) but need to know, when an object was removed, should connect to this signal */
	void objectRemoved (RObject *object);
/** classes which are not RKEditor(s) but need to know, when an object was renamed or otherwise changed its properties, should connect to this signal */
	void objectPropertiesChanged (RObject *object);
/** classes which are not RKEditor(s) but need to know, when an object was added, should connect to this signal */
	void objectAdded (RObject *object);
private:
	int updates_locked;

friend class RContainerObject;
/** uncondiontally remove the given object. Do *not* call this except from RContainerObject::moveChild() or internally from removeObject(). Call removeObject(), instead. */
	void internalRemoveObject (RObject *object, bool removed_in_workspace, bool delete_obj);
};

#endif
