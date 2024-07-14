/*
twintable.h - This file is part of the RKWard project. Created: Tue Oct 29 2002
SPDX-FileCopyrightText: 2002-2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
class QAction;
class QSplitter;
class KMessageWidget;

/**
  *@author Thomas Friedrichsmeier
  */

class TwinTable : public RKEditor, public RObjectListener, public KXMLGUIClient {
	Q_OBJECT
public: 
	explicit TwinTable(QWidget *parent=nullptr);
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
	void setWindowStyleHint (const QString& hint) override;
public Q_SLOTS:
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
/** connected to RKVarEditModel::hasProblems() */
	void hasProblems();
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

	QSplitter *splitter;
protected:	
/** Returns the active Table (of the two members), 0 if no table active */
	TwinTableMember *activeTable ();

	KMessageWidget* problems_note;
	TwinTableMember* metaview;
	TwinTableMember* dataview;

	QAction* action_insert_col_left;
	QAction* action_delete_col;
	QAction* action_insert_row_above;
	QAction* action_delete_row;
	QAction* action_delete_rows;
	QAction* action_enable_editing;
	QAction* action_tb_lock_editing;
	QAction* action_tb_unlock_editing;
	QAction* action_show_rownames;
	QAction* editCut;
	QAction* editCopy;
	QAction* editPaste;
	QAction* editPasteToSelection;
	QAction* editPasteToTable;

	QActionGroup* edit_actions;

/** receives object meta change notifications. This updates the caption */
	void objectMetaChanged (RObject* changed) override;

	void initActions ();

	RObject* main_object;
private Q_SLOTS:
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
