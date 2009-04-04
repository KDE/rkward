/***************************************************************************
                          rkprogresscontol  -  description
                             -------------------
    begin                : Sun Sep 10 2006
    copyright            : (C) 2006, 2007, 2008, 2009 by Thomas Friedrichsmeier
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

#include <QHBoxLayout>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QTimer>

#include <klocale.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../settings/rksettingsmoduler.h"

#include "../debug.h"

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
		dialog->finished ();
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
	for (int i = 0; i < output_log.count (); ++i) {
		dialog->addOutput (&(output_log[i]));
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
#include <QTextEdit>
#include <qlabel.h>

#include <kvbox.h>
#include <kstandardguiitem.h>

RKProgressControlDialog::RKProgressControlDialog (const QString &text, const QString &caption, int mode_flags, bool modal) : KDialog (0) {
	RK_TRACE (MISC);

	setAttribute (Qt::WA_DeleteOnClose, true);
	setModal (modal);
	setCaption (caption);

	KVBox *vbox = new KVBox (this);
	vbox->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	setMainWidget (vbox);

	QLabel *label = new QLabel (text, vbox);
	label->setWordWrap (true);

	error_indicator = new QLabel (i18n ("<b>There have been errors and / or warnings! See below for a transcript</b>"), vbox);
	QPalette palette = error_indicator->palette ();
	palette.setColor (error_indicator->foregroundRole (), QColor (255, 0, 0));
	error_indicator->setPalette (palette);
	error_indicator->hide ();

	KVBox* output_box = new KVBox ();
	if (mode_flags & (RKProgressControl::IncludeErrorOutput | RKProgressControl::IncludeRegularOutput)) {
		QString ocaption;
		if (mode_flags & RKProgressControl::IncludeRegularOutput) {
			output_button_text = i18n ("Output");
			ocaption = i18n ("Output:");
		} else {
			output_button_text = i18n ("Errors / Warnings");
			ocaption = i18n ("Errors / Warnings:");
		}
		new QLabel (ocaption, output_box);

		output_text = new QTextEdit (output_box);
		output_text->setReadOnly (true);
		output_text->setPlainText (QString ());
		output_text->setUndoRedoEnabled (false);
		output_text->setLineWrapMode (QTextEdit::NoWrap);
		output_text->setMinimumWidth (QFontMetrics (output_text->font ()).averageCharWidth () * RKSettingsModuleR::getDefaultWidth ());
		output_box->setStretchFactor (output_text, 10);

		if (!(mode_flags & RKProgressControl::OutputShownByDefault)) {
			output_box->hide ();
		}
	}
	setDetailsWidget (output_box);
	connect (this, SIGNAL(aboutToShowDetails()), this, SLOT(scrollDown()));

	KDialog::ButtonCodes button_codes = KDialog::Cancel;
	if (mode_flags & RKProgressControl::OutputSwitchable) button_codes |= KDialog::Details;
	setButtons (button_codes);
	setButtonText (KDialog::Details, output_button_text);
	if (mode_flags & RKProgressControl::AllowCancel) setButtonText (KDialog::Cancel, i18n ("Cancel"));
	else (setCloseTextToClose ());

	setDetailsWidgetVisible (mode_flags & RKProgressControl::OutputShownByDefault);

	prevent_close = (mode_flags & RKProgressControl::PreventClose);

	last_output_type = ROutput::Output;
	is_done = false;
}

RKProgressControlDialog::~RKProgressControlDialog () {
	RK_TRACE (MISC);
}

void RKProgressControlDialog::addOutput (const ROutput *output) {
	RK_TRACE (MISC);

	// scrolled all the way to the bottom?
	bool at_end = true;
	QScrollBar *bar = output_text->verticalScrollBar ();
	if (bar && (bar->value () < bar->maximum ())) at_end = false;

	if (output->type != last_output_type) {
		output_text->insertPlainText ("\n");

		if (output->type == ROutput::Output) {
			output_text->setTextColor (Qt::black);
		} else {
			output_text->setTextColor (Qt::red);
			if (!isDetailsWidgetVisible ()) setDetailsWidgetVisible (true);
			error_indicator->show ();
		}
	}

	output_text->insertPlainText (output->output);

	// if previously at end, auto-scroll
	if (at_end && output_text->isVisible ()) scrollDown ();
}

void RKProgressControlDialog::scrollDown () {
	RK_TRACE (MISC);

	// oh what an ugly hack... (to cope with changing slider position just when the details widget becomes visible
	if (!output_text->isVisible ()) QTimer::singleShot (0, this, SLOT(scrollDown()));

	QScrollBar *bar = output_text->verticalScrollBar ();
	if (bar) bar->setValue (bar->maximum ());
}

void RKProgressControlDialog::setCloseTextToClose () {
	RK_TRACE (MISC);

	setButtonGuiItem (KDialog::Cancel, KStandardGuiItem::ok ());
	setButtonText (KDialog::Cancel, i18n ("Done"));
}

void RKProgressControlDialog::finished () {
	RK_TRACE (MISC);

	is_done = true;
	setCloseTextToClose ();
	if (!isDetailsWidgetVisible ()) reject ();
}

void RKProgressControlDialog::closeEvent (QCloseEvent *e) {
	RK_TRACE (DIALOGS);

	if (prevent_close && (!is_done)) {
		e->ignore ();
	} else {
		KDialog::closeEvent (e);
	}
}

#include "rkprogresscontrol.moc"
