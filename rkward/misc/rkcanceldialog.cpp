/***************************************************************************
                          rkcanceldialog  -  description
                             -------------------
    begin                : Wed Sep 8 2004
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
#include "rkcanceldialog.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kdialog.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"

#include "../debug.h"

RKCancelDialog::RKCancelDialog (const QString &caption, const QString &text, QWidget *parent, RCommand *associated_command) : QDialog (parent, 0, true) {
	RK_TRACE (DIALOGS);
	setCaption (caption);
	
	QVBoxLayout *layout = new QVBoxLayout (this, KDialog::marginHint (), KDialog::spacingHint ());
	QLabel *label = new QLabel (text, this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	layout->addWidget (label);
	
	QPushButton *cancel_button = new QPushButton (i18n ("Cancel"), this);
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (reject ()));
	cancel_button->setFixedWidth (cancel_button->sizeHint ().width ());
	layout->addWidget (cancel_button, 0, Qt::AlignHCenter);
	
	resize (sizeHint ().expandedTo (QSize (300, 80)));
	
	command = associated_command;
}

RKCancelDialog::~RKCancelDialog () {
	RK_TRACE (DIALOGS);
}

//static
int RKCancelDialog::showCancelDialog (const QString &caption, const QString &text, QWidget *parent, QObject *done_sender, const char *done_signal, RCommand *associated_command) {
	RK_TRACE (DIALOGS);
	RKCancelDialog *dialog = new RKCancelDialog (caption, text, parent, associated_command);
	connect (done_sender, done_signal, dialog, SLOT (completed ()));
	
	int res = dialog->exec ();
	RK_ASSERT ((res == QDialog::Accepted) || (res == QDialog::Rejected));
	
	delete dialog;
	return res;
}

void RKCancelDialog::closeEvent (QCloseEvent *e) {
	RK_TRACE (DIALOGS);
	e->ignore ();
}

void RKCancelDialog::completed () {
	RK_TRACE (DIALOGS);
	accept ();
}

void RKCancelDialog::reject () {
	RK_TRACE (DIALOGS);
	if (command) RKGlobals::rInterface ()->cancelCommand (command);
	QDialog::reject ();
}

#include "rkcanceldialog.moc"
