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
#ifndef CELLEDITOR_H
#define CELLEDITOR_H

#include <qlineedit.h>
#include <qintdict.h>

#include "../core/robject.h"

class QStringList;
class QPopupMenu;

/**
This is the main editor used in the TwinTableMembers

TODO: the acutal editor will have to be separated from the value_list-popup in order to allow showing the popup, even if the list does not have strong
focus.

@author Thomas Friedrichsmeier
*/
class CellEditor : public QLineEdit {
Q_OBJECT
public:
	CellEditor (QWidget *parent, const QString &text, int mode, const RObject::ValueLabels *named_values=0);

	~CellEditor();
public slots:
	void selectedFromList (int id);
protected:
/// for showing/hiding list of name_values
	void timerEvent (QTimerEvent *e);
/// reimplemented to ignore arrow left/right if at the beginning/end
	void keyPressEvent (QKeyEvent *e);
private:
	QPopupMenu *value_list;
	int timer_id;
	QIntDict<QString> popup_values;
};

#endif
