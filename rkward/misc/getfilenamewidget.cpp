/***************************************************************************
                          getfilenamewidget  -  description
                             -------------------
    begin                : Tue Aug 24 2004
    copyright            : (C) 2004, 2007, 2009, 2010, 2012, 2015 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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
#include <QFileDialog>

#include <klocale.h>
#include <KLineEdit>
#include <kurlrequester.h>

#include "../settings/rksettingsmodulegeneral.h"

#include "../debug.h"

GetFileNameWidget::GetFileNameWidget (QWidget *parent, FileType mode, bool only_local, const QString &label, const QString &caption, const QString &initial) : QWidget (parent) {
	RK_TRACE (MISC);
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	vbox->setSizeConstraint (QLayout::SetMinimumSize);

	if (!label.isEmpty ()) vbox->addWidget (new QLabel (label, this));

	edit = new KUrlRequester (this);
	vbox->addWidget (edit);

	_mode = mode;
	KFile::Modes mode_flags;
	if (mode == ExistingDirectory) {
		mode_flags = KFile::Directory | KFile::ExistingOnly;
#ifdef Q_OS_WIN
		// TODO: Hang on Windows when trying to select any dir (also in KFileDialog::getExistingDirectory ()). KDE 4.10
		// this hack works around this, by using QFileDialog::getExistingDirectory ().
		edit->button ()->disconnect (SIGNAL (clicked())); // Use old and new syntax, as we don't know, which way it was connected
		disconnect (edit->button (), &QPushButton::clicked, 0, 0);
		connect (edit->button (), &QPushButton::clicked, this, &GetFileNameWidget::hackOverrideDirDialog);
#endif
	} else if (mode == ExistingFile) {
		mode_flags = KFile::File | KFile::ExistingOnly;
	} else if (mode == SaveFile) {
		mode_flags = KFile::File;
	} else {
		RK_ASSERT (false);
	}
	if (only_local) mode_flags |= KFile::LocalOnly;
	edit->setMode (mode_flags);

	QString append = initial;
	if (initial.startsWith ('<')) {
		storage_key = initial.section ('>', 0, 0).mid (1);
		append = initial.section ('>', 1);
	}
	QUrl initial_url = RKSettingsModuleGeneral::lastUsedUrlFor (storage_key);  // storage_key == QString () in the default case is intended
	if (!append.isEmpty ()) {
		if (initial_url.isLocalFile ()) {
			initial_url = QUrl::fromUserInput (append, initial_url.toLocalFile (), QUrl::AssumeLocalFile);
		} else {
			initial_url.setPath (initial_url.path () + '/' + append);
		}
		initial_url = initial_url.adjusted (QUrl::NormalizePathSegments);
	}
	if (initial_url.isLocalFile () || !only_local) {
		edit->setUrl (initial_url);
	}
	connect (edit, &KUrlRequester::textChanged, this, &GetFileNameWidget::locationEditChanged);
	connect (edit, &KUrlRequester::urlSelected, this, &GetFileNameWidget::updateLastUsedUrl);

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

void GetFileNameWidget::updateLastUsedUrl (const QUrl& url) {
	RK_TRACE (MISC);

	if (!url.isValid ()) return;
	if (edit->mode () & KFile::Directory) RKSettingsModuleGeneral::updateLastUsedUrl (storage_key, url);
	else RKSettingsModuleGeneral::updateLastUsedUrl (storage_key, url.adjusted (QUrl::RemoveFilename));
}

void GetFileNameWidget::setLocation (const QString &new_location) {
	RK_TRACE (MISC);

	if (edit->text () != new_location) edit->setUrl (QUrl::fromUserInput (new_location, QString (), QUrl::AssumeLocalFile));
}

#ifdef Q_OS_WIN
void GetFileNameWidget::hackOverrideDirDialog () {
	RK_TRACE (MISC);

	// TODO: Hang on Windows when trying to select any dir using (K|Q)FileDialog::getExistingDirectory (). KDE 4.10
	QFileDialog dummy (this, edit->windowTitle (), edit->startDir ().toLocalFile ());
	dummy.setFileMode (QFileDialog::Directory);
	dummy.setOptions (QFileDialog::ShowDirsOnly);
	if (dummy.exec ()) {
		edit->setUrl (dummy.selectedFiles ().value (0));
		emit (locationChanged ());
	}
}
#endif

void GetFileNameWidget::locationEditChanged (const QString &) {
	RK_TRACE (MISC);
	emit (locationChanged ());
}

QString GetFileNameWidget::getLocation () {
	if (edit->url ().isLocalFile ()) return (edit->url ().toLocalFile ());
	return (edit->url ().url ());
}

void GetFileNameWidget::setBackgroundColor (const QColor & color) {
	RK_TRACE (MISC);

	QPalette palette = edit->lineEdit ()->palette ();
	palette.setColor (edit->lineEdit ()->backgroundRole (), color);
	edit->lineEdit ()->setPalette (palette);
}

