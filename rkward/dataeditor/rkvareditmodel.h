/***************************************************************************
                          rkvareditmodel  -  description
                             -------------------
    begin                : Mon Nov 05 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#ifndef RKVAREDITMODEL
#define RKVAREDITMODEL

#include <QAbstractTableModel>
#include <QList>

#include "../core/rkvariable.h"

class RKVarEditMetaModel;

/** This class represents a collection of RKVariables of uniform length (typically a data.frame) suitable for editing in a model/view editor such as QTableView. Probably it will only ever support editing a single RKVariable, though, as it is not possible to ensure uniform length outside of a data.frame. For a data.frame use RKVarEditDataFrameModel . */
class RKVarEditModel : public QAbstractTableModel {
	Q_OBJECT
public:
	RKVarEditModel (QObject *parent);
	~RKVarEditModel ();

	/** Add an object to the model at the given index. Calls this only once, unless from an RKVarEditDataFrameModel. You should add at least one object, **before** you add this model to any view.
	@param index position to insert at, or -1 to append the item
	@param object the objects to insert */
	void addObject (int index, RKVariable* object);
	/** Return a pointer to the meta model. The meta model is created the first time this function is called. */
	RKVarEditMetaModel* getMetaModel ();

	// QAbstractTableModel implementations
	bool insertRows (int row, int count, const QModelIndex& parent = QModelIndex());
	bool removeRows (int row, int count, const QModelIndex& parent = QModelIndex());
	int rowCount (const QModelIndex& parent = QModelIndex()) const;
	int columnCount (const QModelIndex& parent = QModelIndex()) const;
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags (const QModelIndex& index) const;
	bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
signals:
	// convenience signal to tell the editor to block editing entirely, without having to set all flags to non-editable.
	void blockEdit (bool block);
public slots:
	/** Receives notifications of object removals. Takes care of removing the object from the list. */
	virtual void objectRemoved (RObject* object);
protected:
friend class RKVarEditMetaModel;
	QList<RKVariable*> objects;

	/** very simple convenience function to return the number of true cols + trailing cols */
	int apparentCols () const { return objects.size () + trailing_cols; };
	/** very simple convenience function to return the number of true rows + trailing rows */
	int apparentRows () const { return (trailing_rows + objects.isEmpty () ? 0 : objects[0]->getLength ()); };

	/** insert a new column at index. Default implementation does nothing. To be implemented in subclasses */
	virtual void doInsertColumn (int index);

	int trailing_rows;
	int trailing_cols;

	int edit_blocks;

	RKVarEditMetaModel* meta_model;
};

/** Represents the meta information portion belonging to an RKVarEditModel. Implemented in a separate class for technical reasons, only (so this info can be displayed in a separate QTableView). This model mostly acts as a slave of an RKVarEditModel. You will not need to call any functions directly except from the RKVarEditModel, or an item view. */
class RKVarEditMetaModel : public QAbstractTableModel {
	Q_OBJECT
public:
	// QAbstractTableModel implementations
	int rowCount (const QModelIndex& parent = QModelIndex()) const;
	int columnCount (const QModelIndex& parent = QModelIndex()) const;
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags (const QModelIndex& index) const;
	bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
protected:
friend class RKVarEditModel;
	RKVarEditMetaModel (RKVarEditModel* datamodel);
	~RKVarEditMetaModel ();

	void beginAddDataObject (int index);
	void endAddDataObject (int index);
	void beginRemoveDataObject (int index);
	void endRemoveDataObject (int index);

	RKVarEditModel* data_model;
};


class RKVarEditDataFrameModel : public RKVarEditModel {
	Q_OBJECT
public:
	RKVarEditDataFrameModel (RContainerObject* dataframe, QObject *parent);
	~RKVarEditDataFrameModel ();

	bool insertColumns (int column, int count, const QModelIndex& parent = QModelIndex());
	bool removeColumns (int column, int count, const QModelIndex& parent = QModelIndex());

	// reimplemented from RKVarEditModel to list for the dataframe object as well
	void objectRemoved (RObject* object);
public slots:
	void objectAdded (RObject* object);
protected:
	RContainerObject* dataframe;
};

#endif
