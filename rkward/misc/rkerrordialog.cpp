/***************************************************************************
                          rkerrordialog  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rkerrordialog.h"

#include <qdialog.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qlabel.h>

RKErrorDialog::RKErrorDialog (const QString &text, const QString &caption, bool modal) : QObject () {
	RKErrorDialog::text = text;
	RKErrorDialog::caption = caption;
	show_modal = modal;
	dialog = 0;
}

RKErrorDialog::~RKErrorDialog () {
	// dialog deletes itself via Qt::WDestructiveClose!
	//delete dialog;
}

void RKErrorDialog::newError (const QString &error) {
	if (!dialog) createDialog ();
	error_log->append (error);
}

void RKErrorDialog::dialogDestroyed () {
	dialog = 0;
}

void RKErrorDialog::createDialog () {
	dialog = new QDialog (0, 0, show_modal, Qt::WDestructiveClose);
	dialog->setCaption (caption);
	connect (dialog, SIGNAL (destroyed ()), this, SLOT (dialogDestroyed ()));
	
	QGridLayout *grid = new QGridLayout (dialog, 1, 1, 11, 6);
	QVBoxLayout *vbox = new QVBoxLayout (0,0, 6);
	grid->addLayout (vbox, 0, 0);
	
	QLabel *label = new QLabel (text, dialog);
	vbox->addWidget (label);
	
	error_log = new QTextEdit (dialog);
	error_log->setReadOnly (true);
	vbox->addWidget (error_log);
	
	dialog->show ();
}

#include "rkerrordialog.moc"
