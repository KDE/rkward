/*
rkcallstackviewer - This file is part of RKWard (https://rkward.kde.org). Created: Wed Oct 19 2011
SPDX-FileCopyrightText: 2011 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcallstackviewer.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextDocument>

#include "../misc/rkdummypart.h"
#include "../misc/rkcommonfunctions.h"
#include "../agents/rkdebughandler.h"
#include "rkcommandeditorwindow.h"

#include "../debug.h"

RKCallstackViewer* RKCallstackViewer::_instance = nullptr;

RKCallstackViewer::RKCallstackViewer (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, RKMDIWindow::CallstackViewerWindow, tool_window, name) {
	RK_TRACE (APP);

	real_widget = nullptr;

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	layout_widget = new QWidget (this);
	layout->addWidget (layout_widget);
	layout_widget->setFocusPolicy (Qt::StrongFocus);

	setPart (new RKDummyPart (this, layout_widget));
	initializeActivationSignals ();

	connect (RKDebugHandler::instance (), &RKDebugHandler::newDebugState, this, &RKCallstackViewer::newDebugState);
}

RKCallstackViewer::~RKCallstackViewer () {
	RK_TRACE (APP);
}

void RKCallstackViewer::showEvent (QShowEvent *e) {
	RK_TRACE (APP);

	createRealWidget ();
	RKMDIWindow::showEvent (e);
}

void RKCallstackViewer::createRealWidget () {
	RK_TRACE (APP);

	if (!real_widget) {
		RK_DEBUG (APP, DL_INFO, "creating callstack viewer");

		real_widget = new RKCallstackViewerWidget (layout_widget);
		QVBoxLayout *l = new QVBoxLayout (layout_widget);
		l->setContentsMargins (0, 0, 0, 0);
		layout ()->addWidget (real_widget);
		setFocusProxy (real_widget);
	}
}

void RKCallstackViewer::newDebugState () {
	RK_TRACE (APP);

	if (!real_widget) createRealWidget ();
	else real_widget->updateState ();
	if (RKDebugHandler::instance ()->state () == RKDebugHandler::InDebugPrompt) activate ();
}



RKCallstackViewerWidget::RKCallstackViewerWidget (QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);

	QHBoxLayout *h_layout = new QHBoxLayout (this);
	h_layout->setContentsMargins (0, 0, 0, 0);

	QVBoxLayout *v_layout = new QVBoxLayout ();
	h_layout->addLayout (v_layout);
	h_layout->setStretchFactor (v_layout, 1);

	QLabel *label = new QLabel (i18n ("<b>Active calls</b>"), this);
	v_layout->addWidget (label);
	frame_selector = new QListWidget (this);
	frame_selector->setSelectionMode (QAbstractItemView::SingleSelection);
	connect (frame_selector, &QListWidget::currentRowChanged, this, &RKCallstackViewerWidget::frameChanged);
	v_layout->addWidget (frame_selector);

	v_layout = new QVBoxLayout ();
	h_layout->addLayout (v_layout);
	h_layout->setStretchFactor (v_layout, 2);

	frame_info = RKCommonFunctions::wordWrappedLabel (QString ());
	v_layout->addWidget (frame_info);

	frame_source = new RKCommandEditorWindow (this, QUrl (), QString (), RKCommandEditorFlags::DefaultToRHighlighting | RKCommandEditorFlags::ReadOnly);
	v_layout->addWidget (frame_source);

	updateState ();
}

RKCallstackViewerWidget::~RKCallstackViewerWidget () {
	RK_TRACE (APP);
}

void RKCallstackViewerWidget::updateState () {
	RK_TRACE (APP);

	if (RKDebugHandler::instance ()->state () == RKDebugHandler::NotInDebugger) {
		QString info = i18n ("Not in a debugger context");
		frame_source->setText (info);
		frame_selector->clear ();
		frame_info->setText ("<b>" + info + "</b>");
	} else if (RKDebugHandler::instance ()->state () == RKDebugHandler::InDebugPrompt) {
		frame_selector->clear ();
		frame_selector->setEnabled (true);
		frame_selector->insertItems (0, RKDebugHandler::instance ()->calls ());
		frame_selector->setCurrentRow (frame_selector->count () - 1);
	} else {
		frame_selector->setEnabled (false);
	}
}

void RKCallstackViewerWidget::frameChanged (int frame_number) {
	RK_TRACE (APP);

	if (RKDebugHandler::instance ()->state () == RKDebugHandler::NotInDebugger) return;

	frame_info->setText (i18n ("<b>Current call:</b> %1<br><b>Environment:</b> %2<br><b>Local objects:</b> %3",
									RKDebugHandler::instance ()->calls ().value (frame_number).toHtmlEscaped (),
									RKDebugHandler::instance ()->environments ().value (frame_number).toHtmlEscaped (),
									RKDebugHandler::instance ()->locals ().value (frame_number).split ('\n').join (", ").toHtmlEscaped ()));
	frame_source->setText (RKDebugHandler::instance ()->functions ().value (frame_number) + '\n');
	int line = RKDebugHandler::instance ()->relativeSourceLines ().value (frame_number, 0);
	if (line > 0) frame_source->highlightLine (line - 1);
	else if (frame_number < RKDebugHandler::instance ()->calls ().size () - 1) {
		// no (valid) source reference available? Make an effort to locate (candidate line(s) for) the call
		QStringList lines = RKDebugHandler::instance ()->functions ().value (frame_number).split ('\n');
		QString call = RKDebugHandler::instance ()->calls ().value (frame_number + 1);
		call = call.left (call.indexOf ('(')).trimmed ();
		QRegularExpression call_exp (QRegularExpression::escape (call) + "\\s*\\(");
		for (int i = lines.size () - 1; i >= 0; --i) {
			if (lines.at (i).contains (call_exp)) frame_source->highlightLine (i);
		}
	}
}

