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

#define TYPE_ROW 1

TwinTable::TwinTable(QWidget *parent, const char *name) : QWidget (parent, name){
    resize( 600, 480 );
    setCaption( i18n( "Form2" ) );

	grid_layout = new QGridLayout( this );

    Splitter1 = new QSplitter( this, "Splitter1" );
    Splitter1->setOrientation( QSplitter::Vertical );

    varview = new TwinTableMember( Splitter1, "varview" );
    varview->setNumRows( 5 );
    varview->setNumCols( 5 );
	varview->horizontalHeader()->setLabel(0, i18n( "new_Variable" ) );
	varview->verticalHeader()->setLabel(0, i18n( "Label" ) );
	varview->verticalHeader()->setLabel(TYPE_ROW, i18n( "Type" ) );
	for (int i=0; i < varview->numCols (); i++) {
		varview->setItem (TYPE_ROW, i, new TypeSelectCell (varview));
	}
	varview->verticalHeader()->setLabel(2, i18n( "something" ) );
	varview->verticalHeader()->setLabel(3, i18n( "4" ) );
	varview->verticalHeader()->setLabel(4, i18n( "__________" ) );
	varview->setMinimumHeight (varview->horizontalHeader ()->height ());
    varview->verticalHeader()->setResizeEnabled (false);

    dataview = new TwinTableMember( Splitter1, "dataview" );
    dataview->setNumRows( 20 );
	dataview->setNumCols( 5 );
    dataview->verticalHeader()->setLabel(0, i18n ( "new Case" ));
    dataview->verticalHeader()->setResizeEnabled (false);

	dataview->horizontalHeader ()->hide ();
	dataview->setTopMargin (0);
	dataview->setLeftMargin (varview->leftMargin ());
	varview->setHScrollBarMode (QScrollView::AlwaysOff);

    grid_layout->addWidget (Splitter1, 0, 0);
	
	// these are to keep the two tables in sync
	varview->setTwin (dataview);
	dataview->setTwin (varview);
	connect (dataview, SIGNAL (contentsMoving (int, int)), this, SLOT (scrolled (int, int)));
	connect (varview, SIGNAL (contentsMoving (int, int)), this, SLOT (autoScrolled (int, int)));
	connect (varview->horizontalHeader (), SIGNAL (clicked (int)), dataview, SLOT (columnClicked (int)));
	connect (varview->horizontalHeader (), SIGNAL (clicked (int)), this, SLOT (headerClicked (int)));
	connect (varview, SIGNAL (selectionChanged ()), this, SLOT (dataClearSelection ()));
	connect (dataview, SIGNAL (selectionChanged ()), this, SLOT (viewClearSelection ()));

	// this is to catch right-clicks on the header
	connect (varview, SIGNAL (headerRightClick (int)), this, SLOT (headerRightClicked (int)));

	// which will be reacted upon by the following popup-menu:
	header_menu = new QPopupMenu (this);
	header_menu->insertItem (i18n ("Insert new variable after"), this, SLOT (insertColumnAfter ()));
	header_menu->insertItem (i18n ("Insert new variable before"), this, SLOT (insertColumnBefore ()));
}

TwinTable::~TwinTable(){
	delete header_menu;
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

void TwinTable::insertNewColumn (int where, QString name) {
	if ((where < 0) || (where > varview->numCols ())) {
		where = varview->numCols ();
	}
	if (name == "") {
		name = "new var";
	}

	varview->insertColumns (where);
	varview->setItem (TYPE_ROW, where, new TypeSelectCell (varview));
	dataview->insertColumns (where);

	varview->horizontalHeader()->setLabel(where, name);
}

void TwinTable::insertNewRow (int where=-1) {
	if ((where < 0) || (where > dataview->numRows ())) {
		where = dataview->numRows ();
	}

	dataview->insertRows (where);
}

void TwinTable::headerClicked (int col) {
	QTableSelection selection;
	selection.init (0, col);
	selection.expandTo (dataview->numRows (), col);

	dataview->addSelection (selection);
}

void TwinTable::headerRightClicked (int col) {
	if (col < varview->numCols ()) {
		header_pos = col;
		header_menu->popup (varview->mouse_at);
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

void TwinTable::insertColumnAfter () {
	insertNewColumn (header_pos+1);
}

void TwinTable::insertColumnBefore () {
	insertNewColumn (header_pos);
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
	QTable *table = activeTable ();
	if (!table) return;

	QTableSelection selection;
	selection = table->selection (table->currentSelection ());

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
			if (paste_mode == PasteToSelection) {
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
				if (paste_mode == PasteToTable) {
					next_delim = next_line;					
					row++;
					col = selection.leftCol ();
					if (row >= table->numRows ()) {
						next_delim = pasted.length ();
					}
				} else {
					// the if only fails, if this is the last line.
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
			if (paste_mode == PasteToSelection) {
				if (row > selection.bottomRow ()) {
					next_delim = pasted.length ();
				}
			}
			if (row >= table->numRows ()) {
				if (paste_mode == PasteToTable) {
					next_delim = pasted.length ();
				} else {
					insertNewRow ();
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

QTable *TwinTable::activeTable () {
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

void TwinTable::setPasteMode (PasteMode mode) {
	paste_mode = mode;
}
