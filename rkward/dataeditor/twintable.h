/***************************************************************************
                          twintable.h  -  description
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

#ifndef TWINTABLE_H
#define TWINTABLE_H

#include "rkeditor.h"

#include <qstring.h>
#include <QItemSelection>

#include "../core/rkmodificationtracker.h"

class TwinTableMember;
class QMenu;
class RKVarEditModel;
class QActionGroup;

/**
  *@author Thomas Friedrichsmeier
  */

class TwinTable : public RKEditor, public RObjectListener {
	Q_OBJECT
public: 
	TwinTable (QWidget *parent=0);
	~TwinTable ();
/** Pastes clipboard content to the current table */
	void paste (RKEditor::PasteMode paste_mode);
/** Copy selection in the current table to clipboard */
	void copy ();
/** Same as above, but flips the data (i.e. row <-> cols) */
//	void pasteEncodedFlipped (QByteArray content);
/** Clear the currently selected cells */
	void clearSelected ();
	QString getSelectedText ();

/** Flushes pending edit-operations */
	void flushEdit ();

	void initTable (RKVarEditModel* model, RObject* object);
	
	RKVarEditModel* datamodel;

	QActionGroup* editActions () const { return edit_actions; };
public slots:
	void dataHeaderContextMenu (int row, int col, const QPoint& pos);
	void metaHeaderContextMenu (int row, int col, const QPoint& pos);

	void metaHeaderPressed (int section);
	void metaHeaderEntered (int section);
	void metaHeaderClicked (int section);

	void enableEditing (bool on);
private:
/** PopupMenu shown when top header is right-clicked */
	QMenu *top_header_menu;
/** PopupMenu shown when top header is right-clicked */
	QMenu *left_header_menu;

	int meta_header_anchor_section;
/** read-write */
	bool rw;
protected:	
/** Returns the active Table (of the two members), 0 if no table active */
	TwinTableMember *activeTable ();

	TwinTableMember* metaview;
	TwinTableMember* dataview;

	QAction* action_insert_col_left;
	QAction* action_insert_col_right;
	QAction* action_delete_col;

	QAction* action_insert_row_above;
	QAction* action_insert_row_below;
	QAction* action_delete_row;
	QAction* action_delete_rows;

	QAction* action_enable_editing;

	QActionGroup* edit_actions;

/** receives object meta change notifications. This updates the caption */
	void objectMetaChanged (RObject* changed);

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
};

#endif
