/***************************************************************************
                          twintablemember.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#include "twintablemember.h"

#include <qevent.h>

bool TwinTableMember::changing_width = false;

TwinTableMember::TwinTableMember(QWidget *parent, const char *name) : QTable (parent, name){
	twin = 0;
	setRowMovingEnabled (false);
	setVScrollBarMode (QScrollView::AlwaysOn);
	horizontalHeader()->installEventFilter (this);
}

TwinTableMember::~TwinTableMember(){
}

void TwinTableMember::setTwin (TwinTableMember * new_twin) {
	twin = new_twin;
}

void TwinTableMember::columnWidthChanged (int col) {
	// does all repainting and stuff ...
	QTable::columnWidthChanged (col);

	// syncs the twin
	if (twin) {
		if (!changing_width) {
			changing_width = true;
			twin->setColumnWidth (col, columnWidth (col));
		}
		changing_width = false;
	}
}

bool TwinTableMember::eventFilter (QObject *object, QEvent *event)
{
	static int i = 0;
    // filter out right mouse button events of the varview-header
    if (object == horizontalHeader()) {
        if (event && (event->type() == QEvent::MouseButtonPress)) {
            QMouseEvent  *mouseEvent = (QMouseEvent *) event;
            if (mouseEvent && (mouseEvent->button() == Qt::RightButton)) {
				qDebug ("got event here %d", i++);
				mouse_at = mouseEvent->globalPos ();
				emit headerRightClick (horizontalHeader ()->sectionAt (mouseEvent->x ()));
                return(true); // got it
            }
        }
    }

    // default processing
    return(QTable::eventFilter (object, event));
}
