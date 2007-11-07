/***************************************************************************
                          twintable.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002, 2006 by Thomas Friedrichsmeier
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

#include "twintable.h"
#include <klocale.h>

#include <qvariant.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <q3popupmenu.h>
#include <q3cstring.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3ValueList>

#include "twintabledatamember.h"
#include "twintablemetamember.h"
#include "twintablemember.h"
#include "rkdrag.h"

#include "../debug.h"

#include "../core/robject.h"
#include "../core/rkvariable.h"

#define HEADER_MENU_ID_ADD_COL_LEFT 0
#define HEADER_MENU_ID_ADD_COL_RIGHT 1
#define HEADER_MENU_ID_DEL_COL 2

#define HEADER_MENU_ID_ADD_ROW_ABOVE 0
#define HEADER_MENU_ID_ADD_ROW_BELOW 1
#define HEADER_MENU_ID_DEL_ROW 2
#define HEADER_MENU_ID_DEL_ROWS 3


TwinTable::TwinTable (QWidget *parent) : RKEditor (parent) {
	RK_TRACE (EDITOR);

	Q3GridLayout *grid_layout = new Q3GridLayout(this);
	
	QSplitter *splitter = new QSplitter(this);
	splitter->setOrientation(Qt::Vertical);
	
	varview = new TwinTableMetaMember (splitter, this);
	varview->setNumRows (5);
	varview->setNumCols (5);
	varview->verticalHeader ()->setLabel (LABEL_ROW, i18n ("Label"));
	varview->verticalHeader ()->setLabel (TYPE_ROW, i18n ("Type"));
	varview->verticalHeader ()->setLabel (LEVELS_ROW, i18n ("Levels"));
	varview->verticalHeader ()->setLabel (FORMAT_ROW, i18n ("Format"));
	varview->verticalHeader ()->setLabel (NAME_ROW, i18n ("Name"));
	varview->setMinimumHeight (varview->horizontalHeader ()->height ());
	varview->setMaximumHeight (varview->rowPos (NAME_ROW) + varview->rowHeight (NAME_ROW) + varview->horizontalHeader ()->height () + 5);
	splitter->setResizeMode (varview, QSplitter::KeepSize);
	varview->verticalHeader()->setResizeEnabled (false);
	
	dataview = new TwinTableDataMember (splitter, this);
	dataview->setNumRows (1);
	dataview->setNumCols (5);
	dataview->verticalHeader()->setResizeEnabled (false);
	
	dataview->horizontalHeader ()->hide ();
	dataview->setTopMargin (0);
	dataview->setLeftMargin (varview->leftMargin ());
	varview->setHScrollBarMode (Q3ScrollView::AlwaysOff);
	
	grid_layout->addWidget (splitter, 0, 0);
	
	// these are to keep the two tables in sync
	varview->setTwin (dataview);
	dataview->setTwin (varview);
	connect (dataview, SIGNAL (contentsMoving (int, int)), this, SLOT (scrolled (int, int)));
	connect (varview, SIGNAL (contentsMoving (int, int)), this, SLOT (autoScrolled (int, int)));
	connect (varview->horizontalHeader (), SIGNAL (clicked (int)), dataview, SLOT (columnClicked (int)));
	connect (varview->horizontalHeader (), SIGNAL (clicked (int)), this, SLOT (headerClicked (int)));
	connect (varview, SIGNAL (selectionChanged ()), this, SLOT (dataClearSelection ()));
	connect (dataview, SIGNAL (selectionChanged ()), this, SLOT (viewClearSelection ()));
	
	// this is to catch right-clicks on the top header
	connect (varview, SIGNAL (headerRightClick (int, int)), this, SLOT (varviewHeaderRightClicked (int, int)));
	
	// which will be reacted upon by the following popup-menu:
	top_header_menu = new Q3PopupMenu (this);
	top_header_menu->insertItem (i18n ("Insert new variable left"), this, SLOT (insertColumnLeft ()), 0, HEADER_MENU_ID_ADD_COL_LEFT);
	top_header_menu->insertItem (i18n ("Insert new variable right"), this, SLOT (insertColumnRight ()), 0, HEADER_MENU_ID_ADD_COL_RIGHT);
	top_header_menu->insertItem (i18n ("Delete this variable"), this, SLOT (requestDeleteColumn ()), 0, HEADER_MENU_ID_DEL_COL);
	
	// and the same for the left header
	connect (dataview, SIGNAL (headerRightClick (int, int)), this, SLOT (dataviewHeaderRightClicked (int, int)));
	left_header_menu = new Q3PopupMenu (this);
	left_header_menu->insertItem (i18n ("Insert new case above"), this, SLOT (insertRowAbove ()), 0, HEADER_MENU_ID_ADD_ROW_ABOVE);
	left_header_menu->insertItem (i18n ("Insert new case below"), this, SLOT (insertRowBelow ()), 0, HEADER_MENU_ID_ADD_ROW_BELOW);
	left_header_menu->insertItem (QString::null, this, SLOT (deleteRow ()), 0, HEADER_MENU_ID_DEL_ROW);
	left_header_menu->insertItem (QString::null, this, SLOT (deleteRows ()), 0, HEADER_MENU_ID_DEL_ROWS);

	setFocusPolicy (Qt::StrongFocus);
}

TwinTable::~TwinTable() {
	RK_TRACE (EDITOR);

	delete top_header_menu;
	delete left_header_menu;
	
/*	for (int i=0; i < numTrueCols (); ++i) {
		RObject *object = getColObject (i);
		if (object) object->setObjectOpened (this, false);
		else RK_ASSERT (false);
	} */
}

void TwinTable::scrolled (int x, int) {
	RK_TRACE (EDITOR);

	disconnect (varview, SIGNAL (contentsMoving (int, int)), this, SLOT (autoScrolled (int, int)));
	varview->setContentsPos (x, varview->contentsY ());
	connect (varview, SIGNAL (contentsMoving (int, int)), this, SLOT (autoScrolled (int, int)));
}

void TwinTable::autoScrolled (int x, int) {
	RK_TRACE (EDITOR);

	disconnect (dataview, SIGNAL (contentsMoving (int, int)), this, SLOT (scrolled (int, int)));
	dataview->setContentsPos (x, dataview->contentsY ());
	connect (dataview, SIGNAL (contentsMoving (int, int)), this, SLOT (scrolled (int, int)));
}

void TwinTable::deleteColumn (int column) {
	RK_TRACE (EDITOR);

	flushEdit ();
	if ((column >= 0) && (column < numTrueCols ())) {
		varview->removeColumn (column);
		dataview->removeColumn (column);
		varview->updateContents ();
		dataview->updateContents ();
	}
}

void TwinTable::insertNewColumn (int where) {
	RK_TRACE (EDITOR);

	flushEdit ();
	if ((where < 0) || (where > varview->numCols ())) {
		where = varview->numCols ();
	}

	varview->insertColumns (where);
	dataview->insertColumns (where);

	if (where >= varview->numTrueCols ()) {		// the new addition was acutally not the new trailing row, but the one to the left - for all practical purposes
		where = varview->numTrueCols () - 1;
	}
	emit (addedColumn (where));
}

void TwinTable::insertNewRow (int where, TwinTableMember *table) {
	RK_TRACE (EDITOR);

	flushEdit ();
	if (!table) table = dataview;
	
	if ((where < 0) || (where > table->numTrueRows ())) {
		where = table->numRows ();
	}
	
	if (table == dataview) {
		emit (dataAddingRow (where));
	} else if (table == varview) {
		RK_ASSERT (false);
	}

	table->insertRows (where);
}

void TwinTable::deleteRow (int where, TwinTableMember *table) {
	RK_TRACE (EDITOR);

	flushEdit ();
	if (!table) table = dataview;
	
	if ((where < 0) || (where > table->numTrueRows ())) {
		where = table->numRows () - 1;
	}
	
	if (table == dataview) {
		emit (dataRemovingRow (where));
	} else if (table == varview) {
		RK_ASSERT (false);
	}

	table->removeRow (where);
}

void TwinTable::headerClicked (int col) {
	RK_TRACE (EDITOR);

	Q3TableSelection selection;
	selection.init (0, col);
	selection.expandTo (dataview->numTrueRows (), col);

	dataview->addSelection (selection);
}

// TODO: handle situation when several entire rows/cols are selected!
void TwinTable::varviewHeaderRightClicked (int, int col) {
	RK_TRACE (EDITOR);

	if (col >= 0) {
		header_pos = col;
		if (col < varview->numTrueCols ()) {
			top_header_menu->setItemVisible (HEADER_MENU_ID_ADD_COL_LEFT, true);
			top_header_menu->setItemVisible (HEADER_MENU_ID_ADD_COL_RIGHT, true);
			top_header_menu->setItemVisible (HEADER_MENU_ID_DEL_COL, true);
			top_header_menu->popup (varview->mouse_at);
		} else if (col == varview->numTrueCols ()) {		// trailing col
			top_header_menu->setItemVisible (HEADER_MENU_ID_ADD_COL_LEFT, true);
			top_header_menu->setItemVisible (HEADER_MENU_ID_ADD_COL_RIGHT, false);
			top_header_menu->setItemVisible (HEADER_MENU_ID_DEL_COL, false);
			top_header_menu->popup (varview->mouse_at);
		} else {
			RK_ASSERT (false);
		}
	}
}

void TwinTable::dataviewHeaderRightClicked (int row, int col) {
	RK_TRACE (EDITOR);

	RK_ASSERT (col < 0);
	if (row >= 0) {
		header_pos = row;
		left_header_menu->setItemVisible (HEADER_MENU_ID_ADD_ROW_ABOVE, true);
		int top, bottom, left, right;
		dataview->getSelectionBoundaries (&top, &left, &bottom, &right);
		if (top >= 0 && bottom <= dataview->numTrueRows () && top != bottom) {
			left_header_menu->setItemVisible (HEADER_MENU_ID_DEL_ROWS, true);
			left_header_menu->changeItem (HEADER_MENU_ID_DEL_ROWS, i18n ("Delete marked rows (%1-%2)", (top+1), (bottom+1)));
		} else {
			left_header_menu->setItemVisible (HEADER_MENU_ID_DEL_ROWS, false);
		}
		if (row < dataview->numTrueRows ()) {
			left_header_menu->setItemVisible (HEADER_MENU_ID_ADD_ROW_BELOW, true);
			left_header_menu->setItemVisible (HEADER_MENU_ID_DEL_ROW, true);
			left_header_menu->changeItem (HEADER_MENU_ID_DEL_ROW, i18n ("Delete this row (%1)", (row+1)));
			left_header_menu->popup (dataview->mouse_at);
		} else if (row == dataview->numTrueRows ()) {		// trailing row
			left_header_menu->setItemVisible (HEADER_MENU_ID_ADD_ROW_BELOW, false);
			left_header_menu->setItemVisible (HEADER_MENU_ID_DEL_ROW, false);
			left_header_menu->popup (dataview->mouse_at);
		} else {
			RK_ASSERT (false);
		}
	}
}

void TwinTable::viewClearSelection () {
	RK_TRACE (EDITOR);
	
	disconnect (varview, SIGNAL (selectionChanged ()), this, SLOT (dataClearSelection ()));
	varview->clearSelection ();
	connect (varview, SIGNAL (selectionChanged ()), this, SLOT (dataClearSelection ()));
}

void TwinTable::dataClearSelection () {
	RK_TRACE (EDITOR);

	disconnect (dataview, SIGNAL (selectionChanged ()), this, SLOT (viewClearSelection ()));
	dataview->clearSelection ();
	connect (dataview, SIGNAL (selectionChanged ()), this, SLOT (viewClearSelection ()));
}

RKDrag *TwinTable::makeDrag () {
	RK_TRACE (EDITOR);
	return (new RKDrag (activeTable ()));
}

void TwinTable::insertColumnRight () {
	RK_TRACE (EDITOR);
	insertNewColumn (header_pos+1);
}

void TwinTable::insertColumnLeft () {
	RK_TRACE (EDITOR);
	insertNewColumn (header_pos);
}

void TwinTable::requestDeleteColumn () {
	RK_TRACE (EDITOR);
	emit (deleteColumnRequest (header_pos));
}

void TwinTable::insertRowBelow () {
	RK_TRACE (EDITOR);
	insertNewRow (header_pos+1);
}

void TwinTable::insertRowAbove () {
	RK_TRACE (EDITOR);
	insertNewRow (header_pos);
}

void TwinTable::deleteRow () {
	RK_TRACE (EDITOR);
	deleteRow (header_pos);
}

void TwinTable::deleteRows () {
	RK_TRACE (EDITOR);
// TODO: this is inefficient. Remove all rows at once
	int top, bottom, left, right;
	dataview->getSelectionBoundaries (&top, &left, &bottom, &right);
	for (; bottom >= top; --bottom) {
		deleteRow (bottom);
	}
}

void TwinTable::paste (QByteArray &content) {
	RK_TRACE (EDITOR);

	flushEdit ();
	
	TwinTableMember *table = activeTable ();
	if (!table) return;

	int top_row, left_col, bottom_row, right_col;
	table->getSelectionBoundaries (&top_row, &left_col, &bottom_row, &right_col);
	if (paste_mode == RKEditor::PasteToSelection) {
		// ok, we got our values
	} else if (paste_mode == RKEditor::PasteToTable) {
		bottom_row = table->numTrueRows () - 1;
		right_col = table->numTrueCols () - 1;
		if (right_col < left_col) return;			// may happen, if the current cell is in the trailing cols/rows
		if (bottom_row < top_row) return;
	} else if (paste_mode == RKEditor::PasteEverywhere) {
		bottom_row = INT_MAX;
		right_col = INT_MAX;
	}
	if (table == varview) {			// do not allow new rows in the varview
		if (bottom_row >= varview->numTrueRows ()) bottom_row = varview->numTrueRows () - 1;
	}

	Q3ValueList<RKVariable*> col_list;

	QString pasted = QString::fromLocal8Bit (content);
	int row = top_row;
	int col = left_col;
	int content_offset = 0;
	int content_length = pasted.length ();
	do {
		// first add new rows/cols if needed. Range check is done below, and on first iteration, we're always inside the valid range
		if (row >= table->numTrueRows ()) insertNewRow (-1, table);
		if (col >= table->numTrueCols ()) insertNewColumn ();

		if (!col_list.contains (getColObject (col))) {		// avoid syncing while doing the paste
			col_list.append (getColObject (col));
			getColObject (col)->setSyncing (false);
		}

		int next_tab = pasted.find ('\t', content_offset);
		if (next_tab < 0) next_tab = content_length;
		int next_delim = next_tab;
		int next_line = pasted.find ('\n', content_offset);
		if (next_line < 0) next_line = content_length;
		if (next_line < next_tab) next_delim = next_line;

		table->setText (row, col, pasted.mid (content_offset, next_delim - content_offset));

		if (next_delim == next_tab) {						// move to next row/column
			++col;
		} else if (next_delim == next_line) {
			col = left_col;
			++row;
		}

		if (col > right_col) {										// check boundaries for next iteration
			next_delim = next_line;
			col = left_col;
			++row;
		}
		if (row > bottom_row) break;

		content_offset = next_delim + 1;
	} while (content_offset < content_length);

	// now do the syncing
	for (Q3ValueList<RKVariable*>::ConstIterator it = col_list.constBegin (); it != col_list.constEnd (); ++it) {
		(*it)->syncDataToR ();
		(*it)->setSyncing (true);
	}
}

TwinTableMember *TwinTable::activeTable () {
	RK_TRACE (EDITOR);

	if (varview->hasFocus ()) {
		return varview;
	} else if (dataview->hasFocus ()) {
		return dataview;
	} else {
		return 0;
	}
}

void TwinTable::clearSelected () {
	RK_TRACE (EDITOR);

	TwinTableMember *table = activeTable ();
 	if (!table) return;

	table->blankSelected ();
}

void TwinTable::setPasteMode (RKEditor::PasteMode mode) {
	RK_TRACE (EDITOR);
	paste_mode = mode;
}

void TwinTable::setRow (TwinTableMember* table, int row, int start_col, int end_col, char **data) {
	RK_TRACE (EDITOR);

	flushEdit ();
	while (numTrueCols () <= end_col) {
		insertNewColumn ();
	}
	
	int i=0;
	for (int col=start_col; col <= end_col; ++col) {
		table->setText (row, col, data[i++]);
	}
}

void TwinTable::setColumn (TwinTableMember* table, int col, int start_row, int end_row, char **data) {
	RK_TRACE (EDITOR);

	flushEdit ();
	while (table->numTrueRows () <= end_row) {
		insertNewRow (table->numTrueRows (), table);
	}
	
	int i=0;
	for (int row=start_row; row <= end_row; ++row) {
		table->setText (row, col, data[i++]);
	}
}

void TwinTable::flushEdit () {
	RK_TRACE (EDITOR);

	// flush pending edit operations
	varview->stopEditing ();
	dataview->stopEditing ();
}

int TwinTable::numTrueCols () {
//	RK_TRACE (EDITOR);
	return varview->numTrueCols ();
}

void TwinTable::setColObject (long int column, RKVariable *object) {
	RK_TRACE (EDITOR);
	if (object) {
		col_map.replace (column, object);		// will insert, if not already in dict
	} else {
		col_map.remove (column);
	}
}

RKVariable *TwinTable::getColObject (long int col) {
	// do not trace. called very often
	//RK_TRACE (EDITOR);
	return col_map.find (col);
}

long int TwinTable::getObjectCol (RObject *object) {
	RK_TRACE (EDITOR);
	for (Q3IntDictIterator<RKVariable> it (col_map); it.current (); ++it) {
		if (it.current () == object) return it.currentKey ();
	}
	
	RK_ASSERT (false);
	return -1;
}

#include "twintable.moc"
