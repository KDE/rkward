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
#include <qlistbox.h>
#include <qstyle.h>

#include "../debug.h"

CellEditor::CellEditor (QWidget *parent, const QString &text, int mode, const QDict<QString> *named_values) : QLineEdit (parent) {
	RK_TRACE (EDITOR);
	
	setText (text);
	setFrame (false);
	selectAll ();
	
	if (named_values) {
		// the next dozen lines basically copied from QComboBox
		value_list = new QListBox (this);
		value_list->setFont (font ());
		value_list->setPalette (palette ());
		value_list->setVScrollBarMode (QListBox::AlwaysOff);
		value_list->setHScrollBarMode (QListBox::AlwaysOff);
		value_list->setFrameStyle (QFrame::Box | QFrame::Plain);
		value_list->setLineWidth (1);
		value_list->resize (100, 10);
		
		//connect(value_list, SIGNAL(selected (int)), SLOT(selectFromList (int)));
		
		for (QDictIterator<QString> it (*named_values); it.current (); ++it) {
			value_list->insertItem (*(it.current ()));
		}
		
		startTimer (200);
	} else {
		value_list = 0;
	}
}

CellEditor::~CellEditor () {
	RK_TRACE (EDITOR);
}

void CellEditor::timerEvent (QTimerEvent *e) {
	RK_TRACE (EDITOR);
	if (!value_list) return;
	
	// stuff below basicall copied from QComboBox
	int w = value_list->variableWidth() ? value_list->sizeHint().width() : width();
	int h = value_list->height () + 2;
	
	QRect screen = QApplication::desktop()->availableGeometry( this );

	int sx = screen.x();                            // screen pos
	int sy = screen.y();
	int sw = screen.width();                        // screen width
	int sh = screen.height();                       // screen height
	QPoint pos = mapToGlobal( QPoint(0,height()) );
	// ## Similar code is in QPopupMenu
	int x = pos.x();
	int y = pos.y();

	// the complete widget must be visible
	if ( x + w > sx + sw )
		x = sx+sw - w;
	if ( x < sx )
		x = sx;
	if (y + h > sy+sh && y - h - height() >= 0 )
		y = y - h - height();

	QRect rect =
		style().querySubControlMetrics( QStyle::CC_ComboBox, this,
										QStyle::SC_ComboBoxListBoxPopup,
										QStyleOption( x, y, w, h ) );
	// work around older styles that don't implement the combobox
	// listbox popup subcontrol
	if ( rect.isNull() )
		rect.setRect( x, y, w, h );
	value_list->setGeometry( rect );

	value_list->raise();
}

#include "celleditor.moc"
