/***************************************************************************
                          twintable.h  -  description
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

#ifndef TWINTABLE_H
#define TWINTABLE_H

#include "rkeditor.h"

#include <qvariant.h>
#include <qstring.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSplitter;
class TwinTableMember;
class QPopupMenu;
class QTable;
class RKDrag;
class RObject;

/**
  *@author Thomas Friedrichsmeier
  */

class TwinTable : public RKEditor {
	Q_OBJECT
public: 
	TwinTable(QWidget *parent=0);
	~TwinTable();
	void insertNewColumn (int where=-1, QString name="");
	void insertNewRow (int where=-1, TwinTableMember *table=0);
	QCString encodeSelection ();
/** Pastes content to the current selection. */
	void pasteEncoded (QByteArray content);
/** Same as above, but flips the data (i.e. row <-> cols) */
	void pasteEncodedFlipped (QByteArray content);
/** Clear the currently selected cells */
	void clearSelected ();
	RKDrag *makeDrag ();

/** Flushes pending edit-operations */
	void flushEdit ();

/** Returns the variable-name of a column */
//	QString varname (int col);

/** Returns the variable-label of a column */
//	QString label (int col);

/** Returns the type of variable (as a string) of a column */
//	QString typeString (int col);

/** Returns the column of a variable named "name" or -1 if no such var */
//	int lookUp (const QString &name);

/** Returns the number of columns in the tables */
	int numCols ();
	
	void setPasteMode (RKEditor::PasteMode mode);
    TwinTableMember* varview;
    TwinTableMember* dataview;
public slots:
	void headerClicked (int col);
	void headerRightClicked (int row, int col);
	void viewClearSelection ();
	void dataClearSelection ();
private:
/** PopupMenu shown when top header is right-clicked */
	QPopupMenu *top_header_menu;
/** PopupMenu shown when top header is right-clicked */
	QPopupMenu *left_header_menu;
/** position (row or col) the header_menu is operating on */
	int header_pos;
/** Returns the active Table (of the two members), 0 if no table active */
	TwinTableMember *activeTable ();

	RKEditor::PasteMode paste_mode;
protected:	
/** set a row of cells, expanding the table if necessary. Assumes you provide the correct amount of data! */
	void setRow (TwinTableMember* table, int row, int start_col, int end_col, char **data);
/** set a column of cells, expanding the table if necessary. Assumes you provide the correct amount of data! */
	void setColumn (TwinTableMember* table, int col, int start_row, int end_row, char **data);
private slots:
	void scrolled (int x, int y);
	void autoScrolled (int x, int y);
/** inserts a new column after the current header_pos */
	void insertColumnAfter ();
/** inserts a new column before the current header_pos */
	void insertColumnBefore ();
/** inserts a new row after the current header_pos */
	void insertRowAfter ();
/** inserts a new row before the current header_pos */
	void insertRowBefore ();
};

#endif
