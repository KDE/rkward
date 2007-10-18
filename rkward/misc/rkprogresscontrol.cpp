/***************************************************************************
                          rkprogresscontol  -  description
                             -------------------
    begin                : Sun Sep 10 2006
    copyright            : (C) 2006, 2007 by Thomas Friedrichsmeier
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

#include "rkprogresscontrol.h"

#include <klocale.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"

#include "../debug.h"
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include <Q3ValueList>
#include <Q3VBoxLayout>

RKProgressControl::RKProgressControl (QObject *parent, const QString &text, const QString &caption, int mode_flags) : QObject (parent) {
	RK_TRACE (MISC);

	RKProgressControl::text = text;
	RKProgressControl::caption = caption;
	RKProgressControl::mode = mode_flags;

	dialog = 0;
	is_done = false;
	modal = false;
	autodelete = false;
	done_command = 0;
}

RKProgressControl::~RKProgressControl () {
	RK_TRACE (MISC);

	if (!is_done) done ();
}

bool RKProgressControl::doModal (bool autodelete) {
	RK_TRACE (MISC);
	RK_ASSERT (!dialog);

	modal = true;
	createDialog ();

	int res = dialog->exec ();
	RK_ASSERT ((res == QDialog::Accepted) || (res == QDialog::Rejected));

	bool ret = is_done;		// for good style, we copy this onto the stack, as we are about to self destruct
	if (autodelete) deleteLater ();

	return ret;
}

void RKProgressControl::doNonModal (bool autodelete) {
	RK_TRACE (MISC);
	RK_ASSERT (!dialog);

	RKProgressControl::autodelete = autodelete;
	if ((!dialog) && (mode & ShowAtOnce)) {		// actually, dialog should alway be 0 at this point
		createDialog ();
		dialog->show ();
	}
}

void RKProgressControl::newError (const QString &error) {
	RK_TRACE (MISC);

	if (!(mode & IncludeErrorOutput)) return;

	ROutput outputc;
	outputc.type = ROutput::Error;
	outputc.output = error;

	output_log.append (outputc);

	if (mode & RaiseOnError) {
		if (!dialog) createDialog ();
		dialog->raise ();
	}
	if (dialog) dialog->addOutput (&outputc);
}

void RKProgressControl::newOutput (const QString &output) {
	RK_TRACE (MISC);

	if (!(mode & IncludeRegularOutput)) return;

	ROutput outputc;
	outputc.type = ROutput::Output;
	outputc.output = output;

	output_log.append (outputc);

	if (mode & RaiseOnRegularOutput) {
		if (!dialog) createDialog ();
		dialog->raise ();
	}
	if (dialog) dialog->addOutput (&outputc);
}

void RKProgressControl::resetOutput () {
	RK_TRACE (MISC);

	output_log.clear ();
}

void RKProgressControl::addRCommand (RCommand *command, bool done_when_finished) {
	RK_TRACE (MISC);
	RK_ASSERT (command);

	command->addReceiver (this);
	if (done_when_finished) done_command = command;
}

void RKProgressControl::dialogDestroyed () {
	RK_TRACE (MISC);

	dialog = 0;
	if ((!is_done) && (mode & AllowCancel)) {
		is_done = true;
		if (mode & AutoCancelCommands) {
			for (RCommandList::const_iterator it = outstanding_commands.begin (); it != outstanding_commands.end (); ++it) {
				RKGlobals::rInterface ()->cancelCommand (*it);
			}
		}
		emit (cancelled ());
	}
}

void RKProgressControl::done () {
	RK_TRACE (MISC);

	is_done = true;
	if (dialog) {
		dialog->setCloseTextToClose ();
		dialog->done ();
	}

	if ((!modal) && autodelete) {
		if (dialog) disconnect (dialog, SIGNAL (destroyed ()), this, SLOT (dialogDestroyed ()));		// we're already dead
		deleteLater ();
	}
}

void RKProgressControl::createDialog () {
	RK_TRACE (MISC);

	dialog = new RKProgressControlDialog (text, caption, mode, modal);
	connect (dialog, SIGNAL (destroyed ()), this, SLOT (dialogDestroyed ()));
	if (is_done) done ();
	for (Q3ValueList<ROutput>::const_iterator it = output_log.begin (); it != output_log.end (); ++it) {
		dialog->addOutput (&(*it));
	}
}

void RKProgressControl::newOutput (RCommand *, ROutput *output) {
	RK_TRACE (MISC);
	RK_ASSERT (output);

	if (output->type == ROutput::Output) {
		newOutput (output->output);
	} else {
		newError (output->output);
	}
}

void RKProgressControl::rCommandDone (RCommand * command) {
	RK_TRACE (MISC);

	if (command == done_command) done ();
}



//////////////////////////// RKProgressControlDialog ///////////////////////////////////////////7

#include <qlayout.h>
#include <q3textedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <q3vbox.h>

RKProgressControlDialog::RKProgressControlDialog (const QString &text, const QString &caption, int mode_flags, bool modal) : QDialog (0, 0, modal, Qt::WDestructiveClose) {
	RK_TRACE (MISC);

	setCaption (caption);

	Q3VBoxLayout *vbox = new Q3VBoxLayout (this, RKGlobals::marginHint (), RKGlobals::spacingHint ());

	QLabel *label = new QLabel (text, this);
	label->setWordWrap (true);
	vbox->addWidget (label);

	error_indicator = new QLabel (i18n ("<b>There have been errors and / or warnings! See below for a transcript</b>"), this);
	error_indicator->setPaletteForegroundColor (QColor (255, 0, 0));
	error_indicator->hide ();
	vbox->addWidget (error_indicator);

	output_box = new Q3VBox (this);
	vbox->addWidget (output_box);
	if (mode_flags & (RKProgressControl::IncludeErrorOutput | RKProgressControl::IncludeRegularOutput)) {
		QString ocaption;
		if (mode_flags & RKProgressControl::IncludeRegularOutput) {
			show_output_text = i18n ("Show Output");
			hide_output_text = i18n ("Hide Output");
			ocaption = i18n ("Output:");
		} else {
			show_output_text = i18n ("Show Errors / Warnings");
			hide_output_text = i18n ("Hide Errors / Warnings");
			ocaption = i18n ("Errors / Warnings:");
		}
		output_caption = new QLabel (ocaption, output_box);

		output_text = new Q3TextEdit (output_box);
		output_text->setReadOnly (true);
		output_text->setTextFormat (Qt::PlainText);
		output_text->setUndoRedoEnabled (false);

		if (!(mode_flags & RKProgressControl::OutputShownByDefault)) {
			output_box->hide ();
		}
	}

	Q3HBoxLayout *button_layout = new Q3HBoxLayout (0, 0, RKGlobals::spacingHint ());
	vbox->addLayout (button_layout);

	toggle_output_button = new QPushButton (show_output_text, this);
	if (!(mode_flags & RKProgressControl::OutputSwitchable)) toggle_output_button->hide ();
	if (mode_flags & RKProgressControl::OutputShownByDefault) toggle_output_button->setText (hide_output_text);
	connect (toggle_output_button, SIGNAL (clicked ()), this, SLOT (toggleOutputButtonPressed ()));
	button_layout->addWidget (toggle_output_button);
	button_layout->addStretch ();

	close_button = new QPushButton (QString::null, this);
	if (mode_flags & RKProgressControl::AllowCancel) close_button->setText (i18n ("Cancel"));
	else setCloseTextToClose ();
	connect (close_button, SIGNAL (clicked ()), this, SLOT (reject ()));
	button_layout->addWidget (close_button);

	prevent_close = (mode_flags & RKProgressControl::PreventClose);
	is_done = false;
}

RKProgressControlDialog::~RKProgressControlDialog () {
	RK_TRACE (MISC);
}

void RKProgressControlDialog::addOutput (const ROutput *output) {
	RK_TRACE (MISC);

	if (output->type != ROutput::Output) {
		output_text->setColor (Qt::red);
		if (!output_box->isShown ()) toggleOutputButtonPressed ();
		error_indicator->show ();
	}

	output_text->append (output->output);
	output_text->setColor (Qt::black);
}

void RKProgressControlDialog::setCloseTextToClose () {
	RK_TRACE (MISC);

	close_button->setText (i18n ("Done"));
}

void RKProgressControlDialog::toggleOutputButtonPressed () {
	RK_TRACE (MISC);

	if (output_box->isShown ()) {
		output_box->hide ();
		toggle_output_button->setText (show_output_text);
	} else {
		output_box->show ();
		toggle_output_button->setText (hide_output_text);
	}
}

void RKProgressControlDialog::done () {
	RK_TRACE (MISC);

	is_done = true;
	if (!output_box->isShown ()) reject ();
}

void RKProgressControlDialog::closeEvent (QCloseEvent *e) {
	RK_TRACE (DIALOGS);

	if (prevent_close && (!is_done)) {
		e->ignore ();
	} else {
		QDialog::closeEvent (e);
	}
}

#include "rkprogresscontrol.moc"
