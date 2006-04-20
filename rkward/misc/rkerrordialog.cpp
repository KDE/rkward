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
#include <klocale.h>

#include "../debug.h"

RKErrorDialog::RKErrorDialog (const QString &text, const QString &caption, bool modal) : QObject () {
	RK_TRACE (MISC);

	RKErrorDialog::text = text;
	RKErrorDialog::caption = caption;
	show_modal = modal;
	dialog = 0;
}

RKErrorDialog::~RKErrorDialog () {
	RK_TRACE (MISC);

	// dialog deletes itself via Qt::WDestructiveClose!
	//delete dialog;
}

void RKErrorDialog::newError (const QString &error) {
	RK_TRACE (MISC);

	if (!dialog) createDialog ();
	error_log->append (error);
}

void RKErrorDialog::newOutput (const QString &output) {
	RK_TRACE (MISC);
	if (dialog) {
		error_log->append (output);
	} else {
		stored_output.append (output);
	}
}

void RKErrorDialog::resetOutput () {
	RK_TRACE (MISC);
	stored_output = QString::null;
}

void RKErrorDialog::dialogDestroyed () {
	RK_TRACE (MISC);
	dialog = 0;
}

void RKErrorDialog::createDialog () {
	RK_TRACE (MISC);

	dialog = new QDialog (0, 0, show_modal, Qt::WDestructiveClose);
	dialog->setCaption (caption);
	connect (dialog, SIGNAL (destroyed ()), this, SLOT (dialogDestroyed ()));
	
	QGridLayout *grid = new QGridLayout (dialog, 1, 1, 11, 6);
	QVBoxLayout *vbox = new QVBoxLayout (0,0, 6);
	grid->addLayout (vbox, 0, 0);
	
	QLabel *label = new QLabel (text, dialog);
	label->setAlignment (Qt::AlignAuto | Qt::ExpandTabs | Qt::WordBreak);
	vbox->addWidget (label);
	
	error_log = new QTextEdit (dialog);
	error_log->setReadOnly (true);
	error_log->append (stored_output);
	stored_output = QString::null;
	vbox->addWidget (error_log);
	
	dialog->show ();
}

////////////////////////////////////// RKRErrorDialog /////////////////////////////////////

#include "../rbackend/rcommand.h"

RKRErrorDialog::RKRErrorDialog (const QString &text, const QString &caption, bool modal) : RKErrorDialog (text, caption, modal) {
	RK_TRACE (MISC);
}

RKRErrorDialog::~RKRErrorDialog () {
	RK_TRACE (MISC);
}

void RKRErrorDialog::addRCommand (RCommand *command) {
	RK_TRACE (MISC);

	if (command->hasOutput ()) {
		RKErrorDialog::newOutput (command->output ());
	}

	if (command->hasError ()) {
		newError (command->error ());
	}

	if (command->errorSyntax ()) {
		newError (i18n ("Syntax error"));
	} else if (command->errorIncomplete ()) {
		newError (i18n ("command incomplete"));
	}
}

void RKRErrorDialog::rCommandDone (RCommand *command) {
	RK_TRACE (MISC);

	addRCommand (command);
}

#include "rkerrordialog.moc"
