/***************************************************************************
                          twintablemember.h  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002, 2006, 2007 by Thomas Friedrichsmeier
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

#include <QTableView>
#include <QItemSelectionRange>
#include <qpoint.h>
//Added by qt3to4:
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

class QMouseEvent;
class TwinTable;
class CellEditor;
class RKVarEditModelBase;

#include "rkeditor.h"

/** One of the tables used in a TwinTable.
@author Thomas Friedrichsmeier
*/
class TwinTableMember : public QTableView {
	Q_OBJECT
public: 
	TwinTableMember (QWidget *parent, TwinTable *table);
	~TwinTableMember();
	TwinTableMember *getTwin () { return twin; };
/** like QTable::numRows (), but returns only the "true", i.e. active rows (excluding the trailing_rows) */
	int numTrueRows () const;
/** like QTable::numCols (), but returns only the "true", i.e. active columns (excluding the trailing_cols) */
	int numTrueCols () const;
/** ends editing. Actually it's just a simple wrapper around QTable::endEdit () */
	void stopEditing ();
#warning maybe still needed?
/** needed to detect right mouse clicks in the header and tab-keypresses in the CellEditor */
//	bool eventFilter (QObject *object, QEvent *event);
/** reimplemented to delete cell contents on DEL and BACKSPACE. Placed in public, here, so CellEditor can have access */
	void keyPressEvent (QKeyEvent *e);

	void copy ();
	void paste (RKEditor::PasteMode mode);

/** blanks out the currently selected cells (or the currently active cell, if there is no selection) */
	void blankSelected ();
/** shortcut to get the boundaries of the current selection */
	QItemSelectionRange getSelectionBoundaries ();

	void setRKModel (RKVarEditModelBase* model);
signals:
	void contextMenuRequest (int row, int col, const QPoint& pos);
protected:
	TwinTableMember *twin;
	TwinTable *table;
	bool updating_twin;

	CellEditor *tted;
/** reimplemented from QTableView to also adjust the twin */
	void scrollContentsBy (int dx, int dy);

	RKVarEditModelBase* mymodel;
friend class TwinTable;
	void setTwin (TwinTableMember *new_twin);
public slots:
	void editorLostFocus ();
/** called when the current cell is changed. If no selection is in place, will (does not do it yet) pop up the value-list */
//	void currentCellChanged (int row, int col);
protected slots:
	void headerContextMenuRequested (const QPoint& pos);
	void updateColWidth (int section, int old_w, int new_w);
	void tableSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected);
};

#endif
