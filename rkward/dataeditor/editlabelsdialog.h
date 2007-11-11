/***************************************************************************
                          editlabelsdialog  -  description
                             -------------------
    begin                : Tue Sep 21 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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
#ifndef EDITLABELSDIALOG_H
#define EDITLABELSDIALOG_H

#include <kdialog.h>

#include <QTableView>
#include <QAbstractTableModel>

#include "../core/robject.h"

class RKVariable;
class RKVarLevelsTableModel;

/** special mini class provides the table in EditLabelsDialog
TODO: make copy/paste work

@author Thomas Friedrichsmeier
*/
class RKVarLevelsTable : public QTableView {
	Q_OBJECT
public:
	RKVarLevelsTable (QWidget *parent, RObject::ValueLabels *labels);
	~RKVarLevelsTable ();
public slots:
/** cut */
	void cut ();
/** cut */
	void copy ();
/** paste */
	void paste ();
private:
	bool getSelectionBoundaries (int* top, int* bottom) const;
friend class EditLabelsDialog;
	RKVarLevelsTableModel* lmodel;
	bool updating_size;
};

/** Data model for the RKVarLevelsTable */
class RKVarLevelsTableModel : public QAbstractTableModel {
public:
	RKVarLevelsTableModel (RObject::ValueLabels* labels, QObject* parent);
	~RKVarLevelsTableModel ();

	int rowCount (const QModelIndex& parent = QModelIndex()) const;
	int columnCount (const QModelIndex& parent = QModelIndex()) const;
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags (const QModelIndex& index) const;
	bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
private:
friend class EditLabelsDialog;
	RObject::ValueLabels* labels;
};

/**
Allows editing of value labels / factor levels for an (edited) RKVariable

@author Thomas Friedrichsmeier
*/
class EditLabelsDialog : public KDialog {
	Q_OBJECT
public:
/** constuctor., the variable to work on.
@param parent a QWidget parent (usually 0)
@param var the variable/factor to set labels for
@param mode not yet used */
	EditLabelsDialog (QWidget *parent, RKVariable *var, int mode=0);

	~EditLabelsDialog ();
protected:
/// reimplemented to submit the changes to the backend
	void accept ();
	void reject ();
private:
	RKVarLevelsTable *table;
	RKVariable *var;
	int mode;
};

#endif
