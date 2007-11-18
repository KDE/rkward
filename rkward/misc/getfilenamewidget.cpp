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

#include <qlabel.h>
#include <QVBoxLayout>

#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>

#include "../debug.h"

GetFileNameWidget::GetFileNameWidget (QWidget *parent, FileType mode, const QString &label, const QString &caption, const QString &initial) : QWidget (parent) {
	RK_TRACE (MISC);
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	vbox->setSizeConstraint (QLayout::SetMinimumSize);

	vbox->addWidget (new QLabel (label, this));

	edit = new KUrlRequester (this);
	connect (edit, SIGNAL (textChanged (const QString &)), this, SLOT (locationEditChanged (const QString &)));
	vbox->addWidget (edit);

	edit->setUrl (initial);
	if (mode == ExistingDirectory) {
		edit->setMode (KFile::Directory | KFile::ExistingOnly);
	} else if (mode == ExistingFile) {
		edit->setMode (KFile::File | KFile::ExistingOnly);
	} else if (mode == SaveFile) {
		edit->setMode (KFile::File);
	} else {
		RK_ASSERT (false);
	}

	if (caption.isEmpty ()) edit->setWindowTitle (label);
	else edit->setWindowTitle (caption);
}

GetFileNameWidget::~GetFileNameWidget () {
	RK_TRACE (MISC);
}

void GetFileNameWidget::setFilter (const QString &filter) {
	RK_TRACE (MISC);

	RK_ASSERT (edit);
	edit->setFilter (filter);
}

void GetFileNameWidget::setLocation (const QString &new_location) {
	RK_TRACE (MISC);

	edit->setUrl (new_location);
}

void GetFileNameWidget::locationEditChanged (const QString &) {
	RK_TRACE (MISC);
	emit (locationChanged ());
}

QString GetFileNameWidget::getLocation () {
	return (edit->url ().path ());
}

void GetFileNameWidget::setBackgroundColor (const QColor & color) {
	RK_TRACE (MISC);

	QPalette palette = edit->lineEdit ()->palette ();
	palette.setColor (edit->lineEdit ()->backgroundRole (), color);
	edit->lineEdit ()->setPalette (palette);
}

#include "getfilenamewidget.moc"
