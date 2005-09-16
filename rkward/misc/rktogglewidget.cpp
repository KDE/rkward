/***************************************************************************
                          rktogglewidget  -  description
                             -------------------
    begin                : Fri Aug 20 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rktogglewidget.h"

#include "../rkglobals.h"
#include "../rkward.h"

RKToggleWidget::RKToggleWidget (QWidget *parent) : QWidget (parent) {
//	connect (this, SIGNAL (closed ()), RKGlobals::rkApp (), SLOT (slotToggleWindowClosed ()));
}

RKToggleWidget::~RKToggleWidget () {
}

void RKToggleWidget::closeEvent (QCloseEvent *e) {
	e->accept ();
	hide ();
	emit (closed ());
}

#include "rktogglewidget.moc"
