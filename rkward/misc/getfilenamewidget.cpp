/***************************************************************************
                          getfilenamewidget  -  description
                             -------------------
    begin                : Tue Aug 24 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
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
#include "getfilenamewidget.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <klocale.h>
#include <kfiledialog.h>

#include "../debug.h"

GetFileNameWidget::GetFileNameWidget (QWidget *parent, FileType mode, const QString &label, const QString &caption, const QString &initial) : QWidget (parent) {
	RK_TRACE (MISC);
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setResizeMode (QLayout::Minimum);

	vbox->addWidget (new QLabel (label, this));

	QHBoxLayout *hbox = new QHBoxLayout (vbox, 6);

	location_edit = new QLineEdit (this);
	location_edit->setText (initial);
	connect (location_edit, SIGNAL (textChanged (const QString &)), this, SLOT (locationEditChanged (const QString &)));
	hbox->addWidget (location_edit);
	
	browse_button = new QPushButton (i18n ("Browse"), this);
	connect (browse_button, SIGNAL (clicked ()), this, SLOT (browseButtonClicked ()));
	hbox->addWidget (browse_button);
	
	GetFileNameWidget::mode = mode;
	
	if (caption.isEmpty ()) {
		GetFileNameWidget::caption = label;
	} else {
		GetFileNameWidget::caption = caption;
	}
}

GetFileNameWidget::~GetFileNameWidget () {
	RK_TRACE (MISC);
}

void GetFileNameWidget::setLocation (const QString &new_location) {
	RK_TRACE (MISC);

	location_edit->setText (new_location);
}

void GetFileNameWidget::locationEditChanged (const QString &) {
	RK_TRACE (MISC);
	emit (locationChanged ());
}

void GetFileNameWidget::browseButtonClicked () {
	RK_TRACE (MISC);
	QString temp;
	if (mode == ExistingDirectory) {
		temp = KFileDialog::getExistingDirectory (location_edit->text (), this, caption);
	} else if (mode == ExistingFile) {
		temp = KFileDialog::getOpenFileName (location_edit->text (), _filter, this, caption);
	} else if (mode == SaveFile) {
		temp = KFileDialog::getSaveFileName (location_edit->text (), _filter, this, caption);
	} else {
		RK_ASSERT (false);
	}

	if (!temp.isEmpty ()) {
		location_edit->setText (temp);
	}
}

QString GetFileNameWidget::getLocation () {
	return location_edit->text ();
}

void GetFileNameWidget::setBackgroundColor (const QColor & color) {
	RK_TRACE (MISC);

	location_edit->setBackgroundColor (color);
}

#include "getfilenamewidget.moc"
