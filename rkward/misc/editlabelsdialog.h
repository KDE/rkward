/*
editlabelsdialog - This file is part of the RKWard project. Created: Tue Sep 21 2004
SPDX-FileCopyrightText: 2004-2007 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef EDITLABELSDIALOG_H
#define EDITLABELSDIALOG_H

#include <QDialog>
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
public Q_SLOTS:
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

	int rowCount (const QModelIndex& parent = QModelIndex()) const override;
	int columnCount (const QModelIndex& parent = QModelIndex()) const override;
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags (const QModelIndex& index) const override;
	bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
friend class EditLabelsDialogProxy;
	RObject::ValueLabels labels;
};

/**
Allows editing of value labels / factor levels for an RKVariable. Use EditLabelsDialogProxy.

@author Thomas Friedrichsmeier
*/
class EditLabelsDialog : public QDialog {
protected:
friend class EditLabelsDialogProxy;
/** constructor., the variable to work on.
@param parent a QWidget parent */
	EditLabelsDialog (QWidget *parent, const RObject::ValueLabels& labels, const QString& varname);

	~EditLabelsDialog ();

/** reimplemented to make sure pending edit operations are not lost */
	void accept () override;
private:
	RKVarLevelsTable* table;
};

/**
Simple proxy / wrapper to allow using a modal EditLabelsDialog in a QTableView
@author Thomas Friedrichsmeier */
class EditLabelsDialogProxy : public QWidget {
	Q_OBJECT
public:
	explicit EditLabelsDialogProxy (QWidget* parent);
	~EditLabelsDialogProxy ();

	void initialize (const RObject::ValueLabels& labels, const QString& varname);

	RObject::ValueLabels getLabels () const { return labels; };
Q_SIGNALS:
	void done (QWidget* widget, RKItemDelegate::EditorDoneReason reason);
protected Q_SLOTS:
	void dialogDone (int result);
private:
	EditLabelsDialog* dialog;
	RObject::ValueLabels labels;
};

#endif
