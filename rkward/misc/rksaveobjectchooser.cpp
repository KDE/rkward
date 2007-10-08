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
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <klocale.h>

#include <../core/robjectlist.h>

#include "../debug.h"

RKSaveObjectChooser::RKSaveObjectChooser (QWidget *parent, const QString &initial, const QString &prompt) : QWidget (parent) {
	RK_TRACE (MISC);

	prev_ok = true;
	object_exists = false;

	Q3VBoxLayout *layout = new Q3VBoxLayout (this);

	QLabel *label = new QLabel (prompt.isNull () ? i18n ("Object name to save to") : prompt, this);
	layout->addWidget (label);

	name_edit = new QLineEdit (this);
	name_edit->setText (initial);
	connect (name_edit, SIGNAL (textChanged (const QString &)), this, SLOT (nameEditChanged (const QString &)));
	layout->addWidget (name_edit);

	overwrite_confirm = new QCheckBox (this);
	connect (overwrite_confirm, SIGNAL (stateChanged (int)), this, SLOT (overwriteConfirmChanged (int)));
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

	return ((!object_exists) || (overwrite_confirm->isChecked ()));
}

void RKSaveObjectChooser::nameEditChanged (const QString &) {
	RK_TRACE (MISC);

	RObject *object = RObjectList::getObjectList ()->findObject (validizedSelectedObjectName ());
	if (object) {
		object_exists = true;
		overwrite_confirm->setText (i18n ("Overwrite? (The given object name already exists)"));
		overwrite_confirm->setEnabled (true);
	} else {
		object_exists = false;
		overwrite_confirm->setText (i18n ("Overwrite?"));
		overwrite_confirm->setEnabled (false);
		overwrite_confirm->setChecked (false);
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
