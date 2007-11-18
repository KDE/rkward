/***************************************************************************
                          twintable.cpp  -  description
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

#include "twintable.h"

#include <klocale.h>

#include <qvariant.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <QMenu>
#include <QVBoxLayout>
#include <QHeaderView>

#include "twintablemember.h"
#include "rkvareditmodel.h"
#include "../core/rcontainerobject.h"
#include "../misc/rkstandardicons.h"

#include "../debug.h"

TwinTable::TwinTable (QWidget *parent) : RKEditor (parent), RObjectListener (RObjectListener::Other) {
	RK_TRACE (EDITOR);

	main_object = 0;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins (0, 0, 0, 0);
	
	QSplitter *splitter = new QSplitter(this);
	splitter->setOrientation(Qt::Vertical);

	metaview = new TwinTableMember (splitter);
	splitter->setStretchFactor (splitter->indexOf (metaview), 0);
	metaview->verticalHeader ()->setResizeMode (QHeaderView::Fixed);
	metaview->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
	
	dataview = new TwinTableMember (splitter);
	splitter->setStretchFactor (splitter->indexOf (dataview), 1);
	dataview->verticalHeader ()->setResizeMode (QHeaderView::Fixed);
	dataview->horizontalHeader ()->hide ();

	layout->addWidget (splitter);

	// these are to keep the two tables in sync
	metaview->setTwin (dataview);
	dataview->setTwin (metaview);

	// pressing the columns in the metaview-header should select columns in the dataview
	disconnect (metaview->horizontalHeader (), SIGNAL (sectionClicked(int)));
	connect (metaview->horizontalHeader (), SIGNAL (sectionClicked(int)), this, SLOT (metaHeaderClicked(int)));
	disconnect (metaview->horizontalHeader (), SIGNAL (sectionPressed(int)));
	connect (metaview->horizontalHeader (), SIGNAL (sectionPressed(int)), this, SLOT (metaHeaderPressed(int)));
	disconnect (metaview->horizontalHeader (), SIGNAL (sectionEntered(int)));
	connect (metaview->horizontalHeader (), SIGNAL (sectionEntered(int)), this, SLOT (metaHeaderEntered(int)));
	meta_header_anchor_section = -1;

	// catch header context menu requests
	connect (dataview, SIGNAL (contextMenuRequest(int,int,const QPoint&)), this, SLOT (dataHeaderContextMenu(int,int,const QPoint&)));
	connect (metaview, SIGNAL (contextMenuRequest(int,int,const QPoint&)), this, SLOT (metaHeaderContextMenu(int,int,const QPoint&)));
	
	// which will be reacted upon by the following popup-menus:
	top_header_menu = new QMenu (this);
	action_insert_col_left = top_header_menu->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionInsertVar), QString (), this, SLOT (insertColumn()));
	action_insert_col_right = top_header_menu->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionInsertVar), QString (), this, SLOT (insertColumn()));
	action_delete_col = top_header_menu->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteVar), QString (), this, SLOT (deleteColumn()));
	
	left_header_menu = new QMenu (this);
	action_insert_row_above = left_header_menu->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionInsertRow), QString (), this, SLOT (insertRow()));
	action_insert_row_below = left_header_menu->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionInsertRow), QString (), this, SLOT (insertRow()));
	action_delete_row = left_header_menu->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteRow), QString (), this, SLOT (deleteRow()));
	action_delete_rows = left_header_menu->addAction (RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteRow), QString (), this, SLOT (deleteSelectedRows()));

	setFocusPolicy (Qt::StrongFocus);
}

TwinTable::~TwinTable() {
	RK_TRACE (EDITOR);

	RK_ASSERT (main_object);
	stopListenForObject (main_object);
// TODO: are the models auto-destructed?
}

void TwinTable::initTable (RKVarEditModel* model, RObject* object) {
	RK_TRACE (EDITOR);

	datamodel = model;
	main_object = object;
	dataview->setRKModel (model);
	metaview->setRKModel (model->getMetaModel ());
	model->setEditor (this);
	dataview->seRKItemDelegate (new RKItemDelegate (this, datamodel));
	metaview->seRKItemDelegate (new RKItemDelegate (this, datamodel->getMetaModel ()));

	metaview->setMinimumHeight (metaview->horizontalHeader ()->height ());
	metaview->setMaximumHeight (metaview->rowHeight (0) * 5 + metaview->horizontalHeader ()->height () + 5);
	dataview->verticalHeader ()->setFixedWidth (metaview->verticalHeader ()->width ());

// init caption
	addNotificationType (RObjectListener::MetaChanged);
	listenForObject (object);
	objectMetaChanged (object);
}

void TwinTable::objectMetaChanged (RObject* changed) {
	RK_TRACE (EDITOR);

	RK_ASSERT (changed == main_object);
	setCaption (main_object->getShortName ());
}

void TwinTable::metaHeaderPressed (int section) {
	RK_TRACE (EDITOR);

	if (meta_header_anchor_section < 0) {
		meta_header_anchor_section = section;
		dataview->selectColumn (section);
	}
	dataview->setFocus ();
}

void TwinTable::metaHeaderClicked (int) {
	RK_TRACE (EDITOR);

	RK_ASSERT (meta_header_anchor_section >= 0);
	meta_header_anchor_section = -1;
	dataview->setFocus ();
}

void TwinTable::metaHeaderEntered (int section) {
	RK_TRACE (EDITOR);

	if (meta_header_anchor_section >= 0) {
		dataview->selectionModel ()->select (QItemSelection (datamodel->index (0, qMin (meta_header_anchor_section, section)), datamodel->index (0, qMax (meta_header_anchor_section, section))), QItemSelectionModel::Columns | QItemSelectionModel::Select);
		dataview->setFocus ();
	}
}

// TODO: handle situation when several entire cols are selected!
void TwinTable::metaHeaderContextMenu (int row, int col, const QPoint& pos) {
	RK_TRACE (EDITOR);

	if (col >= 0) {
		RK_ASSERT (row == -1);
		RK_ASSERT (col <= datamodel->trueCols ());

		action_insert_col_left->setVisible (true);
		action_insert_col_left->setText (i18n ("Insert new variable left"));	// TODO: show name
		action_insert_col_left->setData (col);
		action_insert_col_right->setVisible (col < datamodel->trueCols ());
		action_insert_col_right->setText (i18n ("Insert new variable right"));	// TODO: show name
		action_insert_col_right->setData (col+1);
		action_delete_col->setVisible (col < datamodel->trueCols ());
		action_delete_col->setText (i18n ("Delete this variable"));	// TODO: show name
		action_delete_col->setData (col);

		top_header_menu->popup (pos);
	}
}

void TwinTable::dataHeaderContextMenu (int row, int col, const QPoint& pos) {
	RK_TRACE (EDITOR);

	RK_ASSERT (col < 0);
	if (row >= 0) {
		RK_ASSERT (row <= datamodel->trueRows ());

		action_insert_row_above->setVisible (true);
		action_insert_row_above->setText (i18n ("Insert new case above (at %1)", row + 1));
		action_insert_row_above->setData (row);
		action_insert_row_below->setVisible (row < datamodel->trueRows ());
		action_insert_row_below->setText (i18n ("Insert new case below (at %1)", row + 2));
		action_insert_row_below->setData (row + 1);

		QItemSelectionRange sel = dataview->getSelectionBoundaries ();
		if (sel.isValid ()) {
			int top = sel.top ();
			int bottom = sel.bottom ();
			if (bottom >= datamodel->trueRows ()) bottom = datamodel->trueRows () - 1;
			if (top > bottom) top = bottom;

			action_delete_rows->setVisible (true);
			action_delete_rows->setText (i18n ("Delete marked rows (%1-%2)", (top+1), (bottom+1)));
		} else {
			action_delete_rows->setVisible (false);
		}

		action_delete_row->setVisible (row < datamodel->trueRows ());
		action_delete_row->setText (i18n ("Delete this row (%1)", (row+1)));
		action_delete_row->setData (row);

		left_header_menu->popup (pos);
	}
}

void TwinTable::deleteColumn () {
	RK_TRACE (EDITOR);

	QObject *s = sender ();
	int col;
	if (s == action_delete_col) col = action_delete_col->data ().toInt ();
	else {
		RK_ASSERT (false);
		return;
	}

	flushEdit ();

	datamodel->removeColumns (col, 1);
}

void TwinTable::insertColumn () {
	RK_TRACE (EDITOR);

	QObject *s = sender ();
	int where;
	if (s == action_insert_col_left) where = action_insert_col_left->data ().toInt ();
	else if (s == action_insert_col_right) where = action_insert_col_right->data ().toInt ();
	else {
		RK_ASSERT (false);
		return;
	}

	flushEdit ();

	datamodel->insertColumns (where, 1);
}

void TwinTable::deleteRow () {
	RK_TRACE (EDITOR);

	QObject *s = sender ();
	int where;
	if (s == action_delete_row) where = action_delete_row->data ().toInt ();
	else {
		RK_ASSERT (false);
		return;
	}

	flushEdit ();

	datamodel->removeRows (where, 1);
}

void TwinTable::deleteSelectedRows () {
	RK_TRACE (EDITOR);

	QItemSelectionRange sel = dataview->getSelectionBoundaries ();
	if (sel.isValid ()) {
		int top = sel.top ();
		int bottom = sel.bottom ();
		if (bottom >= datamodel->trueRows ()) bottom = datamodel->trueRows () - 1;
		if (top > bottom) top = bottom;

		datamodel->removeRows (top, bottom - top + 1);
	} else {
		RK_ASSERT (false);
	}
}

void TwinTable::insertRow () {
	RK_TRACE (EDITOR);

	QObject *s = sender ();
	int where;
	if (s == action_insert_row_above) where = action_insert_row_above->data ().toInt ();
	else if (s == action_insert_row_below) where = action_insert_row_below->data ().toInt ();
	else {
		RK_ASSERT (false);
		return;
	}

	flushEdit ();

	datamodel->insertRows (where, 1);
}

void TwinTable::copy () {
	RK_TRACE (EDITOR);

	flushEdit ();
	TwinTableMember *table = activeTable ();
	if (!table) return;
	table->copy ();
}

void TwinTable::paste (RKEditor::PasteMode paste_mode) {
	RK_TRACE (EDITOR);

	flushEdit ();
	
	TwinTableMember *table = activeTable ();
	if (!table) return;

	table->paste (paste_mode);
}

TwinTableMember *TwinTable::activeTable () {
	RK_TRACE (EDITOR);

	if (metaview->hasFocus ()) {
		return metaview;
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

void TwinTable::flushEdit () {
	RK_TRACE (EDITOR);

	// flush pending edit operations
	metaview->stopEditing ();
	dataview->stopEditing ();
}

#include "twintable.moc"
