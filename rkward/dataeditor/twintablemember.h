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

#define LABEL_ROW 0
#define TYPE_ROW 1
#define NAME_ROW 4

class QMouseEvent;

/**
  *@author Thomas Friedrichsmeier
  */

class TwinTableMember : public QTable  {
	Q_OBJECT
public: 
	TwinTableMember (QWidget *parent=0, int trailing_rows=0, int trailing_cols=0);
	~TwinTableMember();
/** stores the position of the mouse, when headerRightClick gets emitted */
	QPoint mouse_at;
	TwinTableMember *varTable ();
	QString rText (int row, int col);
	TwinTableMember *getTwin () { return twin; };
/** Checks all cells in a column for validity (e.g. if the type was changed) */
	void checkColValid (int col);
/** reimplemented from QTable to return only the number of used rows */
	int numRows ();
	int numAllRows ();
/** reimplemented from QTable to return only the number of used columns */
	int numCols ();
	int numAllCols ();
signals:
	void headerRightClick (int row, int col);
private:
friend class RKwardDoc;
	TwinTableMember *twin;
	TwinTableMember *var_table;
	static bool changing_width;
	int trailing_rows;
	int trailing_cols;
friend class TwinTable;
	void setTwin (TwinTableMember *new_twin);
	void setVarTable (TwinTableMember *table);
protected slots:
	void columnWidthChanged (int col);
protected:
	bool eventFilter (QObject *object, QEvent *event);
	void focusOutEvent (QFocusEvent *e);
/** reimplemented from QTable to not begin editing if a selection is in place */
	QWidget *beginEdit (int row, int col, bool replace);
};

#endif
