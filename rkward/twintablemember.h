/***************************************************************************
                          twintablemember.h  -  description
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

#ifndef TWINTABLEMEMBER_H
#define TWINTABLEMEMBER_H

#include <qtable.h>
#include <qpoint.h>

class QMouseEvent;

/**
  *@author Thomas Friedrichsmeier
  */

class TwinTableMember : public QTable  {
	Q_OBJECT
public: 
	TwinTableMember(QWidget *parent=0, const char *name=0);
	~TwinTableMember();
/** stores the position of the mouse, when headerRightClick gets emitted */
	QPoint mouse_at;
signals:
	void headerRightClick (int col);
private:
	TwinTableMember * twin;
	static bool changing_width;
friend class TwinTable;
	void setTwin (TwinTableMember * new_twin);
protected slots:
	void columnWidthChanged (int col);
protected:
	bool eventFilter (QObject *object, QEvent *event);
};

#endif
