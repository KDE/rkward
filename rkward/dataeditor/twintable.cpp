/***************************************************************************
                          twintable.cpp  -  description
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

#include "twintable.h"
#include <klocale.h>

#include <qvariant.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qcstring.h>

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


TwinTable::TwinTable (QWidget *parent) : RKEditor (parent){
	QGridLayout *grid_layout = new QGridLayout(this);

    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(QSplitter::Vertical);

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
	varview->setHScrollBarMode (QScrollView::AlwaysOff);

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
	connect (varview, SIGNAL (headerRightClick (int, int)), this, SLOT (headerRightClicked (int, int)));

	// which will be reacted upon by the following popup-menu:
	top_header_menu = new QPopupMenu (this);
	top_header_menu->insertItem (i18n ("Insert new variable left"), this, SLOT (insertColumnLeft ()), 0, HEADER_MENU_ID_ADD_COL_LEFT);
	top_header_menu->insertItem (i18n ("Insert new variable right"), this, SLOT (insertColumnRight ()), 0, HEADER_MENU_ID_ADD_COL_RIGHT);
	top_header_menu->insertItem (i18n ("Delete this variable"), this, SLOT (requestDeleteColumn ()), 0, HEADER_MENU_ID_DEL_COL);

	// and the same for the left header
	connect (dataview, SIGNAL (headerRightClick (int, int)), this, SLOT (headerRightClicked (int, int)));
	left_header_menu = new QPopupMenu (this);
	left_header_menu->insertItem (i18n ("Insert new case above"), this, SLOT (insertRowAbove ()), 0, HEADER_MENU_ID_ADD_ROW_ABOVE);
	left_header_menu->insertItem (i18n ("Insert new case below"), this, SLOT (insertRowBelow ()), 0, HEADER_MENU_ID_ADD_ROW_BELOW);
	left_header_menu->insertItem (i18n ("Delete this case"), this, SLOT (deleteRow ()), 0, HEADER_MENU_ID_DEL_ROW);
	
	qDebug ("Twintable created");
}

TwinTable::~TwinTable(){
	delete top_header_menu;
	delete left_header_menu;
	
	for (int i=0; i < numTrueCols (); ++i) {
		RObject *object = getColObject (i);
		if (object) object->setObjectOpened (this, false);
		else RK_ASSERT (false);
	}
}

void TwinTable::scrolled (int x, int) {
	disconnect (varview, SIGNAL (contentsMoving (int, int)), this, SLOT (autoScrolled (int, int)));
	varview->setContentsPos (x, varview->contentsY ());
	connect (varview, SIGNAL (contentsMoving (int, int)), this, SLOT (autoScrolled (int, int)));
}

void TwinTable::autoScrolled (int x, int) {
	disconnect (dataview, SIGNAL (contentsMoving (int, int)), this, SLOT (scrolled (int, int)));
	dataview->setContentsPos (x, dataview->contentsY ());
	connect (dataview, SIGNAL (contentsMoving (int, int)), this, SLOT (scrolled (int, int)));
}

void TwinTable::deleteColumn (int column) {
	flushEdit ();
	if ((column >= 0) && (column < numTrueCols ())) {
		varview->removeColumn (column);
		dataview->removeColumn (column);
	}
}

void TwinTable::insertNewColumn (int where) {
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
	flushEdit ();
	if (!table) table = dataview;
	
	if ((where < 0) || (where > table->numRows ())) {
		where = table->numAllRows ();
	}
	
	if (table == dataview) {
		emit (dataAddingRow (where));
	} else if (table == varview) {
		RK_ASSERT (false);
	}

	table->insertRows (where);
}

void TwinTable::deleteRow (int where, TwinTableMember *table) {
	flushEdit ();
	if (!table) table = dataview;
	
	if ((where < 0) || (where > table->numRows ())) {
		where = table->numRows ();
	}
	
	if (table == dataview) {
		emit (dataRemovingRow (where));
	} else if (table == varview) {
		RK_ASSERT (false);
	}

	table->removeRow (where);
}

void TwinTable::headerClicked (int col) {
	QTableSelection selection;
	selection.init (0, col);
	selection.expandTo (dataview->numRows (), col);

	dataview->addSelection (selection);
}

// TODO: handle situation when several entire rows/cols are selected!
void TwinTable::headerRightClicked (int row, int col) {
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
	} else if (row >= 0) {
		header_pos = row;
		if (row < dataview->numRows ()) {
			left_header_menu->setItemVisible (HEADER_MENU_ID_ADD_ROW_ABOVE, true);
			left_header_menu->setItemVisible (HEADER_MENU_ID_ADD_ROW_BELOW, true);
			left_header_menu->setItemVisible (HEADER_MENU_ID_DEL_ROW, true);
			left_header_menu->popup (dataview->mouse_at);
		} else if (row == dataview->numRows ()) {		// trailing row
			left_header_menu->setItemVisible (HEADER_MENU_ID_ADD_ROW_ABOVE, true);
			left_header_menu->setItemVisible (HEADER_MENU_ID_ADD_ROW_BELOW, false);
			left_header_menu->setItemVisible (HEADER_MENU_ID_DEL_ROW, false);
			left_header_menu->popup (dataview->mouse_at);
		} else {
			RK_ASSERT (false);
		}
	}
}

void TwinTable::viewClearSelection () {
	disconnect (varview, SIGNAL (selectionChanged ()), this, SLOT (dataClearSelection ()));
	varview->clearSelection ();
	connect (varview, SIGNAL (selectionChanged ()), this, SLOT (dataClearSelection ()));
}

void TwinTable::dataClearSelection () {
	disconnect (dataview, SIGNAL (selectionChanged ()), this, SLOT (viewClearSelection ()));
	dataview->clearSelection ();
	connect (dataview, SIGNAL (selectionChanged ()), this, SLOT (viewClearSelection ()));
}

RKDrag *TwinTable::makeDrag () {
	return (new RKDrag (this));
}

void TwinTable::insertColumnRight () {
	insertNewColumn (header_pos+1);
}

void TwinTable::insertColumnLeft () {
	insertNewColumn (header_pos);
}

void TwinTable::requestDeleteColumn () {
	emit (deleteColumnRequest (header_pos));
}

void TwinTable::insertRowBelow () {
	insertNewRow (header_pos+1);
}

void TwinTable::insertRowAbove () {
	insertNewRow (header_pos);
}

void TwinTable::deleteRow () {
	deleteRow (header_pos);
}

QCString TwinTable::encodeSelection () {
	QCString encoded_data;
	QTable *table = activeTable ();
	if (!table) {
		// no selection -> return empty
		return encoded_data;
	}

// TODO: check whether there actually is a selection
	QTableSelection selection;
	selection = table->selection (table->currentSelection ());

	QString data;
	for (int row=selection.topRow (); row <= selection.bottomRow (); row++) {
		for (int col=selection.leftCol (); col <= selection.rightCol (); col++) {
			data.append (table->text (row, col));
			if (col != selection.rightCol ()) {
				data.append ("\t");
			}
		}
		if (row != selection.bottomRow ()) {
			data.append ("\n");
		}
	}
	encoded_data = data.local8Bit ();
	return encoded_data;
}

void TwinTable::pasteEncoded (QByteArray content) {
	flushEdit ();
	
	TwinTableMember *table = activeTable ();
	if (!table) return;

	QValueList<RKVariable*> col_list;
	QTableSelection selection;
	if (table->numSelections () <= 0) {
		if ((table->currentRow () < 0) || (table->currentColumn () < 0)) return;
		selection.init (table->currentRow (), table->currentColumn ());
		selection.expandTo (table->currentRow (), table->currentColumn ());
		table->addSelection (selection);
	}
	// Unfortunately, selections added via addSelection () don't get "current".
	// So for this case, we have to set it explicitely --- what did I mean to say with that comment?
	if (table->currentSelection () >= 0) {
		selection = table->selection (table->currentSelection ());
	} else {
		selection = table->selection (0);
	}

	QString pasted = content;

	int row=selection.topRow ();
	int col=selection.leftCol ();
	while (pasted.length ()) {
		int next_tab = pasted.find ("\t");	
		if (next_tab < 0) next_tab = pasted.length ();
		int next_delim = next_tab;
		int next_line = pasted.find ("\n");
		if (next_line < 0) next_line = pasted.length ();
		if (next_line < next_tab) {
			next_delim = next_line;
		}
		if (!col_list.contains (getColObject (col))) {
			col_list.append (getColObject (col));
			getColObject (col)->setSyncing (false);
		}
		table->setText (row, col, pasted.left (next_delim));
		if (next_delim == next_tab) {
			col++;
			if (paste_mode == RKEditor::PasteToSelection) {
				if (col > selection.rightCol ()) {
					next_delim = next_line;
					row++;
					col = selection.leftCol ();
					if (row > selection.bottomRow ()) {
						next_delim = pasted.length ();
					}
				}
			}
			if (col >= table->numTrueCols ()) {
				if (paste_mode == RKEditor::PasteToTable) {
					next_delim = next_line;					
					row++;
					col = selection.leftCol ();
					if (row >= table->numRows ()) {
						next_delim = pasted.length ();
					}
				} else {
					// the if below only fails, if this is the last line.
					// We don't want a new column, then.
					// Everything else does not get affected in this situation.
					if (next_delim != next_line) {
						insertNewColumn ();
					}
				}
			}
		} else {
			row++;
			col=selection.leftCol ();
			if (paste_mode == RKEditor::PasteToSelection) {
				if (row > selection.bottomRow ()) {
					next_delim = pasted.length ();
				}
			}
			if (row >= table->numRows ()) {
				if (paste_mode == RKEditor::PasteToTable) {
					next_delim = pasted.length ();
				} else {
					if (next_delim != (pasted.length () -1)) {
						insertNewRow (-1, table);
					}
				}
			}
		}

		// proceed to the next segment
		if (pasted.length () <= (next_delim + 1)) {
			// unfortunately QString.right (<=0) does not return an empty string!
			pasted = "";
		} else {
			pasted=pasted.right (pasted.length () - (next_delim + 1));
		}
	}
	
	for (QValueList<RKVariable*>::ConstIterator it = col_list.constBegin (); it != col_list.constEnd (); ++it) {
		(*it)->syncDataToR ();
		(*it)->setSyncing (true);
	}
}

TwinTableMember *TwinTable::activeTable () {
	if (varview->hasFocus ()) {
		return varview;
	} else if (dataview->hasFocus ()) {
		return dataview;
	} else {
		return 0;
	}
}

void TwinTable::clearSelected () {
	QTable *table = activeTable ();
 	if (!table) return;

	QTableSelection selection;
	selection = table->selection (table->currentSelection ());

	for (int row=selection.topRow (); row <= selection.bottomRow (); row++) {
		for (int col=selection.leftCol (); col <= selection.rightCol (); col++) {
			table->clearCell (row, col);
		}
	}
}

void TwinTable::setPasteMode (RKEditor::PasteMode mode) {
	paste_mode = mode;
}

void TwinTable::setRow (TwinTableMember* table, int row, int start_col, int end_col, char **data) {
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
	flushEdit ();
	while (table->numRows () <= end_row) {
		insertNewRow (table->numRows (), table);
	}
	
	int i=0;
	for (int row=start_row; row <= end_row; ++row) {
		table->setText (row, col, data[i++]);
	}
}

void TwinTable::flushEdit () {
	// flush pending edit operations
	varview->stopEditing ();
	dataview->stopEditing ();
}

int TwinTable::numTrueCols () {
	return varview->numTrueCols ();
}

void TwinTable::setColObject (long int column, RKVariable *object) {
	RK_TRACE (EDITOR);
	if (object) {
		col_map.insert (column, object);
	} else {
		col_map.take (column);
	}
}

RKVariable *TwinTable::getColObject (long int col) {
	// do not trace. called very often
	//RK_TRACE (EDITOR);
	return col_map.find (col);
}

long int TwinTable::getObjectCol (RObject *object) {
	RK_TRACE (EDITOR);
	for (QIntDictIterator<RKVariable> it (col_map); it.current (); ++it) {
		if (it.current () == object) return it.currentKey ();
	}
	
	RK_ASSERT (false);
	return -1;
}
