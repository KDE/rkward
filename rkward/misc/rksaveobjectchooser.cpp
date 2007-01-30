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

#include "rksaveobjectchooser.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>

#include <klocale.h>

#include <../core/robjectlist.h>

#include "../debug.h"

RKSaveObjectChooser::RKSaveObjectChooser (QWidget *parent, bool allow_overwrite, const QString &initial, const QString &prompt) : QWidget (parent) {
	RK_TRACE (MISC);

	RKSaveObjectChooser::allow_overwrite = allow_overwrite;
	prev_ok = true;
	object_exists = false;

	QVBoxLayout *layout = new QVBoxLayout (this);

	QLabel *label = new QLabel (prompt.isNull () ? i18n ("Object name to save to") : prompt, this);
	layout->addWidget (label);

	name_edit = new QLineEdit (this);
	name_edit->setText (initial);
	connect (name_edit, SIGNAL (textChanged (const QString &)), this, SLOT (nameEditChanged (const QString &)));
	layout->addWidget (name_edit);

	inuse_label = new QLabel (i18n ("The given symbol already exists"), this);
	inuse_label->hide ();
	layout->addWidget (inuse_label);

	overwrite_confirm = new QCheckBox (i18n ("Overwrite?"), this);
	connect (overwrite_confirm, SIGNAL (stateChanged (int)), this, SLOT (overwriteConfirmChanged (int)));
	overwrite_confirm->hide ();
	layout->addWidget (overwrite_confirm);

	nameEditChanged (QString ());	// initialize
}

RKSaveObjectChooser::~RKSaveObjectChooser () {
	RK_TRACE (MISC);
}

void RKSaveObjectChooser::setObjectName (const QString &name) {
	RK_TRACE (MISC);

	name_edit->setText (name);
	nameEditChanged (name);
}

QString RKSaveObjectChooser::validizedSelectedObjectName () {
	RK_TRACE (MISC);

	return (RObjectList::getObjectList ()->validizeName (name_edit->text (), false));
}

bool RKSaveObjectChooser::isOk () const {
	RK_TRACE (MISC);

	return ((!object_exists) || (allow_overwrite && overwrite_confirm->isChecked ()));
}

void RKSaveObjectChooser::nameEditChanged (const QString &) {
	RK_TRACE (MISC);

	RObject *object = RObjectList::getObjectList ()->findObject (validizedSelectedObjectName ());
	if (object) {
		object_exists = true;
		inuse_label->show ();
		if (allow_overwrite) overwrite_confirm->show ();
	} else {
		object_exists = false;
		inuse_label->hide ();
		if (allow_overwrite) {
			overwrite_confirm->hide ();
			overwrite_confirm->setChecked (false);
		}
	}

	if (isOk () != prev_ok) {
		prev_ok = isOk ();
		emit (okStatusChanged (isOk ()));
	}
	emit (changed ());
}

void RKSaveObjectChooser::overwriteConfirmChanged (int) {
	RK_TRACE (MISC);

	nameEditChanged (QString ());
}

void RKSaveObjectChooser::setBackgroundColor (const QColor &color) {
	RK_TRACE (MISC);

	name_edit->setBackgroundColor (color);
}

#include "rksaveobjectchooser.moc"
