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
class TwinTable;
class CellEditor;

/**
  *@author Thomas Friedrichsmeier
  */

class TwinTableMember : public QTable {
	Q_OBJECT
public: 
	TwinTableMember (QWidget *parent, TwinTable *table, int trailing_rows=0, int trailing_cols=0);
	~TwinTableMember();
/** stores the position of the mouse, when headerRightClick gets emitted */
	QPoint mouse_at;
/// TODO: can this be removed?
	virtual TwinTableMember *varTable () { return this; };
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
/** reimplemented form QTable not to use QTableItems. This one raises an assert (should never be called) */
	void removeRows (const QMemArray<int> &rows);
/** reimplemented form QTable not to use QTableItems. This one raises an assert (should never be called) */
	void swapRows (int row1, int row2, bool swapHeader);
/** reimplemented form QTable not to use QTableItems. This one raises an assert (should never be called) */
	void swapCells (int row1, int col1, int row2, int col2);
/** reimplemented form QTable not to use QTableItems. This one raises an assert (should never be called) */
	void swapColumns (int col1, int col2, bool swapHeader);
/** reimplemented form QTable not to use QTableItems. This one always returns 0 */
	QTableItem *item (int, int) { return 0; }
/** reimplemented form QTable not to use QTableItems. This one has no effect */
	void setItem (int, int, QTableItem *) {};
/** reimplemented form QTable not to use QTableItems. This one has no effect */
	void takeItem (QTableItem *) {};
/** reimplemented form QTable not to use QTableItems. This one always returns 0 or tted */
	QWidget *cellWidget (int row, int col) const;
/** reimplemented form QTable not to use QTableItems. This one has no effect */
	void clearCellWidget (int, int) {};
/** reimplemented form QTable not to use QTableItems. This one has no effect */
	void setCellWidget (int, int, QWidget *) {};
/** ends editing. Actually it's just a simple wrapper around QTable::endEdit () */
	void stopEditing ();
signals:
	void headerRightClick (int row, int col);
protected:
/** reimplemented form QTable not to use QTableItems. This one has no effect */
	void resizeData (int len) {};
/** reimplemented form QTable not to use QTableItems. This one has no effect */
	void insertWidget (int, int, QWidget *) {};
friend class RKwardDoc;
	TwinTableMember *twin;
	TwinTable *table;
	static bool changing_width;
	int trailing_rows;
	int trailing_cols;
	CellEditor *tted;
	bool focussing_editor;
friend class TwinTable;
	void setTwin (TwinTableMember *new_twin);
public slots:
	void editorLostFocus ();
protected slots:
	void columnWidthChanged (int col);
protected:
	bool eventFilter (QObject *object, QEvent *event);
/** reimplemented from QTable to not begin editing if a selection is in place */
	QWidget *beginEdit (int row, int col, bool replace);
};

#endif
