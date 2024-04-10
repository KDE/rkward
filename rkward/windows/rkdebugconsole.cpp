/*
rkdebugconsole - This file is part of RKWard (https://rkward.kde.org). Created: Wed Oct 19 2011
SPDX-FileCopyrightText: 2011-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkdebugconsole.h"

#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <KLocalizedString>
#include <kmessagebox.h>
#include <khistorycombobox.h>

#include "../agents/rkdebughandler.h"
#include "../misc/rkdummypart.h"
#include "../misc/rkcommonfunctions.h"
#include "../rkconsole.h"

#include "../debug.h"

RKDebugConsole* RKDebugConsole::_instance = nullptr;

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
	button_layout->addWidget(addButton("n\n", i18n("Next (step over)"), i18n("Evaluate the next statement, stepping over function calls.")));
	button_layout->addWidget(addButton("s\n", i18n("Next (step into)"), i18n("Evaluate the next statement, stepping into function calls.")));
	button_layout->addWidget(addButton("browserSetDebug(1)\ncont\n", i18n("Step out"), i18n(
	        "<p>Continue until the caller of this function is reached (unless another debug statement is hit, earlier)</p>"
	        "<p><b>Note:</b> In some cases, the calling function will never be reached, because the call was the last step in the caller. "
	        "In these cases, the behavior is identical to 'Continue'.</p>")
	));
	button_layout->addWidget(addButton("f\n", i18n("Finish current"), i18n("Finish current loop or function.")));
	button_layout->addWidget(addButton("cont\n", i18n("Continue"), i18n("Continue evaluation.")));
	button_layout->addStretch ();
	button_layout->addWidget(addButton("Q\n", i18n("Cancel"), i18n("Exit debugger and return to top-level statement.")));
	button_layout->addStretch ();

	QHBoxLayout *lower_layout = new QHBoxLayout ();
	main_layout->addLayout (lower_layout);

	prompt_label = new QLabel (this);
	lower_layout->addWidget (prompt_label);
	reply_edit = new KHistoryComboBox (this);
	reply_edit->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	connect (reply_edit, static_cast<void (KHistoryComboBox::*)(const QString&)>(&KHistoryComboBox::returnPressed), this, &RKDebugConsole::sendReplySlot);
	lower_layout->addWidget (reply_edit);
	setFocusProxy (reply_edit);

	setFocusPolicy (Qt::StrongFocus);

	setPart (new RKDummyPart (this, this));
	initializeActivationSignals ();

	connect (RKDebugHandler::instance (), &RKDebugHandler::newDebugState, this, &RKDebugConsole::newDebugState);
	newDebugState ();
}

RKDebugConsole::~RKDebugConsole () {
	RK_TRACE (APP);
}

QPushButton* RKDebugConsole::addButton(const QString& command, const QString& text, const QString& tip) {
	QPushButton *button = new QPushButton(text);
	connect(button, &QPushButton::clicked, this, [this, command]() { this->sendReply(command); } );
	buttons.append(button);
	RKCommonFunctions::setTips(tip, button);
	return button;
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
		QStringList ch = RKConsole::mainConsole ()->commandHistory ();
		QStringList ch_rev;	// limit to 100 items (dropdown list!), and reverse to have most recent item first.
		for (int i = ch.size () - 1; i >= qMax (0, ch.size () - 101); --i) {
			ch_rev.append (ch[i]);
		}
		reply_edit->setMaxCount (ch_rev.size ());
		reply_edit->setHistoryItems (ch_rev);
		activate (true);
	}

	setEnabled(true);
	for (int i = 0; i < buttons.size(); ++i) buttons[i]->setEnabled(enable);
}

void RKDebugConsole::sendReplySlot () {
	RK_TRACE (APP);

	QString reply = reply_edit->currentText ();
	sendReply (reply);
	RKConsole::mainConsole ()->addCommandToHistory (reply);
	reply_edit->clear ();
}

void RKDebugConsole::sendReply (const QString &reply) {
	RK_TRACE (APP);

	RKDebugHandler::instance ()->submitDebugString (reply);
}

bool RKDebugConsole::close (CloseWindowMode ask_save) {
	RK_TRACE (APP);

	if (RKDebugHandler::instance ()->state () != RKDebugHandler::NotInDebugger) {
		KMessageBox::error(this, i18n("This window cannot be closed, while a debugger is active. If you have no idea what this means, and you want to get out, press the 'Cancel' button on the right hand side of this window."));
		return false;
	}
	return RKMDIWindow::close (ask_save);
}

