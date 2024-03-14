/*
rkrecoverdialog - This file is part of RKWard (https://rkward.kde.org). Created: Fri Feb 04 2011
SPDX-FileCopyrightText: 2011-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrecoverdialog.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDateTime>

#include "../settings/rksettingsmodulegeneral.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkdialogbuttonbox.h"

#include "../debug.h"

RKRecoverDialog::RKRecoverDialog (const QStringList &recovery_files) : QDialog () {
	RK_TRACE (DIALOGS);
	RK_ASSERT (!recovery_files.isEmpty ());
	files = recovery_files;

	QVBoxLayout *layout = new QVBoxLayout (this);

	const QString caption = i18n ("Crash recovery file detected");
	setWindowTitle (caption);

	QString text = QString ("<p><b>%1</b></p>").arg (caption);
	text.append (i18n ("<p>It looks like RKWard has crashed, recently. We are sorry about that! However, not everything is lost, and with a bit of luck, your data has been saved in time.</p>"));
	text.append (i18np ("<p>A workspace recovery file exists, dating from <i>%2</i>.</p>", "<p>%1 workspace recovery files exist, the most recent one of which dates from <i>%2</i>.</p>", recovery_files.count (),  QLocale::system().toString(QFileInfo (recovery_files.first ()).lastModified (), QLocale::ShortFormat)));
	text.append (i18n ("<p>Do you want to open this file, now, save it for later (as <i>%1</i>), or discard it?</p>", saveFileFor (recovery_files.first ())));
	layout->addWidget (RKCommonFunctions::wordWrappedLabel (text));

	RKDialogButtonBox *buttons = new RKDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Discard, this);
	buttons->button (QDialogButtonBox::Ok)->setText (i18n ("Recover"));
	RKCommonFunctions::setTips (i18n ("Saves the recovery file(s), and opens it (or the most recent one)"), buttons->button (QDialogButtonBox::Ok));
	buttons->button (QDialogButtonBox::Cancel)->setText (i18n ("Save for later"));
	RKCommonFunctions::setTips (i18n ("Saves the recovery file(s) for later use, but does not open it"), buttons->button (QDialogButtonBox::Cancel));
	buttons->button (QDialogButtonBox::Discard)->setText (i18n ("Delete"));
	RKCommonFunctions::setTips (i18n ("Deletes the recovery file(s)"), buttons->button (QDialogButtonBox::Discard));
	connect (buttons->button (QDialogButtonBox::Discard), &QPushButton::clicked, this, &RKRecoverDialog::deleteButtonClicked);
	layout->addWidget (buttons);
}

RKRecoverDialog::~RKRecoverDialog () {
	RK_TRACE (DIALOGS);
}

void RKRecoverDialog::deleteButtonClicked () {
	RK_TRACE (DIALOGS);

	if (KMessageBox::warningContinueCancel (this, i18np ("You are about to delete the recovery file %2. There will be no way to bring it back. Really delete it?", "You are about to delete %1 recovery files (the most recent one is %2). There will be no way to bring them back. Really delete them?", files.count (), files.first ()), i18n ("Really delete recovery file(s)?"), KStandardGuiItem::del()) != KMessageBox::Continue) return;

	for (int i = 0; i < files.count (); ++i) {
		QFile (files[i]).remove ();	// TODO: error handling?
	}
	files.clear ();
	reject ();
}

//static
QString RKRecoverDialog::saveFileFor (const QString& recovery_file) {
	RK_TRACE (DIALOGS);

	QFileInfo fi (recovery_file);
	QDateTime mtime = fi.lastModified ();
	QDir dir = fi.absoluteDir ();
	QString new_name;
	for (int i = 0; i < 100; ++i) {	// If we just had more than 100 crashes per minutes, you'll excuse another small bug, at this point
		QString num;
		if (i > 0) num = '_' + QString::number (i+1);
		new_name = dir.absoluteFilePath ("recovered_workspace_" + mtime.toString ("yyyy-MM-dd_hh:mm") + num + ".RData");
		if (!QFileInfo::exists(new_name)) break;
	}
	return new_name;
}

//static
QUrl RKRecoverDialog::checkRecoverCrashedWorkspace () {
	RK_TRACE (DIALOGS);

	QDir dir (RKSettingsModuleGeneral::filesPath ());
	dir.setNameFilters (QStringList ("rkward_recover*.RData"));
	QStringList matches = dir.entryList (QDir::Files, QDir::Time);
	for (int i = 0; i < matches.count (); ++i) {
		matches[i] = dir.absoluteFilePath (matches[i]);
	}

	if (!matches.isEmpty ()) {
		RKRecoverDialog dialog (matches);
		dialog.exec ();

		// "Save" recovery files, so they want be matched, again
		matches = dialog.files;	// May have been modified, notably deleted
		for (int i = matches.count () - 1; i >= 0; --i) {
			QString new_name = saveFileFor (matches[i]);
			QFile::rename (matches[i], new_name);
			matches[i] = new_name;
		}

		if (dialog.result () == QDialog::Accepted) return (QUrl::fromLocalFile (dir.absoluteFilePath (matches.first ())));
	}

	return QUrl ();
}

