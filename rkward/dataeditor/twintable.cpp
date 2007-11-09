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

#include "../debug.h"

TwinTable::TwinTable (QWidget *parent) : RKEditor (parent) {
	RK_TRACE (EDITOR);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins (0, 0, 0, 0);
	
	QSplitter *splitter = new QSplitter(this);
	splitter->setOrientation(Qt::Vertical);

	metaview = new TwinTableMember (splitter, this);
	splitter->setResizeMode (metaview, QSplitter::KeepSize);
	metaview->verticalHeader ()->setResizeMode (QHeaderView::Fixed);

#warning TODO set item delegates
	
	dataview = new TwinTableMember (splitter, this);
	dataview->verticalHeader ()->setResizeMode (QHeaderView::Fixed);
	
	dataview->horizontalHeader ()->hide ();
#warning is this needed?
//	dataview->setTopMargin (0);
//	dataview->setLeftMargin (metaview->leftMargin ());
	metaview->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
	
	layout->addWidget (splitter);
	
	// these are to keep the two tables in sync
	metaview->setTwin (dataview);
	dataview->setTwin (metaview);
	connect (metaview->horizontalHeader (), SIGNAL (sectionClicked(int)), dataview, SLOT (selectColumn(int)));
	connect (metaview->horizontalHeader (), SIGNAL (sectionPressed(int)), dataview, SLOT (selectColumn(int)));
	connect (metaview, SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)), this, SLOT (metaSelectionChanged (const QItemSelection&, const QItemSelection&)));
	connect (dataview, SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)), this, SLOT (dataSelectionChanged (const QItemSelection&, const QItemSelection&)));
	
	// catch header context menu requests
	connect (dataview, SIGNAL (contextMenuRequest(int,int)), this, SLOT (dataHeaderContextMenu(int,int)));
	connect (metaview, SIGNAL (contextMenuRequest(int,int)), this, SLOT (metaHeaderContextMenu(int,int)));
	
	// which will be reacted upon by the following popup-menu:
	top_header_menu = new QMenu (this);
	action_insert_col_left = top_header_menu->addAction (QString (), this, SLOT (insertColumn()));
	action_insert_col_right = top_header_menu->addAction (QString (), this, SLOT (insertColumn()));
	action_delete_col = top_header_menu->addAction (QString (), this, SLOT (deleteColumn()));
	
	left_header_menu = new QMenu (this);
	action_insert_row_above = left_header_menu->addAction (QString (), this, SLOT (insertRow()));
	action_insert_row_below = left_header_menu->addAction (QString (), this, SLOT (insertRow()));
	action_delete_row = left_header_menu->addAction (QString (), this, SLOT (deleteRow()));
	action_delete_rows = left_header_menu->addAction (QString (), this, SLOT (deleteSelectedRows()));

	setFocusPolicy (Qt::StrongFocus);
}

TwinTable::~TwinTable() {
	RK_TRACE (EDITOR);

// TODO: are the models auto-destructed?
}

void TwinTable::initTable (RKVarEditDataFrameModel* model) {
	RK_TRACE (EDITOR);

	datamodel = model;
	dataview->setModel (model);
	metaview->setModel (model->getMetaModel ());

	metaview->setMinimumHeight (metaview->horizontalHeader ()->height ());
	metaview->setMaximumHeight (metaview->rowHeight (0) * 5 + metaview->horizontalHeader ()->height () + 5);
	dataview->verticalHeader ()->setFixedWidth (metaview->verticalHeader ()->width ());

	setCaption (model->getObject ()->getShortName ());
}

// TODO: handle situation when several entire cols are selected!
void TwinTable::metaHeaderContextMenu (int row, int col) {
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

		// TODO: should be passed as a parameter, instead
		top_header_menu->popup (metaview->mouse_at);
	}
}

void TwinTable::dataHeaderContextMenu (int row, int col) {
	RK_TRACE (EDITOR);

	RK_ASSERT (col < 0);
	if (row >= 0) {
		RK_ASSERT (row <= datamodel->trueRows ());

		action_insert_row_above->setVisible (true);
		action_insert_row_above->setText (i18n ("Insert new case above (at %1)", col + 1));
		action_insert_row_above->setData (col);
		action_insert_row_below->setVisible (row < datamodel->trueRows ());
		action_insert_row_above->setText (i18n ("Insert new case below (at %1)", col + 2));
		action_insert_row_below->setData (col + 1);

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

		left_header_menu->popup (dataview->mouse_at);
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

void TwinTable::metaSelectionChanged (const QItemSelection& selected, const QItemSelection&) {
	RK_TRACE (EDITOR);

	if (!selected.isEmpty ()) dataview->clearSelection ();
}

void TwinTable::dataSelectionChanged (const QItemSelection& selected, const QItemSelection&) {
	RK_TRACE (EDITOR);

	if (!selected.isEmpty ()) metaview->clearSelection ();
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
