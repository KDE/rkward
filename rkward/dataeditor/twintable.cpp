/***************************************************************************
                          twintable.cpp  -  description
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

#include "twintable.h"

#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <kmessagebox.h>

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
#include "../rkward.h"

#include "../debug.h"

TwinTable::TwinTable (QWidget *parent) : RKEditor (parent), RObjectListener (RObjectListener::Other), KXMLGUIClient () {
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
	dataview->setAlternatingRowColors (true);

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
	connect (dataview, SIGNAL (contextMenuRequest(int,int,const QPoint&)), this, SLOT (contextMenu(int,int,const QPoint&)));
	connect (metaview, SIGNAL (contextMenuRequest(int,int,const QPoint&)), this, SLOT (contextMenu(int,int,const QPoint&)));
	context_menu_table = 0;
	context_menu_row = context_menu_column = -2;

	setXMLFile ("rkeditordataframepart.rc");
	initActions ();

	setFocusPolicy (Qt::StrongFocus);
}

TwinTable::~TwinTable() {
	RK_TRACE (EDITOR);

	RK_ASSERT (main_object);
	stopListenForObject (main_object);
// TODO: are the models auto-destructed?
}

void TwinTable::initActions () {
	RK_TRACE (EDITOR);

	editCut = actionCollection ()->addAction (KStandardAction::Cut, "cut", this, SLOT(cut()));
	editCut->setStatusTip (i18n("Cuts the selected section and puts it to the clipboard"));
	editCopy = actionCollection ()->addAction (KStandardAction::Copy, "copy", this, SLOT(copy()));
	editCopy->setStatusTip (i18n("Copies the selected section to the clipboard"));
//	editor->editActions ()->addAction (editCopy);	// this is a read-only action, not an "edit" action
	editPaste = actionCollection ()->addAction (KStandardAction::Paste, "paste", this, SLOT(paste()));
	editPaste->setStatusTip (i18n("Pastes the clipboard contents to current position"));

	editPasteToTable = actionCollection ()->addAction ("paste_to_table", this, SLOT(pasteToTable()));
	editPasteToTable->setText (i18n("Paste inside table"));
	editPasteToTable->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionPasteInsideTable));
	editPasteToTable->setStatusTip (i18n("Pastes the clipboard contents to current position, but not beyond the table's boundaries"));

	editPasteToSelection = actionCollection ()->addAction ("paste_to_selection", this, SLOT(pasteToSelection()));
	editPasteToSelection->setText (i18n("Paste inside selection"));
	editPasteToSelection->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionPasteInsideSelection));
	editPasteToSelection->setStatusTip (i18n("Pastes the clipboard contents to current position, but not beyond the boundaries of the current selection"));

	// header menus
	action_insert_col_left = actionCollection ()->addAction ("insert_col_left", this, SLOT (insertColumn()));
	action_insert_col_left->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionInsertVar));
	action_delete_col = actionCollection ()->addAction ("delete_col", this, SLOT (deleteColumn()));
	action_delete_col->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteVar));

	action_insert_row_above = actionCollection ()->addAction ("insert_row_above", this, SLOT (insertRow()));
	action_insert_row_above->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionInsertRow));
	action_delete_row = actionCollection ()->addAction ("delete_row", this, SLOT (deleteRow()));
	action_delete_row->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteRow));
	action_delete_rows = actionCollection ()->addAction ("delete_rows", this, SLOT (deleteSelectedRows()));
	action_delete_rows->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteRow));

	// global actions
	action_enable_editing = actionCollection ()->addAction ("enable_editing", this, SLOT (enableEditing(bool)));
	action_enable_editing->setText ("Enable editing");
	action_enable_editing->setCheckable (true);
	action_show_rownames = actionCollection ()->addAction ("show_rownames", this, SLOT (showRownames(bool)));
	action_show_rownames->setText ("Show / Edit row names");
	action_show_rownames->setCheckable (true);

	// add all edit-actions to a group, so they can be enabled/disabled easily
	edit_actions = new QActionGroup (this);
	edit_actions->addAction (editCut);
	edit_actions->addAction (editPaste);
	edit_actions->addAction (editPasteToTable);
	edit_actions->addAction (editPasteToSelection);
	edit_actions->addAction (action_insert_col_left);
	edit_actions->addAction (action_delete_col);
	edit_actions->addAction (action_insert_row_above);
	edit_actions->addAction (action_delete_row);
	edit_actions->addAction (action_delete_rows);

	enableEditing (true);
}

void TwinTable::initTable (RKVarEditModel* model, RObject* object) {
	RK_TRACE (EDITOR);

	datamodel = model;
	main_object = object;
	dataview->setRKModel (model);
	metaview->setRKModel (model->getMetaModel ());
	model->setEditor (this);
	dataview->setRKItemDelegate (new RKItemDelegate (this, datamodel));
	metaview->setRKItemDelegate (new RKItemDelegate (this, datamodel->getMetaModel ()));

	metaview->setMinimumHeight (metaview->horizontalHeader ()->height ());
	metaview->setMaximumHeight (metaview->rowHeight (0) * 5 + metaview->horizontalHeader ()->height () + 5);
	dataview->verticalHeader ()->setFixedWidth (metaview->verticalHeader ()->width ());
	showRownames (false);

// init caption
	addNotificationType (RObjectListener::MetaChanged);
	listenForObject (object);
	objectMetaChanged (object);
	connect (model, SIGNAL (hasDuplicates(const QStringList&)), this, SLOT (containsDuplicates(const QStringList&)));
}

void TwinTable::containsDuplicates (const QStringList& dupes) {
	RK_TRACE (EDITOR);

	if (!rw) return;
	KMessageBox::informationList (this, i18n ("The editor '%1' contains the following duplicate columns. Editing this table may not be safe, and has been disabled. You may re-enable editing if you know what you are doing, but you are strongly advised to fix the table, and/or backup your data, first.", windowTitle ()), dupes, i18n ("Duplicate columns detected"));
	enableEditing (false);
}

void TwinTable::objectMetaChanged (RObject* changed) {
	RK_TRACE (EDITOR);

	RK_ASSERT (changed == main_object);
	QString caption = main_object->getShortName ();
	if (!rw) caption += i18n (" [read-only]");
	setCaption (caption);
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
void TwinTable::contextMenu (int row, int col, const QPoint& pos) {
	RK_TRACE (EDITOR);

	RK_ASSERT (context_menu_table == 0);
	context_menu_row = row;
	context_menu_column = col;
	QString container_name;

	if (sender () == metaview) {
		context_menu_table = metaview;

		if (row == -1) {	// header
			action_insert_col_left->setEnabled (rw && (col >= datamodel->firstRealColumn ()));
			action_insert_col_left->setText (i18n ("Insert new variable left"));	// TODO: show name

			action_delete_col->setEnabled (rw && (col >= datamodel->firstRealColumn ()) && (col < datamodel->trueCols ()));
			action_delete_col->setText (i18n ("Delete this variable"));	// TODO: show name

			container_name = "top_header_menu";
		}
	} else if (sender () == dataview) {
		context_menu_table = dataview;

		if (col == -1) {
			if (row >= 0) {
				RK_ASSERT (row <= datamodel->trueRows ());

				action_insert_row_above->setText (i18n ("Insert new case above (at %1)", row + 1));

				QItemSelectionRange sel = dataview->getSelectionBoundaries ();
				if (sel.isValid () && rw) {
					int top = sel.top ();
					int bottom = sel.bottom ();
					if (bottom >= datamodel->trueRows ()) bottom = datamodel->trueRows () - 1;

					action_delete_rows->setEnabled (top > bottom);
					if (top > bottom) bottom = top;
					action_delete_rows->setText (i18n ("Delete marked rows (%1-%2)", (top+1), (bottom+1)));
				} else {
					action_delete_rows->setEnabled (false);
				}

				action_delete_row->setEnabled (rw && (row < datamodel->trueRows ()));
				action_delete_row->setText (i18n ("Delete this row (%1)", (row+1)));

				container_name = "left_header_menu";
			}
		}
	} else {
		RK_ASSERT (sender () == this);
	}

	if (container_name.isEmpty ()) {	// none of the headers
		container_name = "general_context_menu";
	}

	RK_ASSERT (factory ());
	QMenu* menu = dynamic_cast<QMenu*> (factory ()->container (container_name, this));

	if (menu) {
		menu->exec (pos);
	} else {
		RK_ASSERT (false);	// but may happen, if ui.rc-file was not found
	}

	context_menu_table = 0;
	context_menu_row = context_menu_column = -2;
}

void TwinTable::deleteColumn () {
	RK_TRACE (EDITOR);
	RK_ASSERT (rw);

	if (context_menu_table != metaview) {
		RK_ASSERT (false);
		return;
	}

	RK_ASSERT (context_menu_column >= datamodel->firstRealColumn ());
	flushEdit ();
	datamodel->removeColumns (context_menu_column, 1);
}

void TwinTable::insertColumn () {
	RK_TRACE (EDITOR);
	RK_ASSERT (rw);

	if (context_menu_table != metaview) {
		RK_ASSERT (false);
		return;
	}

	RK_ASSERT (context_menu_column >= datamodel->firstRealColumn ());
	flushEdit ();
	datamodel->insertColumns (context_menu_column, 1);
}

void TwinTable::deleteRow () {
	RK_TRACE (EDITOR);
	RK_ASSERT (rw);

	if (context_menu_table != dataview) {
		RK_ASSERT (false);
		return;
	}

	RK_ASSERT (context_menu_row > 0);
	flushEdit ();
	datamodel->removeRows (context_menu_row, 1);
}

void TwinTable::deleteSelectedRows () {
	RK_TRACE (EDITOR);
	RK_ASSERT (rw);

	if (context_menu_table != dataview) {
		RK_ASSERT (false);
		return;
	}

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
	RK_ASSERT (rw);

	if (context_menu_table != dataview) {
		RK_ASSERT (false);
		return;
	}

	RK_ASSERT (context_menu_row > 0);
	flushEdit ();
	datamodel->insertRows (context_menu_row, 1);
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

	if (!rw) return;

	flushEdit ();

	TwinTableMember *table = activeTable ();
	if (!table) return;

	RKWardMainWindow::getMain ()->slotSetStatusBarText (i18n ("Inserting clipboard contents..."));
	table->paste (paste_mode);
	RKWardMainWindow::getMain ()->slotSetStatusReady ();
}

void TwinTable::paste() {
	RK_TRACE (EDITOR);
	
	paste (PasteEverywhere);
}

void TwinTable::pasteToTable() {
	RK_TRACE (EDITOR);
	
	paste (PasteToTable);
}

void TwinTable::pasteToSelection() {
	RK_TRACE (EDITOR);
	
	paste (PasteToSelection);
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

	if (!rw) return;

	TwinTableMember *table = activeTable ();
	if (!table) return;

	table->blankSelected ();
}

void TwinTable::cut () {
	RK_TRACE (EDITOR);

	copy();
	clearSelected ();
}

void TwinTable::flushEdit () {
	RK_TRACE (EDITOR);

	// flush pending edit operations
	metaview->stopEditing ();
	dataview->stopEditing ();
}

void TwinTable::enableEditing (bool on) {
	RK_TRACE (EDITOR);

	flushEdit ();

	rw = on;
	metaview->rw = rw;
	dataview->rw = rw;

	QPalette palette = metaview->palette ();
	if (on) palette.setColor (QPalette::Base, QApplication::palette ().color (QPalette::Active, QPalette::Base));
	else palette.setColor (QPalette::Base, QApplication::palette ().color (QPalette::Disabled, QPalette::Base));
	metaview->setPalette (palette);
	dataview->setPalette (palette);

	QAbstractItemView::EditTriggers triggers = QAbstractItemView::NoEditTriggers;
	if (rw) triggers = QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed;
	metaview->setEditTriggers (triggers);
	dataview->setEditTriggers (triggers);

	edit_actions->setEnabled (rw);
	action_enable_editing->setChecked (rw);

	if (main_object) objectMetaChanged (main_object);	// update_caption;
}

void TwinTable::showRownames (bool show) {
	RK_TRACE (EDITOR);
	RK_ASSERT (show == action_show_rownames->isChecked ());

	metaview->setColumnHidden (0, !show);
	dataview->setColumnHidden (0, !show);
	datamodel->lockHeader (!show);
}

#include "twintable.moc"
