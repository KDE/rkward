/***************************************************************************
                          celleditor  -  description
                             -------------------
    begin                : Mon Sep 13 2004
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
#include "celleditor.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qstyle.h>

#include "twintablemember.h"
#include "../debug.h"

CellEditor::CellEditor (TwinTableMember *parent, const QString &text, int mode, const RObject::ValueLabels *named_values) : QLineEdit (parent->viewport ()) {
	RK_TRACE (EDITOR);
	
	table = parent;
	qDebug ("create: cols: %d, true cols: %d, current col: %d", table->numCols (), table->numTrueCols (), table->currentColumn ());
	
	setText (text);
	setFrame (false);
	selectAll ();
	
	timer_id = 0;
	if (named_values) {
		value_list = new QPopupMenu ();
		value_list->setFont (font ());
		value_list->setPalette (palette ());
		value_list->setFrameStyle (QFrame::Box | QFrame::Plain);
		value_list->setLineWidth (1);
		value_list->setFocusProxy (this);
		
		connect(value_list, SIGNAL (activated (int)), SLOT (selectedFromList (int)));
		
		int i=0;
		for (RObject::ValueLabels::const_iterator it = named_values->constBegin (); it != named_values->constEnd (); ++it) {
			popup_values.insert (value_list->insertItem (it.key () + ": " + it.data (), i), &(it.key ()));
			i++;
		}
		
		timer_id = startTimer (200);
	} else {
		value_list = 0;
	}
}

CellEditor::~CellEditor () {
	RK_TRACE (EDITOR);
	if (value_list) {
		value_list->setFocusProxy (0);
		delete value_list;
	}
}

void CellEditor::selectedFromList (int id) {
	RK_TRACE (EDITOR);
	setText (*popup_values[id]);
}

void CellEditor::timerEvent (QTimerEvent *e) {
	if (e->timerId () != timer_id) {
		QLineEdit::timerEvent (e);
		return;
	}
	RK_TRACE (EDITOR);
	
	RK_ASSERT (value_list);
	
	QPoint pos = mapToGlobal (QPoint (5, height ()+5));

	value_list->popup (QPoint (pos));
	
	killTimer (timer_id);
	timer_id = 0;
}

bool CellEditor::event (QEvent *e) {
	if (e->type () == QEvent::KeyPress) {
		QKeyEvent *kev = static_cast<QKeyEvent *> (e);
		if ((kev->key () == Qt::Key_Tab) || (kev->key () == Qt::Key_BackTab)) {
			table->keyPressEvent (kev);
			return true;
		}
	}
	return QLineEdit::event (e);
}

void CellEditor::keyPressEvent (QKeyEvent *e) {
	if (!e->state ()) {
		if (e->key () == Qt::Key_Left) {
			if (cursorPosition () < 1) {
				table->keyPressEvent (e);
				return;
			}
		} else if (e->key () == Qt::Key_Right) {
			if (cursorPosition () >= text ().length ()) {
				qDebug ("cols: %d, true cols: %d, current col: %d", table->numCols (), table->numTrueCols (), table->currentColumn ());
				table->keyPressEvent (e);
				return;
			}
		}
	}
	QLineEdit::keyPressEvent (e);
}

#include "celleditor.moc"
