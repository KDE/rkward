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
#define LEVELS_ROW 2
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
/** returns cell-value in a form suitable for submission to R (e.g. quoted for strings). Default implementation simply quotes the result of text () */
	virtual QString rText (int row, int col) const;
	TwinTableMember *getTwin () { return twin; };
/** reimplemented from QTable to return only the number of used rows */
	int numRows ();
	int numAllRows ();
/** like QTable::numCols (), but returns only the "true", i.e. active columns (excluding the trailing_cols) */
	int numTrueCols ();
/** reimplemented form QTable not to use QTableItems. This one raises an assert (should never be called) */
	void removeRows (const QMemArray<int> &rows);
/** reimplemented form QTable not to use QTableItems. This one has no effect */
	void swapRows (int row1, int row2, bool swapHeader);
/** reimplemented form QTable not to use QTableItems. This one raises an assert (should never be called) */
	void swapCells (int row1, int col1, int row2, int col2);
/** reimplemented form QTable not to use QTableItems. This one has no effect */
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
/** reimplemented form QTable not to work on TableColumns instead of QTableItems. */
	void endEdit (int row, int col, bool accept, bool replace);
/** reimplemented form QTable not to work on TableColumns instead of QTableItems */
	void setCellContentFromEditor (int row, int col);
/** needed to detect right mouse clicks in the header and tab-keypresses in the CellEditor */
	bool eventFilter (QObject *object, QEvent *event);
/** actually simply calls QTable::keyPressEvent (). Reimplemented only to allow CellEditor access to this function */
	void keyPressEvent (QKeyEvent *e) { QTable::keyPressEvent (e); };
signals:
	void headerRightClick (int row, int col);
protected:
/** reimplemented form QTable not to use QTableItems. This one has no effect */
	void resizeData (int) {};
/** reimplemented form QTable not to use QTableItems. This one has no effect */
	void insertWidget (int, int, QWidget *) {};
friend class RKwardDoc;
	TwinTableMember *twin;
	TwinTable *table;
	static bool changing_width;
	int trailing_rows;
	int trailing_cols;
	CellEditor *tted;
friend class TwinTable;
	void setTwin (TwinTableMember *new_twin);
public slots:
	void editorLostFocus ();
/** called when the current cell is changed. If no selection is in place, will (does not do it yet) pop up the value-list */
	void currentCellChanged (int row, int col);
protected slots:
	void columnWidthChanged (int col);
};

#endif
