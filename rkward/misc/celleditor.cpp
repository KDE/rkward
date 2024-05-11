/*
celleditor - This file is part of RKWard (https://rkward.kde.org). Created: Mon Sep 13 2004
SPDX-FileCopyrightText: 2004-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "celleditor.h"

#include <QMenu>
#include <QTimer>
#include <QEvent>
#include <QKeyEvent>

#include <KLocalizedString>

#include "../debug.h"

CellEditor::CellEditor (QWidget* parent) : QLineEdit (parent) {
	RK_TRACE (EDITOR);

	setFrame (false);

	value_list = nullptr;
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

	const int limit = 64;
	int i = 0;
	for (RObject::ValueLabels::const_iterator it = labels.constBegin (); it != labels.constEnd (); ++it) {
		if (++i >= limit) break;
		value_list->addAction (it.key () + ": " + it.value ())->setData (it.key ());
	}
	if (i >= limit) {
		value_list->addAction (i18n ("[Omitted %1 more factor levels]", labels.size () - limit))->setEnabled (false);
	}
	connect (value_list, &QMenu::triggered, this, &CellEditor::selectedFromList);

	QTimer::singleShot(200, this, &CellEditor::showValueLabels);
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
				Q_EMIT done(this, RKItemDelegate::EditorExitLeft);
				return;
			}
		}
		if (e->key () == Qt::Key_Right) {
			if (cursorPosition () >= (int) text ().length ()) {
				Q_EMIT done(this, RKItemDelegate::EditorExitRight);
				return;
			}
		}
		if (e->key () == Qt::Key_Up) {
			Q_EMIT done(this, RKItemDelegate::EditorExitUp);
			return;
		}
		if (e->key () == Qt::Key_Down) {
			Q_EMIT done(this, RKItemDelegate::EditorExitDown);
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

