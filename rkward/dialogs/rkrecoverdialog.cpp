/***************************************************************************
                          rkrecoverdialog  -  description
                             -------------------
    begin                : Fri Feb 04 2011
    copyright            : (C) 2011, 2012, 2014 by Thomas Friedrichsmeier
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

#include "rkrecoverdialog.h"

#include <krun.h>
#include <klocale.h>

#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <kmessagebox.h>

#include "../settings/rksettingsmodulegeneral.h"

#include "../debug.h"

RKRecoverDialog::RKRecoverDialog (const QStringList &recovery_files) {
	RK_TRACE (DIALOGS);
	RK_ASSERT (!recovery_files.isEmpty ());
	files = recovery_files;

	const QString caption = i18n ("Crash recovery file detected");
	setCaption (caption);
	setButtons (KDialog::Ok | KDialog::Cancel | KDialog::User1);
	setButtonText (KDialog::Ok, i18n ("Recover"));
	setButtonToolTip (KDialog::Ok, i18n ("Saves the recovery file(s), and opens it (or the most recent one)"));
	setButtonWhatsThis (KDialog::Ok, buttonToolTip (KDialog::Ok));
	setButtonText (KDialog::Cancel, i18n ("Save for later"));
	setButtonToolTip (KDialog::Cancel, i18n ("Saves the recovery file(s) for later use, but does not open it"));
	setButtonWhatsThis (KDialog::Cancel, buttonToolTip (KDialog::Cancel));
	setButtonText (KDialog::User1, i18n ("Delete"));
	setButtonToolTip (KDialog::User1, i18n ("Deletes the recovery file(s)"));
	setButtonWhatsThis (KDialog::User1, buttonToolTip (KDialog::User1));
	
	connect (this, SIGNAL (user1Clicked()), this, SLOT (deleteButtonClicked ()));
	QLabel *label = new QLabel (this);
	QString text = QString ("<p><b>%1</b></p>").arg (caption);
	text.append (i18n ("<p>It looks like RKWard has crashed, recently. We are sorry about that! However, not everything is lost, and with a bit of luck, your data has been saved in time.</p>"));
	text.append (i18np ("<p>A workspace recovery file exists, dating from <i>%2</i></p>", "<p>%1 workspace recovery files exist, the most recent one of which dates from <i>%2</i>.</p>", recovery_files.count (), QFileInfo (recovery_files.first ()).lastModified ().toString (Qt::SystemLocaleLongDate)));
	text.append (i18n ("<p>Do you want to open this file, now, save it for later (as %1), or discard it?</p>", saveFileFor (recovery_files.first ())));
	label->setText (text);
	label->setWordWrap (true);
	setMainWidget (label);
}

RKRecoverDialog::~RKRecoverDialog () {
	RK_TRACE (DIALOGS);
}

void RKRecoverDialog::deleteButtonClicked () {
	RK_TRACE (DIALOGS);

	if (KMessageBox::warningContinueCancel (this, i18np ("You are about to delete the recovery file %2. There will be no way to bring it back. Continue?", "You are about to delete %1 recovery files (the most recent one is %2). There will be no way to bring them back. Continue?", files.count (), files.first ())) != KMessageBox::Continue) return;

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
		if (i > 0) num = "_" + QString::number (i+1);
		new_name = dir.absoluteFilePath ("recovered_workspace_" + mtime.toString ("yyyy-MM-dd_hh:mm") + num + ".RData");
		if (!QFileInfo (new_name).exists ()) break;
	}
	return new_name;
}

//static
KUrl RKRecoverDialog::checkRecoverCrashedWorkspace () {
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

		if (dialog.result () == QDialog::Accepted) return (KUrl::fromLocalFile (dir.absoluteFilePath (matches.first ())));
	}

	return KUrl ();
}

#include "rkrecoverdialog.moc"
