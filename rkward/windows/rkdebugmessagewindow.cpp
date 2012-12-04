/***************************************************************************
                          rkdebugmessagewindow  -  description
                             -------------------
    begin                : Sat Dec 01 2012
    copyright            : (C) 2012 by Thomas Friedrichsmeier
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

#include "rkdebugmessagewindow.h"

#include <QShowEvent>
#include <QHideEvent>
#include <QVBoxLayout>
#include <QTextEdit>

#include <kvbox.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "../misc/rkdummypart.h"

#include "../debug.h"

RKDebugMessageWindow* RKDebugMessageWindow::_instance = 0;

RKDebugMessageWindow::RKDebugMessageWindow (QWidget* parent, bool tool_window, const char* name) : RKMDIWindow (parent, RKMDIWindow::DebugMessageWindow, tool_window, name) {
	RK_TRACE (APP);
	RK_ASSERT (!_instance);
	real_widget = 0;
	first = true;

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	layout_widget = new KVBox (this);
	layout->addWidget (layout_widget);
	layout_widget->setFocusPolicy (Qt::StrongFocus);

	setPart (new RKDummyPart (this, layout_widget));
	initializeActivationSignals ();
}

RKDebugMessageWindow::~RKDebugMessageWindow () {
	RK_TRACE (APP);
}

void RKDebugMessageWindow::showEvent (QShowEvent *e) {
	RK_TRACE (APP);

	if (!e->spontaneous ()) createWidget ();
	RKMDIWindow::showEvent (e);
}

void RKDebugMessageWindow::hideEvent (QHideEvent *e) {
	RK_TRACE (APP);

	if (!e->spontaneous ()) discardWidget ();
	RKMDIWindow::hideEvent (e);
}

void RKDebugMessageWindow::createWidget () {
	RK_TRACE (APP);

	if (!real_widget) {
		RK_DEBUG (APP, DL_INFO, "creating debug message viewer");
		real_widget = new RKDebugMessageWindowWidget (layout_widget);
		setFocusProxy (real_widget);

		if (first) {
			KMessageBox::information (this, i18n ("<p>This window is used for displaying RKWard related debug messages. It is targetted primarily at (plugin) developers. It does <b>not</b> offer any features for debugging R code.</p>"
				"<p>Note that the list of message is cleared everytime you close the window.</p><p>Type and severity level of messages can be controlled from Settings->Configure RKWard->Debug</p>"), i18n ("About this window"), "inforkdebugmessagewindow");
			first = false;
		}
	}
}

void RKDebugMessageWindow::discardWidget () {
	RK_TRACE (APP);

	if (real_widget) {
		RK_DEBUG (APP, DL_INFO, "discarding debug message viewer");
		delete real_widget;
		real_widget = 0;
	}
}

void RKDebugMessageWindow::newMessage (const int flags, const int level, const QString &message) {
	// Not tracing this! That might lead to infinite recursion!
	if (_instance && _instance->real_widget) _instance->real_widget->newMessage (flags, level, message);
}



RKDebugMessageWindowWidget::RKDebugMessageWindowWidget (QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);

	QVBoxLayout *v_layout = new QVBoxLayout (this);
	v_layout->setContentsMargins (0, 0, 0, 0);
	message_viewer = new QTextEdit (this);
	message_viewer->setUndoRedoEnabled (false);
	message_viewer->setReadOnly (true);
	message_viewer->setTextBackgroundColor (Qt::white);
	v_layout->addWidget (message_viewer);
}

RKDebugMessageWindowWidget::~RKDebugMessageWindowWidget () {
	RK_TRACE (APP);
}

void RKDebugMessageWindowWidget::newMessage (const int flags, const int level, const QString &message) {
	Q_UNUSED (flags);

	// Not tracing this! That might lead to infinite recursion!
	if (level == DL_TRACE) {
		message_viewer->setTextColor (Qt::gray);
		message_viewer->insertPlainText ("TRACE\t");
	} else if (level == DL_DEBUG) {
		message_viewer->setTextColor (Qt::blue);
		message_viewer->insertPlainText ("DEBUG\t");
	} else if (level == DL_INFO) {
		message_viewer->setTextColor (Qt::green);
		message_viewer->insertPlainText ("INFO\t");
	} else if (level == DL_WARNING) {
		message_viewer->setTextColor (Qt::darkYellow);
		message_viewer->insertPlainText ("WARNING\t");
	} else {
		message_viewer->setTextColor (Qt::red);
		message_viewer->insertPlainText ("ERROR\t");
	}
	message_viewer->setTextColor (Qt::black);

	message_viewer->insertPlainText (message + "\n");
}

