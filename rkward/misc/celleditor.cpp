/***************************************************************************
                          celleditor  -  description
                             -------------------
    begin                : Mon Sep 13 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
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

#include <QMenu>
#include <QTimer>
#include <QEvent>
#include <QKeyEvent>

#include "../debug.h"

CellEditor::CellEditor (QWidget* parent) : QLineEdit (parent) {
	RK_TRACE (EDITOR);

	setFrame (false);

	value_list = 0;
}

CellEditor::~CellEditor () {
	RK_TRACE (EDITOR);
}

void CellEditor::setValueLabels (const RObject::ValueLabels& labels) {
	RK_TRACE (EDITOR);

	if (labels.isEmpty ()) return;

// NOTE: not using a QComboBox, as we do not want it to pop up immediately
	value_list = new QMenu (this);
	value_list->setFont (font ());
	value_list->setPalette (palette ());
	value_list->setFocusProxy (this);
	value_list->installEventFilter (this);	// somehow setting us as a focus proxy is not enough to continue to receive the key-presses

	for (RObject::ValueLabels::const_iterator it = labels.constBegin (); it != labels.constEnd (); ++it) {
		value_list->addAction (it.key () + ": " + it.value ())->setData (it.key ());
	}
	connect (value_list, SIGNAL (triggered(QAction*)), SLOT (selectedFromList(QAction*)));

	QTimer::singleShot (200, this, SLOT (showValueLabels ()));
}

void CellEditor::selectedFromList (QAction* action) {
	RK_TRACE (EDITOR);
	RK_ASSERT (action);

	setText (action->data ().toString ());	// which is a string representation of an int, really
}

void CellEditor::setText (const QString& text) {
	RK_TRACE (EDITOR);

	QLineEdit::setText (text);
	selectAll ();
}

void CellEditor::showValueLabels () {
	RK_TRACE (EDITOR);
	RK_ASSERT (value_list);

	QPoint pos = mapToGlobal (QPoint (5, height ()+5));
	value_list->popup (QPoint (pos));
}

void CellEditor::keyPressEvent (QKeyEvent *e) {
	if (e->modifiers () == Qt::NoModifier) {
		if ((e->key () == Qt::Key_Left) || (e->key () == Qt::Key_Backspace)) {
			if (cursorPosition () < 1) {
				emit (done (this, RKItemDelegate::EditorExitLeft));
				return;
			}
		}
		if (e->key () == Qt::Key_Right) {
			if (cursorPosition () >= (int) text ().length ()) {
				emit (done (this, RKItemDelegate::EditorExitRight));
				return;
			}
		}
		if (e->key () == Qt::Key_Up) {
			emit (done (this, RKItemDelegate::EditorExitUp));
			return;
		}
		if (e->key () == Qt::Key_Down) {
			emit (done (this, RKItemDelegate::EditorExitDown));
			return;
		}
	}
	QLineEdit::keyPressEvent (e);
}

bool CellEditor::eventFilter (QObject* object, QEvent* e) {
	if (object && (object == value_list)) {
		if (e->type() == QEvent::KeyPress) {
			RK_TRACE (EDITOR);
			return event (e);
		}
	}
	return false;
}

#include "celleditor.moc"
