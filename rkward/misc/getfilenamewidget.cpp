/*
getfilenamewidget - This file is part of RKWard (https://rkward.kde.org). Created: Tue Aug 24 2004
SPDX-FileCopyrightText: 2004-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "getfilenamewidget.h"

#include <qlabel.h>
#include <QVBoxLayout>
#include <QFileDialog>

#include <KLocalizedString>
#include <kurlrequester.h>
#include <KLineEdit>

#include "../settings/rkrecenturls.h"

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
		// KF5 TODO: Still needed?
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
	QUrl initial_url = RKRecentUrls::mostRecentUrl(storage_key).adjusted(QUrl::RemoveFilename);  // storage_key == QString () in the default case is intended
	if (!append.isEmpty ()) {
		if (initial_url.isLocalFile ()) {
			initial_url = QUrl::fromUserInput (append, initial_url.toLocalFile (), QUrl::AssumeLocalFile);
		} else {
			initial_url.setPath (initial_url.path () + '/' + append);
		}
		initial_url = initial_url.adjusted (QUrl::NormalizePathSegments);
	}
	if (initial_url.isLocalFile () || !only_local) {
		if (!initial.isEmpty ()) edit->setUrl (initial_url);
		else edit->setStartDir (initial_url);
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
	edit->setNameFilter (filter);
}

void GetFileNameWidget::updateLastUsedUrl (const QUrl& url) {
	RK_TRACE (MISC);

	if (!url.isValid ()) return;
	RKRecentUrls::addRecentUrl(storage_key, url);
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
		edit->setUrl (QUrl::fromLocalFile (dummy.selectedFiles ().value (0)));
		Q_EMIT (locationChanged ());
	}
}
#endif

void GetFileNameWidget::locationEditChanged (const QString &) {
	RK_TRACE (MISC);
	Q_EMIT locationChanged();
}

QString GetFileNameWidget::getLocation () {
	if (edit->url ().isLocalFile ()) return (edit->url ().toLocalFile ());
	return (edit->url ().url ());
}

void GetFileNameWidget::setStyleSheet (const QString & style) {
	RK_TRACE (MISC);
	edit->setStyleSheet(style);
}
