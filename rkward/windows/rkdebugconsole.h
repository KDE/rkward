/*
rkdebugconsole - This file is part of the RKWard project. Created: Wed Oct 19 2011
SPDX-FileCopyrightText: 2011 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKDEBUGCONSOLE_H
#define RKDEBUGCONSOLE_H

#include "rkmdiwindow.h"

class QPushButton;
class KHistoryComboBox;
class QTextEdit;
class QLabel;

/** A very simple debugger console */
class RKDebugConsole : public RKMDIWindow {
	Q_OBJECT
public:
	RKDebugConsole (QWidget *parent, bool tool_window, const char *name=0);
	~RKDebugConsole ();

	static RKDebugConsole *instance () { return _instance; };

	// reimplemented to refuse closing while inside the debugger
	bool close (CloseWindowMode ask_save) override;
public slots:
	void newDebugState ();
private slots:
	void sendReplySlot ();
	void stepButtonClicked ();
	void stepOutButtonClicked ();
	void continueButtonClicked ();
	void cancelButtonClicked ();
private:
	void sendReply (const QString &reply);

	QTextEdit* context_view;
	KHistoryComboBox* reply_edit;
	QLabel* prompt_label;

	QPushButton* step_button;
	QPushButton* step_out_button;
	QPushButton* continue_button;
	QPushButton* cancel_button;

friend class RKWardMainWindow;
	static RKDebugConsole *_instance;
};

#endif
