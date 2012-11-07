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

#include <QAbstractTableModel>

#include "../core/robject.h"
#include "rktableview.h"

class RKVariable;
class RKVarLevelsTableModel;

/** special mini class provides the table in EditLabelsDialog

@author Thomas Friedrichsmeier
*/
class RKVarLevelsTable : public RKTableView {
	Q_OBJECT
public:
	RKVarLevelsTable (QWidget *parent, const RObject::ValueLabels& labels);
	~RKVarLevelsTable ();
public slots:
/** cut */
	void cut ();
/** cut */
	void copy ();
/** paste */
	void paste ();
	void blankSelected ();
private:
friend class EditLabelsDialogProxy;
	RKVarLevelsTableModel* lmodel;
	bool updating_size;
};

/** Data model for the RKVarLevelsTable */
class RKVarLevelsTableModel : public QAbstractTableModel {
public:
	RKVarLevelsTableModel (const RObject::ValueLabels& labels, QObject* parent);
	~RKVarLevelsTableModel ();

	int rowCount (const QModelIndex& parent = QModelIndex()) const;
	int columnCount (const QModelIndex& parent = QModelIndex()) const;
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags (const QModelIndex& index) const;
	bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
private:
friend class EditLabelsDialogProxy;
	RObject::ValueLabels labels;
};

/**
Allows editing of value labels / factor levels for an RKVariable. Use EditLabelsDialogProxy.

@author Thomas Friedrichsmeier
*/
class EditLabelsDialog : public KDialog {
protected:
friend class EditLabelsDialogProxy;
/** constuctor., the variable to work on.
@param parent a QWidget parent */
	EditLabelsDialog (QWidget *parent, const RObject::ValueLabels& labels, const QString& varname);

	~EditLabelsDialog ();

/** reimplemented to make sure pending edit operations are not lost */
	void accept ();
private:
	RKVarLevelsTable* table;
};

/**
Simple proxy / wrapper to allow using a modal EditLabelsDialog in a QTableView
@author Thomas Friedrichsmeier */
class EditLabelsDialogProxy : public QWidget {
	Q_OBJECT
public:
	EditLabelsDialogProxy (QWidget* parent);
	~EditLabelsDialogProxy ();

	void initialize (const RObject::ValueLabels& labels, const QString& varname);

	RObject::ValueLabels getLabels () const { return labels; };
signals:
	void done (QWidget* widget, RKItemDelegate::EditorDoneReason reason);
protected slots:
	void dialogDone (int result);
private:
	EditLabelsDialog* dialog;
	RObject::ValueLabels labels;
};

#endif
