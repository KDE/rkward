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

#ifndef RKDEBUGCONSOLE_H
#define RKDEBUGCONSOLE_H

#include "rkmdiwindow.h"

class QPushButton;
class QLineEdit;
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
	bool close (bool auto_delete);
public slots:
	void newDebugState ();
private slots:
	void sendReply ();
	void stepButtonClicked ();
	void continueButtonClicked ();
	void cancelButtonClicked ();
private:
	void sendReply (const QString &reply);

	QTextEdit* context_view;
	QLineEdit* reply_edit;
	QLabel* prompt_label;

	QPushButton* step_button;
	QPushButton* continue_button;
	QPushButton* cancel_button;

friend class RKWardMainWindow;
	static RKDebugConsole *_instance;
};

#endif
