/***************************************************************************
                          rkmdiwindow  -  description
                             -------------------
    begin                : Tue Sep 26 2006
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

#include "rkmdiwindow.h"

#include <qapplication.h>

#include <kparts/event.h>

#include "rkworkplace.h"
#include "rkworkplaceview.h"

#include "../debug.h"

RKMDIWindow::RKMDIWindow (QWidget *parent, Type type, bool tool_window, char *name) : QWidget (parent, name) {
	RK_TRACE (APP);

	RKMDIWindow::type = type;
	if (tool_window) state = ToolWindow;
	else state = Attached;
	wrapper = 0;
}

RKMDIWindow::~RKMDIWindow () {
	RK_TRACE (APP);
}

//virtual
QString RKMDIWindow::fullCaption () {
	RK_TRACE (APP);
	return shortCaption ();
}

//virtual
QString RKMDIWindow::shortCaption () {
	RK_TRACE (APP);
	return caption ();
}

// virtual from QWidget
void RKMDIWindow::setCaption (const QString &caption) {
	RK_TRACE (APP);
	QWidget::setCaption (caption);
	emit (captionChanged (this));
}

void RKMDIWindow::activate (bool with_focus) {
	RK_TRACE (APP);

	// WORKAROUND for KMDI: it will always grab focus, so we need to make sure to release it again, if needed
	QWidget *old_focus = qApp->focusWidget ();

	if (isToolWindow ()) {
		RK_ASSERT (wrapper);
		wrapper->wrapperWidget ()->topLevelWidget ()->show ();
		wrapper->wrapperWidget ()->topLevelWidget ()->raise ();
		wrapper->show ();
	} else {
		if (isAttached ()) {
			RKWorkplace::mainWorkplace ()->view ()->setActivePage (this);
		} else {
			topLevelWidget ()->show ();
			topLevelWidget ()->raise ();
		}
	}

	if (with_focus) setFocus ();
	else {
		if (old_focus) old_focus->setFocus ();
	}
}

bool RKMDIWindow::close (bool also_delete) {
	RK_TRACE (APP);

	if (isToolWindow ()) {
		RK_ASSERT (wrapper);
		wrapper->hide ();
		return true;
	}

	return QWidget::close (also_delete);
}

void RKMDIWindow::prepareToBeAttached () {
	RK_TRACE (APP);

	RK_ASSERT (!isToolWindow ());
}

void RKMDIWindow::prepareToBeDetached () {
	RK_TRACE (APP);

	RK_ASSERT (!isToolWindow ());
}

bool RKMDIWindow::eventFilter (QObject *watched, QEvent *e) {
	RK_TRACE (APP);

#warning TODO
// TODO: use partmanager's activePartChanged in rkward.cpp instead. There will have to be special handling in DetachedWindowContainer, anyway
	RK_ASSERT (watched == getPart ());
	if (KParts::PartActivateEvent::test (e)) {
		KParts::PartActivateEvent *ev = static_cast<KParts::PartActivateEvent *> (e);
		if (ev->activated ()) {
			qDebug ("activated: %s", getDescription ().latin1 ());
		} else {
			qDebug ("deactivated: %s", getDescription ().latin1 ());
		}
	}
	return FALSE;
}

void RKMDIWindow::initializeActivationSignals () {
	RK_TRACE (APP);

	RK_ASSERT (getPart ());
	getPart ()->installEventFilter (this);
}

#include "rkmdiwindow.moc"
