/***************************************************************************
                          rkrecoverdialog  -  description
                             -------------------
    begin                : Fri Feb 04 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#include "../settings/rksettingsmodulegeneral.h"

#include "../debug.h"

RKRecoverDialog::RKRecoverDialog (const QStringList &recovery_files) {
	RK_TRACE (DIALOGS);
	RK_ASSERT (!recovery_files.isEmpty ());

	setCaption (i18n ("Crash recovery file detected"));
	setButtons (KDialog::Ok | KDialog::Cancel | KDialog::User1);
	setButtonText (KDialog::User1, i18n ("Show in file browser"));
	setButtonText (KDialog::Ok, i18n ("Recover"));
	connect (this, SIGNAL (user1Clicked()), this, SLOT (showButtonClicked ()));

	QLabel *label = new QLabel (this);
	QString text = i18n ("<p><b>Crash recovery file detected</b></p>");
	text.append (i18n ("<p>It looks like RKWard has crashed, recently. We are sorry about that! However, not everything is lost, and with a bit of luck, your data has been saved in time.</p>"));
	text.append (i18np ("<p>A workspace recovery file exists in <i>%2</i> as <i>%3</i>.</p>", "<p>%1 workspace recovery files exist in <i>%2</i>, the most recent one of which is <i>%3</i>.</p>", recovery_files.count (), RKSettingsModuleGeneral::filesPath (), recovery_files.first ()));
	text.append (i18n ("<p>Do you want to open this file, now? <b>Note</b>: You will be prompted again, next time you start RKWard, until you remove/rename the file, manually.</p>"));
	label->setText (text);
	label->setWordWrap (true);
	setMainWidget (label);
}

RKRecoverDialog::~RKRecoverDialog () {
	RK_TRACE (DIALOGS);
}

void RKRecoverDialog::showButtonClicked () {
	RK_TRACE (DIALOGS);

	new KRun (KUrl::fromLocalFile (RKSettingsModuleGeneral::filesPath ()), this);	// KRun auto-deletes itself by default
	reject ();
}

//static
KUrl RKRecoverDialog::checkRecoverCrashedWorkspace () {
	RK_TRACE (DIALOGS);

	QDir dir (RKSettingsModuleGeneral::filesPath ());
	dir.setNameFilters (QStringList ("rkward_recover*.RData"));
	QStringList matches = dir.entryList (QDir::Files, QDir::Time);

	if (!matches.isEmpty ()) {
		RKRecoverDialog dialog (matches);
		dialog.exec ();
		if (dialog.result () == QDialog::Accepted) return (KUrl::fromLocalFile (dir.absoluteFilePath (matches.first ())));
	}

	return KUrl ();
}

#include "rkrecoverdialog.moc"
