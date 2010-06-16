/***************************************************************************
                          twintable.h  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002, 2006, 2007, 2010 by Thomas Friedrichsmeier
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

#include <qstring.h>
#include <QItemSelection>

#include <kxmlguiclient.h>

#include "../core/rkmodificationtracker.h"

class TwinTableMember;
class RKVarEditModel;
class QActionGroup;
class KAction;

/**
  *@author Thomas Friedrichsmeier
  */

class TwinTable : public RKEditor, public RObjectListener, public KXMLGUIClient {
	Q_OBJECT
public: 
	TwinTable (QWidget *parent=0);
	~TwinTable ();
/** Pastes clipboard content to the current table */
	void paste (RKEditor::PasteMode paste_mode);
/** Clear the currently selected cells */
	void clearSelected ();

/** Flushes pending edit-operations */
	void flushEdit ();

	void initTable (RKVarEditModel* model, RObject* object);
	
	RKVarEditModel* datamodel;

	QActionGroup* editActions () const { return edit_actions; };
public slots:
	void metaHeaderPressed (int section);
	void metaHeaderEntered (int section);
	void metaHeaderClicked (int section);

	void enableEditing (bool on);
	void showRownames (bool show);

/** put the marked cells into the clipboard and remove them from the table */
	void cut();
/** Copy selection in the current table to clipboard */
	void copy();
/** paste the clipboard into the table, expanding the table as necessary */
	void paste();
/** paste the clipboard into the table, but not beyond table boundaries */
	void pasteToTable();
/** paste the clipboard into the table, but not beyond selection boundaries	*/
	void pasteToSelection();
private:
	int meta_header_anchor_section;
/** read-write */
	bool rw;

/** If currently in one of the context menus, this holds, which table the mouse was pressed over (or 0) */
	TwinTableMember* context_menu_table;
/** Only valid, if context_menu_table != 0. Row of current context menu event. -1 for header row. -2 for no cell. */
	int context_menu_row;
/** Only valid, if context_menu_table != 0. Column of current context menu event. -1 for header column. -2 for no cell. */
	int context_menu_column;
protected:	
/** Returns the active Table (of the two members), 0 if no table active */
	TwinTableMember *activeTable ();

	TwinTableMember* metaview;
	TwinTableMember* dataview;

	KAction* action_insert_col_left;
	KAction* action_delete_col;
	KAction* action_insert_row_above;
	KAction* action_delete_row;
	KAction* action_delete_rows;
	KAction* action_enable_editing;
	KAction* action_show_rownames;
	KAction* editCut;
	KAction* editCopy;
	KAction* editPaste;
	KAction* editPasteToSelection;
	KAction* editPasteToTable;

	QActionGroup* edit_actions;

/** receives object meta change notifications. This updates the caption */
	void objectMetaChanged (RObject* changed);

	void initActions ();

	RObject* main_object;
private slots:
/** inserts a new column (NOTE the action connected to this signal carries the info, where the column is to be inserted) */
	void insertColumn ();
/** inserts a new row in the dataview (NOTE the action connected to this signal carries the info, where the column is to be inserted) */
	void insertRow ();
/** deletes the current row (in the dataview) */
	void deleteRow ();
/** deletes all marked rows (in the dataview) */
	void deleteSelectedRows ();
/** deletes the column at the current header_pos. Actually it does not really delete the column, but requests object-removal from the model, which will pass the request to RKModifcationTracker */
	void deleteColumn ();
/** handle context menu requests from the two members */
	void contextMenu (int row, int col, const QPoint& pos);
};

#endif
