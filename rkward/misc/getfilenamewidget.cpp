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
#include <qlabel.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>

#include "../debug.h"

GetFileNameWidget::GetFileNameWidget (QWidget *parent, FileType mode, const QString &label, const QString &caption, const QString &initial) : QWidget (parent) {
	RK_TRACE (MISC);
	Q3VBoxLayout *vbox = new Q3VBoxLayout (this);
	vbox->setResizeMode (QLayout::Minimum);

	vbox->addWidget (new QLabel (label, this));

	edit = new KUrlRequester (this);
	connect (edit, SIGNAL (textChanged (const QString &)), this, SLOT (locationEditChanged (const QString &)));
	vbox->addWidget (edit);

	edit->setURL (initial);
	if (mode == ExistingDirectory) {
		edit->setMode (KFile::Directory | KFile::ExistingOnly);
	} else if (mode == ExistingFile) {
		edit->setMode (KFile::File | KFile::ExistingOnly);
	} else if (mode == SaveFile) {
		edit->setMode (KFile::File);
	} else {
		RK_ASSERT (false);
	}

	if (caption.isEmpty ()) edit->setCaption (label);
	else edit->setCaption (caption);
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

	edit->setURL (new_location);
}

void GetFileNameWidget::locationEditChanged (const QString &) {
	RK_TRACE (MISC);
	emit (locationChanged ());
}

QString GetFileNameWidget::getLocation () {
	return edit->url ();
}

void GetFileNameWidget::setBackgroundColor (const QColor & color) {
	RK_TRACE (MISC);

	edit->lineEdit ()->setBackgroundColor (color);
}

#include "getfilenamewidget.moc"
