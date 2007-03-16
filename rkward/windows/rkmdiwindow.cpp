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
#include <qpainter.h>

#include <kparts/event.h>

#include "rkworkplace.h"
#include "rkworkplaceview.h"

#include "../debug.h"

RKMDIWindow::RKMDIWindow (QWidget *parent, Type type, bool tool_window, char *name) : QFrame (parent, name) {
	RK_TRACE (APP);

	RKMDIWindow::type = type;
	if (tool_window) state = ToolWindow;
	else state = Attached;
	wrapper = 0;
	part = 0;
	active = false;
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
		wrapper->show ();
		wrapper->wrapperWidget ()->topLevelWidget ()->show ();
		wrapper->wrapperWidget ()->topLevelWidget ()->raise ();
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
	// WARNING: The derived object and the part may both the destroyed at this point of time!
	// Make sure not to call any virtual function on this object!

	RK_ASSERT (watched == getPart ());
	if (KParts::PartActivateEvent::test (e)) {
		RK_TRACE (APP);		// trace only the "interesting" calls to this function

		KParts::PartActivateEvent *ev = static_cast<KParts::PartActivateEvent *> (e);
		if (ev->activated ()) {
			emit (windowActivated (this));
			setFocus ();		// designed to aid the kate part
			active = true;
		} else {
			clearFocus ();		// designed to aid the kate part
			active = false;
		}
		if (layout()->margin () < 1) {
			layout()->setMargin (1);
		}
		update ();
	}
	return FALSE;
}

void RKMDIWindow::initializeActivationSignals () {
	RK_TRACE (APP);

	RK_ASSERT (getPart ());
	getPart ()->installEventFilter (this);
}

void RKMDIWindow::paintEvent (QPaintEvent *e) {
	// RK_TRACE (APP); Do not trace!

	QFrame::paintEvent (e);

	if (isActive ()) {
		QPainter paint (this);
		paint.setPen (QColor (255, 0, 0));
		paint.drawLine (0, 0, 0, height ()-1);
		paint.drawLine (0, height ()-1, width ()-1, height ()-1);
		paint.drawLine (0, 0, width ()-1, 0);
		paint.drawLine (width ()-1, 0, width ()-1, height ()-1);
	}
}

#include "rkmdiwindow.moc"
