/*
rksaveobjectchooser - This file is part of the RKWard project. Created: Mon Nov 27 2006
SPDX-FileCopyrightText: 2006-2007 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	void setStyleSheet (const QString &style);
	RObject* rootObject () const { return root_object; };
	void setRootObject (RObject* new_root);
private slots:
	void updateState ();
	void selectRootObject ();
signals:
	void changed (bool);
protected:
	void objectRemoved (RObject* removed) override;
	void childAdded (int index, RObject* parent) override;
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
