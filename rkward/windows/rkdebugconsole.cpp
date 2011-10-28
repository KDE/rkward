/***************************************************************************
                          rkdebugconsole  -  description
                             -------------------
    begin                : Wed Oct 19 2011
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

#include "rkdebugconsole.h"

#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <klocale.h>
#include <kmessagebox.h>

#include "../agents/rkdebughandler.h"
#include "../misc/rkdummypart.h"
#include "../misc/rkcommonfunctions.h"

#include "../debug.h"

RKDebugConsole* RKDebugConsole::_instance = 0;

RKDebugConsole::RKDebugConsole (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, DebugConsoleWindow, tool_window, name) {
	RK_TRACE (APP);

	QVBoxLayout *main_layout = new QVBoxLayout (this);
	main_layout->setContentsMargins (0, 0, 0, 0);
	QHBoxLayout *upper_layout = new QHBoxLayout ();
	main_layout->addLayout (upper_layout);

	context_view = new QTextEdit (this);
	context_view->setReadOnly (true);
	context_view->setAcceptRichText (false);
	upper_layout->addWidget (context_view);

	QVBoxLayout *button_layout = new QVBoxLayout ();
	upper_layout->addLayout (button_layout);
	step_button = new QPushButton (i18n ("Next"), this);
	connect (step_button, SIGNAL (clicked()), this, SLOT (stepButtonClicked()));
	button_layout->addWidget (step_button);
	step_out_button = new QPushButton (i18n ("Step out"), this);
	connect (step_out_button, SIGNAL (clicked()), this, SLOT (stepOutButtonClicked()));
	RKCommonFunctions::setTips (i18n ("<p>Continue until the caller of this function is reached (unless another debug statement is hit, earlier)</p>"
	                                  "<p><b>Note:</b> In some cases, the calling function will never be reached, because the call was the last step in the caller."
	                                  "In these cases, the behavior is identical to 'Continue'.</p>"), step_out_button);
	button_layout->addWidget (step_out_button);
	continue_button = new QPushButton (i18n ("Continue"), this);
	connect (continue_button, SIGNAL (clicked()), this, SLOT (continueButtonClicked()));
	button_layout->addWidget (continue_button);
	cancel_button = new QPushButton (i18n ("Cancel"), this);
	connect (cancel_button, SIGNAL (clicked()), this, SLOT (cancelButtonClicked()));
	button_layout->addWidget (cancel_button);
	button_layout->addStretch ();

	QHBoxLayout *lower_layout = new QHBoxLayout ();
	main_layout->addLayout (lower_layout);

	prompt_label = new QLabel (this);
	lower_layout->addWidget (prompt_label);
	reply_edit = new QLineEdit (this);
	connect (reply_edit, SIGNAL (returnPressed()), this, SLOT (sendReply()));
	lower_layout->addWidget (reply_edit);
	setFocusProxy (reply_edit);

	setFocusPolicy (Qt::StrongFocus);

	setPart (new RKDummyPart (this, this));
	initializeActivationSignals ();

	connect (RKDebugHandler::instance (), SIGNAL (newDebugState()), this, SLOT (newDebugState()));
	newDebugState ();
}

RKDebugConsole::~RKDebugConsole () {
	RK_TRACE (APP);
}

void RKDebugConsole::newDebugState () {
	RK_TRACE (APP);

	bool enable = true;
	if (RKDebugHandler::instance ()->state () == RKDebugHandler::NotInDebugger) {
		context_view->setPlainText (i18n ("Not in a debugger context"));
		setEnabled (false);
		return;
	} else if (RKDebugHandler::instance ()->state () == RKDebugHandler::InDebugRun) {
		enable = false;
		reply_edit->setEnabled (false);
	} else {
		context_view->setPlainText (RKDebugHandler::instance ()->outputContext ());
		prompt_label->setText (RKDebugHandler::instance ()->debugPrompt ());
		reply_edit->setEnabled (true);	// must come before focus
		activate (true);
	}

	setEnabled (true);
	step_button->setEnabled (enable);
	step_out_button->setEnabled (enable);
	continue_button->setEnabled (enable);
	cancel_button->setEnabled (enable);
}

void RKDebugConsole::sendReply () {
	RK_TRACE (APP);

	sendReply (reply_edit->text ());
	reply_edit->clear ();
}

void RKDebugConsole::stepOutButtonClicked () {
	RK_TRACE (APP);

	sendReply ("browserSetDebug(1)\ncont\n");
}

void RKDebugConsole::stepButtonClicked () {
	RK_TRACE (APP);

	sendReply ("n\n");
}

void RKDebugConsole::continueButtonClicked () {
	RK_TRACE (APP);

	sendReply ("cont\n");
}

void RKDebugConsole::cancelButtonClicked () {
	RK_TRACE (APP);

	RKDebugHandler::instance ()->sendCancel ();
}

void RKDebugConsole::sendReply (const QString &reply) {
	RK_TRACE (APP);

	RKDebugHandler::instance ()->submitDebugString (reply);
}

bool RKDebugConsole::close (bool also_delete) {
	RK_TRACE (APP);

	if (RKDebugHandler::instance ()->state () != RKDebugHandler::NotInDebugger) {
		KMessageBox::sorry (this, i18n ("This window cannot be closed, while a debugger is active. If you have no idea what this means, and you want to get out, press the 'Cancel' button on the right hand side of this window."));
		return false;
	}
	return RKMDIWindow::close (also_delete);
}

#include "rkdebugconsole.moc"
