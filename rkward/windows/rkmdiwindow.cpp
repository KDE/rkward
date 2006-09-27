/***************************************************************************
                          rkmdiwindow  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#include "rkworkplace.h"

#include "../debug.h"

RKMDIWindow::RKMDIWindow (QWidget *parent, Type type) : QWidget (parent) {
	RK_TRACE (APP);

	RKMDIWindow::type = type;
	state = Attached;
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

#include "rkmdiwindow.moc"
