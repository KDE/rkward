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

#include "twintablemember.h"
#include "typeselectcell.h"
#include "rtableitem.h"
#include "nameselectcell.h"
#include "labelcell.h"
#include "rkdrag.h"

TwinTable::TwinTable (QWidget *parent) : RKEditor (parent){
	QGridLayout *grid_layout = new QGridLayout(this);

    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(QSplitter::Vertical);

    varview = new TwinTableMember (splitter);
    varview->setNumRows (5);
    varview->setNumCols (5);
	for (int i=0; i < varview->numCols (); i++) {
		for (int j=0; j < varview->numRows (); j++) {
			if (j == TYPE_ROW) {
				varview->setItem (TYPE_ROW, i, new TypeSelectCell (varview));
			} else if (j == NAME_ROW) {
				varview->setItem (NAME_ROW, i, new NameSelectCell (varview));
				((NameSelectCell *) varview->item (NAME_ROW, i))->init ();
			} else if (j == LABEL_ROW) {
				varview->setItem (LABEL_ROW, i, new LabelCell (varview));
			} else {
				varview->setItem (j, i, new RTableItem (varview));
			}
		}
	}
	varview->verticalHeader()->setLabel(0, i18n( "Label" ) );
	varview->verticalHeader()->setLabel(TYPE_ROW, i18n( "Type" ) );
	varview->verticalHeader()->setLabel(2, i18n( "e.g. format" ) );
	varview->verticalHeader()->setLabel(3, i18n( "e.g. category" ) );
	varview->verticalHeader()->setLabel(NAME_ROW, i18n( "Name" ) );
	varview->setMinimumHeight (varview->horizontalHeader ()->height ());
	varview->setMaximumHeight (varview->rowPos (NAME_ROW) + varview->rowHeight (NAME_ROW) + varview->horizontalHeader ()->height () + 3);
    varview->verticalHeader()->setResizeEnabled (false);

    dataview = new TwinTableMember (splitter);
    dataview->setNumRows (20);
	dataview->setNumCols (5);
	dataview->setVarTable (varview);	// needed for initialization of RTableItems
	for (int i=0; i < dataview->numCols (); i++) {
		for (int j=0; j < dataview->numRows (); j++) {
			RTableItem *rti;
			dataview->setItem (j, i, rti = new RTableItem (dataview));
			rti->checkValid ();
		}
	}
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
	top_header_menu->insertItem (i18n ("Insert new variable left"), this, SLOT (insertColumnAfter ()));
	top_header_menu->insertItem (i18n ("Insert new variable right"), this, SLOT (insertColumnBefore ()));
	top_header_menu->insertItem (i18n ("Delete this variable"), this, SLOT (deleteColumn ()));

	// and the same for the left header
	connect (dataview, SIGNAL (headerRightClick (int, int)), this, SLOT (headerRightClicked (int, int)));
	left_header_menu = new QPopupMenu (this);
	left_header_menu->insertItem (i18n ("Insert new case above"), this, SLOT (insertRowAfter ()));
	left_header_menu->insertItem (i18n ("Insert new case below"), this, SLOT (insertRowBefore ()));
	
	qDebug ("Twintable created");
}

TwinTable::~TwinTable(){
	delete top_header_menu;
	delete left_header_menu;
}

void TwinTable::scrolled (int x, int y) {
	disconnect (varview, SIGNAL (contentsMoving (int, int)), this, SLOT (autoScrolled (int, int)));
	varview->setContentsPos (x, varview->contentsY ());
	connect (varview, SIGNAL (contentsMoving (int, int)), this, SLOT (autoScrolled (int, int)));
}

void TwinTable::autoScrolled (int x, int y) {
	disconnect (dataview, SIGNAL (contentsMoving (int, int)), this, SLOT (scrolled (int, int)));
	dataview->setContentsPos (x, dataview->contentsY ());
	connect (dataview, SIGNAL (contentsMoving (int, int)), this, SLOT (scrolled (int, int)));
}

void TwinTable::deleteColumn (int column) {
	if ((column >= 0) && (column < numCols ())) {
		emit (aboutToDeleteColumn (column));
		varview->removeColumn (column);
		dataview->removeColumn (column);
	}
}

void TwinTable::insertNewColumn (int where, QString name) {
	if ((where < 0) || (where > varview->numCols ())) {
		where = varview->numCols ();
	}

	varview->insertColumns (where);
	for (int i=0; i < varview->numRows (); i++) {
		if (i == TYPE_ROW) {
			varview->setItem (TYPE_ROW, where, new TypeSelectCell (varview));
		} else if (i == NAME_ROW) {
			varview->setItem (NAME_ROW, where, new NameSelectCell (varview));
			((NameSelectCell *) varview->item (NAME_ROW, where))->init ();
		} else if (i == LABEL_ROW) {
			varview->setItem (LABEL_ROW, i, new LabelCell (varview));
		} else {
			varview->setItem (i, where, new RTableItem (varview));
		}
	}
	dataview->insertColumns (where);
	for (int i=0; i < dataview->numRows (); i++) {
		RTableItem *rti;
		dataview->setItem (i, where, rti = new RTableItem (dataview));
		rti->checkValid ();
	}

	if (name != "") {
		varview->setText (NAME_ROW, where, name);
	}
}

void TwinTable::insertNewRow (int where, TwinTableMember *table) {
	if (!table) table = dataview;
	
	if ((where < 0) || (where > table->numRows ())) {
		where = table->numRows ();
	}

	table->insertRows (where);

	// initialize cells according to table
	if (table == dataview) {
		for (int i=0; i < dataview->numCols (); i++) {
			RTableItem *rti;
			dataview->setItem (where, i, rti = new RTableItem (dataview));
			rti->checkValid ();
		}
	} else if (table == varview) {
		for (int i=0; i <= varview->numCols (); i++) {
			if (where == TYPE_ROW) {
				varview->setItem (where, i, new TypeSelectCell (varview));
			} else if (where == NAME_ROW) {
				varview->setItem (where, i, new NameSelectCell (varview));
				((NameSelectCell *) varview->item (where, i))->init ();
			} else if (where == LABEL_ROW) {
				varview->setItem (where, i, new LabelCell (varview));
			} else {
				varview->setItem (where, i, new RTableItem (varview));
			}
		}
	}
}

void TwinTable::headerClicked (int col) {
	QTableSelection selection;
	selection.init (0, col);
	selection.expandTo (dataview->numRows (), col);

	dataview->addSelection (selection);
}

void TwinTable::headerRightClicked (int row, int col) {
	if (col >= 0) {
		if (col < varview->numCols ()) {
			header_pos = col;
			top_header_menu->popup (varview->mouse_at);
		}
	} else if (row >= 0) {
		if (row < dataview->numRows ()) {
			header_pos = row;
			left_header_menu->popup (dataview->mouse_at);
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

void TwinTable::insertColumnAfter () {
	insertNewColumn (header_pos+1);
}

void TwinTable::insertColumnBefore () {
	insertNewColumn (header_pos);
}

void TwinTable::deleteColumn () {
	deleteColumn (header_pos);
}

void TwinTable::insertRowAfter () {
	insertNewRow (header_pos+1);
}

void TwinTable::insertRowBefore () {
	insertNewRow (header_pos);
}

QCString TwinTable::encodeSelection () {
	QCString encoded_data;
	QTable *table = activeTable ();
	if (!table) {
		// no selection -> return empty
		return encoded_data;
	}

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

	QTableSelection selection;
	// Unfortunately, selections added via addSelection () don't get "current".
	// So for this case, we have to set it explicitely
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
			if (col >= table->numCols ()) {
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
}

void TwinTable::pasteEncodedFlipped (QByteArray content) {
	// this function mostly duplicates the code of the above, and the two
	// should really be merged one day!

	flushEdit ();
	TwinTableMember *table = activeTable ();
	if (!table) return;

	QTableSelection selection;
	// Unfortunately, selections added via addSelection () don't get "current".
	// So for this case, we have to set it explicitely
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
		table->setText (row, col, pasted.left (next_delim));
		if (next_delim == next_tab) {
			row++;
			if (paste_mode == RKEditor::PasteToSelection) {
				if (row > selection.bottomRow ()) {
					next_delim = next_line;
					col++;
					row = selection.topRow ();
					if (col > selection.rightCol ()) {
						next_delim = pasted.length ();
					}
				}
			}
			if (row >= table->numRows ()) {
				if (paste_mode == RKEditor::PasteToTable) {
					next_delim = next_line;
					col++;
					row = selection.topRow ();
					if (col >= table->numCols ()) {
						next_delim = pasted.length ();
					}
				} else {
					// the if below only fails, if this is the last line.
					// We don't want a new column, then.
					// Everything else does not get affected in this situation.
					if (next_delim != next_line) {
						insertNewRow (-1, table);
					}
				}
			}

		} else {
			col++;
			row=selection.topRow ();
			if (paste_mode == RKEditor::PasteToSelection) {
				if (col > selection.rightCol ()) {
					next_delim = pasted.length ();
				}
			}
			if (col >= table->numCols ()) {
				if (paste_mode == RKEditor::PasteToTable) {
					next_delim = pasted.length ();
				} else {
					// the if below only fails, if this is the last line.
					// We don't want a new column, then.
					// Everything else does not get affected in this situation.
					if (next_delim != (pasted.length () -1)) {
						insertNewColumn ();
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
}

TwinTableMember *TwinTable::activeTable () {
	if (varview->numSelections ()) {
		return varview;
	} else if (dataview->numSelections ()) {
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
	while (numCols () <= end_col) {
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
	varview->endEdit (varview->currentRow (), varview->currentColumn (), true, false);
	dataview->endEdit (dataview->currentRow (), dataview->currentColumn (), true, false);
}

/*QString TwinTable::varname (int col) {
	return varview->item (NAME_ROW, col)->text ();
}

QString TwinTable::label (int col) {
	return varview->item (LABEL_ROW, col)->text ();
}

QString TwinTable::typeString (int col) {
	return varview->item (TYPE_ROW, col)->text ();
}*/

int TwinTable::numCols () {
	return varview->numCols ();
}

/*int TwinTable::lookUp (const QString &name) {
	int return_val = -1;
	for (unsigned int i = 0; i < numCols (); i++) {
		// that section thing is an ugly hack only for the time being,
		// that sorts out the varname from rk.data["varname"]
		if (varname (i) == name.section ("\"", 1, 1)) {
			return_val = i;
			i = numCols ();
		}
	}

	return return_val;	
}*/

