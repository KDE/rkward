/***************************************************************************
                          editlabelsdialog  -  description
                             -------------------
    begin                : Tue Sep 21 2004
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
#ifndef EDITLABELSDIALOG_H
#define EDITLABELSDIALOG_H

#include <qdialog.h>

#include "../core/robject.h"

class RKVariable;
class QListView;
class QLineEdit;
class QPushButton;
class QListViewItem;

/**
Allows editing of value labels / factor levels for an (edited) RKVariable

@author Thomas Friedrichsmeier
*/

class EditLabelsDialog : public QDialog {
Q_OBJECT
public:
	EditLabelsDialog (QWidget *parent, RKVariable *var, int mode=0);

	~EditLabelsDialog ();
public slots:
	void addButtonClicked ();
	void removeButtonClicked ();
	void changeButtonClicked ();
	void listSelectionChanged (QListViewItem *item);
	void labelEditEnterPressed ();
protected:
/// reimplemented to submit the changes to the backend
	void accept ();
private:
	QListView *list;
	RKVariable *var;
	RObject::ValueLabels *labels;
	int mode;
	QPushButton *add_button;
	QPushButton *remove_button;
	QPushButton *change_button;
	QLineEdit *label_edit;
};

#endif
