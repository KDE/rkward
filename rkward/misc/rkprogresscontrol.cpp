/***************************************************************************
                          rkprogresscontol  -  description
                             -------------------
    begin                : Sun Sep 10 2006
    copyright            : (C) 2006, 2007, 2008, 2009, 2010, 2011 by Thomas Friedrichsmeier
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

#include "rkprogresscontrol.h"

#include <QHBoxLayout>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>

#include <klocale.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../settings/rksettingsmoduler.h"

#include "../debug.h"

/** This class provides the dialog shown as part of an RKProgressControl. Generally you should not use this class directly, but rather use RKProgressControl. */
class RKProgressControlDialog : public QDialog {
public:
/** constructor. */
	RKProgressControlDialog (const QString &text, const QString &caption, int mode_flags, bool modal);
/** destructor. */
	~RKProgressControlDialog ();
public:
	void addOutput (const ROutput *output);
	void finished ();
protected:
	void closeEvent (QCloseEvent *e) override;
	void scrollDown ();
	void toggleDetails ();
private:
/** Replace "Cancel" button text with "Close" (to be called, when underlying command has finished */
	void setCloseTextToClose ();

	QLabel *error_indicator;
	QTextEdit *output_text;
	QWidget *detailsbox;
	QDialogButtonBox *buttons;

	QString output_button_text;

	ROutput::ROutputType last_output_type;
	bool prevent_close;
	bool is_done;
};


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
		if (dialog) disconnect (dialog, &QObject::destroyed, this, &RKProgressControl::dialogDestroyed);		// we're already dead
		deleteLater ();
	}
}

void RKProgressControl::createDialog () {
	RK_TRACE (MISC);

	dialog = new RKProgressControlDialog (text, caption, mode, modal);
	connect (dialog, &QObject::destroyed, this, &RKProgressControl::dialogDestroyed);
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

QString RKProgressControl::fullCommandOutput() {
	RK_TRACE (MISC);

	QString ret;
	foreach (const ROutput& out, output_log) ret.append (out.output);
	return ret;
}

//////////////////////////// RKProgressControlDialog ///////////////////////////////////////////7

#include <qlayout.h>
#include <QTextEdit>
#include <qlabel.h>
#include <QPushButton>

#include <kstandardguiitem.h>

RKProgressControlDialog::RKProgressControlDialog (const QString &text, const QString &caption, int mode_flags, bool modal) : QDialog (0) {
	RK_TRACE (MISC);

	setAttribute (Qt::WA_DeleteOnClose, true);
	setModal (modal);
	setWindowTitle (caption);

	QVBoxLayout *layout = new QVBoxLayout (this);

	QWidget *mainbox = new QWidget (this);
	QVBoxLayout *mainboxlayout = new QVBoxLayout (mainbox);
	mainbox->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	mainboxlayout->setContentsMargins (0, 0, 0, 0);

	QLabel *label = new QLabel (text, mainbox);
	label->setWordWrap (true);
	mainboxlayout->addWidget (label);

	error_indicator = new QLabel (i18n ("<b>There have been errors and / or warnings. See below for a transcript</b>"), mainbox);
	QPalette palette = error_indicator->palette ();
	palette.setColor (error_indicator->foregroundRole (), QColor (255, 0, 0));
	error_indicator->setPalette (palette);
	error_indicator->hide ();
	mainboxlayout->addWidget (error_indicator);

	detailsbox = new QWidget (this);
	QVBoxLayout *detailsboxlayout = new QVBoxLayout (detailsbox);
	detailsboxlayout->setContentsMargins (0, 0, 0, 0);

	QString ocaption;
	if (mode_flags & RKProgressControl::IncludeRegularOutput) {
		output_button_text = i18n ("Output");
		ocaption = i18n ("Output:");
	} else {
		output_button_text = i18n ("Errors / Warnings");
		ocaption = i18n ("Errors / Warnings:");
	}
	label = new QLabel (ocaption, detailsbox);
	detailsboxlayout->addWidget (label);

	output_text = new QTextEdit (detailsbox);
	output_text->setReadOnly (true);
	output_text->setPlainText (QString ());
	output_text->setUndoRedoEnabled (false);
	output_text->setLineWrapMode (QTextEdit::NoWrap);
	output_text->setMinimumWidth (QFontMetrics (output_text->font ()).averageCharWidth () * RKSettingsModuleR::getDefaultWidth ());
	detailsboxlayout->addWidget (output_text);
	detailsboxlayout->setStretchFactor (output_text, 10);

	buttons = new QDialogButtonBox (QDialogButtonBox::Cancel, this);
	connect (buttons->button (QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);
	if (mode_flags & RKProgressControl::OutputSwitchable) {
		QPushButton* button = buttons->addButton (output_button_text, QDialogButtonBox::HelpRole);
		connect (button, &QPushButton::clicked, this, &RKProgressControlDialog::toggleDetails);
	}
	if (mode_flags & RKProgressControl::AllowCancel) buttons->button (QDialogButtonBox::Cancel)->setText (i18n ("Cancel"));
	else (setCloseTextToClose ());

	detailsbox->setVisible (mode_flags & RKProgressControl::OutputShownByDefault);

	layout->addWidget (mainbox);
	layout->addWidget (detailsbox);
	layout->addWidget (buttons);

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
	if (detailsbox->isVisible ()) {
		QScrollBar *bar = output_text->verticalScrollBar ();
		if (bar && (bar->value () < bar->maximum ())) at_end = false;
	}

	if (output->type != last_output_type) {
		last_output_type = output->type;
		output_text->insertPlainText ("\n");

		if (output->type == ROutput::Output) {
			output_text->setTextColor (Qt::black);
		} else {
			output_text->setTextColor (Qt::red);
			if (!detailsbox->isVisible ()) toggleDetails ();
			error_indicator->show ();
		}
	}

	output_text->insertPlainText (output->output);

	// if previously at end, auto-scroll
	if (at_end && output_text->isVisible ()) scrollDown ();
}

void RKProgressControlDialog::toggleDetails () {
	RK_TRACE (MISC);

	int new_height = height ();
	if (detailsbox->isVisible ()) {
		new_height -= detailsbox->height ();
		detailsbox->hide ();
	} else {
		int h = detailsbox->height ();
		if (h <= 0) h = detailsbox->sizeHint ().height ();
		new_height += h;
		detailsbox->show ();
		scrollDown ();
	}

	layout ()->activate ();
	resize (width (), new_height);
}

void RKProgressControlDialog::scrollDown () {
	RK_TRACE (MISC);

	QScrollBar *bar = output_text->verticalScrollBar ();
	if (bar) bar->setValue (bar->maximum ());
}

void RKProgressControlDialog::setCloseTextToClose () {
	RK_TRACE (MISC);

	buttons->removeButton (buttons->button (QDialogButtonBox::Cancel));
	QPushButton *done_button = buttons->addButton (QDialogButtonBox::Ok);
	done_button->setText (i18n ("Done"));
	connect (done_button, &QPushButton::clicked, this, &QDialog::reject);
}

void RKProgressControlDialog::finished () {
	RK_TRACE (MISC);

	is_done = true;
	setCloseTextToClose ();
	if (!detailsbox->isVisible ()) reject ();
}

void RKProgressControlDialog::closeEvent (QCloseEvent *e) {
	RK_TRACE (DIALOGS);

	if (prevent_close && (!is_done)) {
		e->ignore ();
	} else {
		QDialog::closeEvent (e);
	}
}

