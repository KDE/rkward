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
#include <qdict.h>

class QStringList;
class QListBox;

/**
This is the main editor used in the TwinTableMembers

@author Thomas Friedrichsmeier
*/
class CellEditor : public QLineEdit {
Q_OBJECT
public:
	CellEditor (QWidget *parent, const QString &text, int mode, const QDict<QString> *named_values=0);

	~CellEditor();
protected:
/// for showing/hiding list of name_values
	void timerEvent (QTimerEvent *e);
private:
	QListBox *value_list;
};

#endif
