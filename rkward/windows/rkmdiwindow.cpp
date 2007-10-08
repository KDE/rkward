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
#include <qtimer.h>
//Added by qt3to4:
#include <QEvent>
#include <Q3Frame>
#include <QPaintEvent>

#include <kparts/event.h>

#include "rkworkplace.h"
#include "rkworkplaceview.h"

#include "../debug.h"

RKMDIWindow::RKMDIWindow (QWidget *parent, int type, bool tool_window, const char *name) : Q3Frame (parent, name) {
	RK_TRACE (APP);

	if (tool_window) {
		type |= ToolWindow;
	} else {
		type |= DocumentWindow;
	}
	RKMDIWindow::type = type;
	state = Attached;
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

bool RKMDIWindow::isActive () {
	// don't trace, called pretty often

	if (!topLevelWidget ()->isActiveWindow ()) return false;
	return (active || (!isAttached ()));
}

void RKMDIWindow::activate (bool with_focus) {
	RK_TRACE (APP);

	// WORKAROUND for KMDI: it will always grab focus, so we need to make sure to release it again, if needed
	QWidget *old_focus = qApp->focusWidget ();

	if (isAttached ()) {
		if (isToolWindow ()) {
			RK_ASSERT (wrapper);
			wrapper->show ();
			wrapper->wrapperWidget ()->topLevelWidget ()->show ();
			wrapper->wrapperWidget ()->topLevelWidget ()->raise ();
		} else {
			RKWorkplace::mainWorkplace ()->view ()->setActivePage (this);
		}
	} else {
		topLevelWidget ()->show ();
		topLevelWidget ()->raise ();
	}

	if (with_focus) {
		topLevelWidget ()->setActiveWindow ();
		setFocus();
	} else {
		if (old_focus) old_focus->setFocus ();
	}
}

bool RKMDIWindow::close (bool also_delete) {
	RK_TRACE (APP);

	if (isToolWindow ()) {
		if (!isAttached ()) {
			topLevelWidget ()->deleteLater ();
			// flee the dying DetachedWindowContainer
			RKWorkplace::mainWorkplace ()->attachWindow (this);
		}

		RK_ASSERT (wrapper);
		wrapper->hide ();
		return true;
	}

	return QWidget::close (also_delete);
}

void RKMDIWindow::prepareToBeAttached () {
	RK_TRACE (APP);

	if (isToolWindow ()) {
		static_cast<KDockWidget *>(wrapper->wrapperWidget ())->setWidget (this);
		wrapper->show ();
	}
}

void RKMDIWindow::prepareToBeDetached () {
	RK_TRACE (APP);

	if (isToolWindow ()) {
		wrapper->hide ();
	}
}

bool RKMDIWindow::eventFilter (QObject *watched, QEvent *e) {
	// WARNING: The derived object and the part may both the destroyed at this point of time!
	// Make sure not to call any virtual function on this object!
	RK_ASSERT (acceptsEventsFor (watched));

	if (watched == getPart ()) {
		if (KParts::PartActivateEvent::test (e)) {
			RK_TRACE (APP);		// trace only the "interesting" calls to this function
	
			KParts::PartActivateEvent *ev = static_cast<KParts::PartActivateEvent *> (e);
			if (ev->activated ()) {
				emit (windowActivated (this));
				setFocus ();		// focus doesn't always work correctly for the kate part
				active = true;
			} else {
				active = false;
			}
			if (layout()->margin () < 1) {
				layout()->setMargin (1);
			}
			update ();
		}
	} else {
		RK_ASSERT (isToolWindow ());
		if (watched == wrapper->wrapperWidget ()) {
			// don't show the wrapper if detached. Instead, show this window.
			if (e->type () == QEvent::Show) {
				if (!isAttached ()) {
					// calling hide() on the wrapper directly is unsafe for some obscure reason
					QTimer::singleShot (0, wrapper, SLOT (hide()));
					activate (true);
					return true;
				}
			}
		}
	}
	return false;
}

bool RKMDIWindow::acceptsEventsFor (QObject *object) {
	// called very often. Don't trace

	if (object == getPart ()) return true;
	if (isToolWindow () && (object == wrapper->wrapperWidget ())) return true;
	return false;
}

void RKMDIWindow::initializeActivationSignals () {
	RK_TRACE (APP);

	RK_ASSERT (getPart ());
	getPart ()->installEventFilter (this);
	getPart ()->actionCollection ()->setHighlightingEnabled (true);
}

void RKMDIWindow::paintEvent (QPaintEvent *e) {
	// RK_TRACE (APP); Do not trace!

	Q3Frame::paintEvent (e);

	if (isActive ()) {
		QPainter paint (this);
		paint.setPen (QColor (255, 0, 0));
		paint.drawLine (0, 0, 0, height ()-1);
		paint.drawLine (0, height ()-1, width ()-1, height ()-1);
		paint.drawLine (0, 0, width ()-1, 0);
		paint.drawLine (width ()-1, 0, width ()-1, height ()-1);
	}
}

void RKMDIWindow::setToolWrapper (KMdiToolViewAccessor *wrapper_widget) {
	RK_TRACE (APP);

	wrapper = wrapper_widget;
	wrapper->wrapperWidget ()->installEventFilter (this);
	static_cast<KDockWidget *> (wrapper->wrapperWidget ())->setEnableDocking (KDockWidget::DockFullSite);
}

void RKMDIWindow::windowActivationChange (bool) {
	RK_TRACE (APP);

	// NOTE: active is NOT the same as isActive(). Active just means that this window *would* be active, if its toplevel window is active.
	if (active || (!isAttached ())) update ();
}

#include "rkmdiwindow.moc"
