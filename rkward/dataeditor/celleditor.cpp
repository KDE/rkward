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

#include "../debug.h"

CellEditor::CellEditor (QWidget *parent, const QString &text, int mode, const QMap<QString, QString> *named_values) : QLineEdit (parent) {
	RK_TRACE (EDITOR);
	
	setText (text);
	setFrame (false);
	selectAll ();
	
	timer_id = 0;
	if (named_values) {
		value_list = new QPopupMenu (this);
		value_list->setFont (font ());
		value_list->setPalette (palette ());
		value_list->setFrameStyle (QFrame::Box | QFrame::Plain);
		value_list->setLineWidth (1);
		value_list->setFocusProxy (this);
		
		//connect(value_list, SIGNAL(selected (int)), SLOT(selectFromList (int)));
		
		for (QMap<QString, QString>::const_iterator it = named_values->constBegin (); it != named_values->constEnd (); ++it) {
			value_list->insertItem (it.key () + ": " + it.data ());
		}
		
		timer_id = startTimer (200);
	} else {
		value_list = 0;
	}
}

CellEditor::~CellEditor () {
	RK_TRACE (EDITOR);
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

#include "celleditor.moc"
