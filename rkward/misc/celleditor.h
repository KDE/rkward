/*
celleditor - This file is part of the RKWard project. Created: Mon Sep 13 2004
SPDX-FileCopyrightText: 2004-2007 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef CELLEDITOR_H
#define CELLEDITOR_H

#include <QLineEdit>
#include <QList>

#include "../core/robject.h"
#include "rktableview.h"

class QMenu;
class QEvent;
class QKeyEvent;

/**
This is the main editor used in the TwinTableMembers

TODO: the actual editor will have to be separated from the value_list-popup in order to allow showing the popup, even if the list does not have strong
focus.

@author Thomas Friedrichsmeier
*/
class CellEditor : public QLineEdit {
Q_OBJECT
public:
	explicit CellEditor (QWidget* parent);
	~CellEditor ();

	void setValueLabels (const RObject::ValueLabels& labels);

	void setText (const QString& text);
Q_SIGNALS:
	void done (QWidget* widget, RKItemDelegate::EditorDoneReason reason);
public Q_SLOTS:
	void selectedFromList (QAction* action);
	void showValueLabels ();
protected:
/// reimplemented to ignore arrow left/right if at the beginning/end
	void keyPressEvent (QKeyEvent *e) override;
	bool eventFilter (QObject* object, QEvent* event) override;
private:
	QMenu *value_list;
};

#endif
