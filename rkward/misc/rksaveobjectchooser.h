/***************************************************************************
                          rksaveobjectchooser  -  description
                             -------------------
    begin                : Mon Nov 27 2006
    copyright            : (C) 2006, 2007 by Thomas Friedrichsmeier
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

#ifndef RKSAVEOBJECTCHOOSER_H
#define RKSAVEOBJECTCHOOSER_H

#include <qwidget.h>
#include "../core/rkmodificationtracker.h"

class QLineEdit;
class QCheckBox;
class QPushButton;
class QLabel;

/** Simple helper widget to select an R symbol name to write something to. */
class RKSaveObjectChooser : public QWidget, public RObjectListener {
	Q_OBJECT
public:
	RKSaveObjectChooser (QWidget *parent, const QString &initial);
	~RKSaveObjectChooser ();

	QString currentFullName () const { return current_full_name; };
	QString currentBaseName () const;
	bool isOk () const;
	void setBaseName (const QString &name);
	void setBackgroundColor (const QColor &color);
	RObject* rootObject () const { return root_object; };
	void setRootObject (RObject* new_root);
private slots:
	void updateState ();
	void selectRootObject ();
signals:
	void changed (bool);
protected:
	void objectRemoved (RObject* removed);
	void childAdded (int index, RObject* parent);
private:
	bool object_exists;

	RObject *current_object;
	RObject *root_object;
	QLabel *root_label;
	QPushButton *root_button;
	QLineEdit *name_edit;
	QCheckBox *overwrite_confirm;

	QString current_full_name;
};

#endif
