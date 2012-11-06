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
#ifndef CELLEDITOR_H
#define CELLEDITOR_H

#include <QLineEdit>
#include <QList>

#include "../core/robject.h"
#include "rktableview.h"

class QStringList;
class QMenu;
class QEvent;
class QKeyEvent;

/**
This is the main editor used in the TwinTableMembers

TODO: the acutal editor will have to be separated from the value_list-popup in order to allow showing the popup, even if the list does not have strong
focus.

@author Thomas Friedrichsmeier
*/
class CellEditor : public QLineEdit {
Q_OBJECT
public:
	CellEditor (QWidget* parent);
	~CellEditor ();

	void setValueLabels (const RObject::ValueLabels& labels);

	void setText (const QString& text);
signals:
	void done (QWidget* widget, RKItemDelegate::EditorDoneReason reason);
public slots:
	void selectedFromList (QAction* action);
	void showValueLabels ();
protected:
/// reimplemented to ignore arrow left/right if at the beginning/end
	void keyPressEvent (QKeyEvent *e);
	bool eventFilter (QObject* object, QEvent* event);
private:
	QMenu *value_list;
};

#endif
