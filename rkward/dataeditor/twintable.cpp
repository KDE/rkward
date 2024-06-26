/*
twintable.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Tue Oct 29 2002
SPDX-FileCopyrightText: 2002-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "twintable.h"

#include <KLocalizedString>
#include <QAction>
#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <kmessagebox.h>
#include <ktoggleaction.h>

#include <qvariant.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <QMenu>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QApplication>
#include <QActionGroup>

#include "twintablemember.h"
#include "rkvareditmodel.h"
#include "../core/rcontainerobject.h"
#include "../misc/rkstandardicons.h"
#include "../rkward.h"

#include "../debug.h"

TwinTable::TwinTable (QWidget *parent) : RKEditor (parent), RObjectListener (RObjectListener::Other), KXMLGUIClient () {
	RK_TRACE (EDITOR);

	main_object = nullptr;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins (0, 0, 0, 0);
	
	splitter = new QSplitter (this);
	splitter->setOrientation (Qt::Vertical);

	metaview = new TwinTableMember (splitter);
	splitter->setStretchFactor (splitter->indexOf (metaview), 0);
	metaview->verticalHeader ()->setSectionResizeMode (QHeaderView::Fixed);
	metaview->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
	
	dataview = new TwinTableMember (splitter);
	splitter->setStretchFactor (splitter->indexOf (dataview), 1);
	dataview->verticalHeader ()->setSectionResizeMode (QHeaderView::Fixed);
	dataview->horizontalHeader ()->hide ();
	dataview->setAlternatingRowColors (true);

	layout->addWidget (splitter);

	// these are to keep the two tables in sync
	metaview->setTwin (dataview);
	dataview->setTwin (metaview);

	// pressing the columns in the metaview-header should select columns in the dataview
	// Note that the disconnects are on connections already set up by Qt. Since we don't want to worry about how these were set, we're disconnecting
	// each in both old and new syntax.
	disconnect (metaview->horizontalHeader (), SIGNAL (sectionClicked(int)));
	disconnect (metaview->horizontalHeader (), &QHeaderView::sectionClicked, nullptr, nullptr);
	connect (metaview->horizontalHeader (), &QHeaderView::sectionClicked, this, &TwinTable::metaHeaderClicked);
	disconnect (metaview->horizontalHeader (), SIGNAL (sectionPressed(int)));
	disconnect (metaview->horizontalHeader (), &QHeaderView::sectionPressed, nullptr, nullptr);
	connect (metaview->horizontalHeader (), &QHeaderView::sectionPressed, this, &TwinTable::metaHeaderPressed);
	disconnect (metaview->horizontalHeader (), SIGNAL (sectionEntered(int)));
	disconnect (metaview->horizontalHeader (), &QHeaderView::sectionEntered, nullptr, nullptr);
	connect (metaview->horizontalHeader (), &QHeaderView::sectionEntered, this, &TwinTable::metaHeaderEntered);
	meta_header_anchor_section = -1;

	// catch header context menu requests
	connect (dataview, &TwinTableMember::contextMenuRequest, this, &TwinTable::contextMenu);
	connect (metaview, &TwinTableMember::contextMenuRequest, this, &TwinTable::contextMenu);
	context_menu_table = nullptr;
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

	editCut = actionCollection()->addAction(KStandardAction::Cut, "cut", this, &TwinTable::cut);
	editCut->setWhatsThis(i18n("Cuts the selected section and puts it to the clipboard"));
	editCopy = actionCollection()->addAction (KStandardAction::Copy, "copy", this, &TwinTable::copy);
	editCopy->setWhatsThis(i18n("Copies the selected section to the clipboard"));
//	editor->editActions ()->addAction (editCopy);	// this is a read-only action, not an "edit" action
	editPaste = actionCollection()->addAction(KStandardAction::Paste, "paste", this, qOverload<>(&TwinTable::paste));
	editPaste->setWhatsThis(i18n("Pastes the clipboard contents to current position"));

	editPasteToTable = actionCollection()->addAction("paste_to_table", this, &TwinTable::pasteToTable);
	editPasteToTable->setText(i18n("Paste inside table"));
	editPasteToTable->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionPasteInsideTable));
	editPasteToTable->setWhatsThis(i18n("Pastes the clipboard contents to current position, but not beyond the table's boundaries"));

	editPasteToSelection = actionCollection()->addAction("paste_to_selection", this, &TwinTable::pasteToSelection);
	editPasteToSelection->setText(i18n("Paste inside selection"));
	editPasteToSelection->setIcon(RKStandardIcons::getIcon (RKStandardIcons::ActionPasteInsideSelection));
	editPasteToSelection->setWhatsThis(i18n("Pastes the clipboard contents to current position, but not beyond the boundaries of the current selection"));

	// header menus
	action_insert_col_left = actionCollection()->addAction("insert_col_left", this, &TwinTable::insertColumn);
	action_insert_col_left->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionInsertVar));
	action_delete_col = actionCollection()->addAction("delete_col", this, &TwinTable::deleteColumn);
	action_delete_col->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionDeleteVar));

	action_insert_row_above = actionCollection()->addAction("insert_row_above", this, &TwinTable::insertRow);
	action_insert_row_above->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionInsertRow));
	action_delete_row = actionCollection()->addAction("delete_row", this, &TwinTable::deleteRow);
	action_delete_row->setIcon(RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteRow));
	action_delete_rows = actionCollection()->addAction("delete_rows", this, &TwinTable::deleteSelectedRows);
	action_delete_rows->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionDeleteRow));

	// global actions
	action_show_rownames = actionCollection()->addAction("show_rownames", this, &TwinTable::showRownames);
	action_show_rownames->setText(i18n("Show / Edit row names"));
	action_show_rownames->setCheckable(true);
	action_enable_editing = actionCollection()->addAction ("enable_editing", this, &TwinTable::enableEditing);
	action_enable_editing->setText(i18n("Enable editing"));
	action_enable_editing->setCheckable(true);
	// these actually do the same thing, but are designed to work well in the toolbar
	QActionGroup *lockactions = new QActionGroup(this);
	lockactions->setExclusive(true);
	action_tb_lock_editing = new KToggleAction(i18nc("verb: switch to read-only state. Make this short.", "Lock"), this);
	action_tb_lock_editing->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionLock));
	action_tb_lock_editing->setActionGroup (lockactions);
	action_tb_lock_editing->setWhatsThis(i18n ("Disable editing (to prevent accidental modification of data)"));
	actionCollection ()->addAction ("lock_editing", action_tb_lock_editing);
	action_tb_unlock_editing = new KToggleAction (i18nc ("verb: switch to read-write state. Make this short.", "Unlock"), this);
	action_tb_unlock_editing->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionUnlock));
	action_tb_unlock_editing->setActionGroup (lockactions);
	action_tb_unlock_editing->setWhatsThis(i18n ("Enable editing"));
	actionCollection ()->addAction ("unlock_editing", action_tb_unlock_editing);
	connect (action_tb_unlock_editing, &QAction::toggled, this, &TwinTable::enableEditing);
	// NOTE: No need to connect lock_editing, too, as they are radio-exclusive

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
	action_enable_editing->setEnabled (object->canWrite ());
	action_tb_unlock_editing->setEnabled (object->canWrite ());
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
	connect (model, &RKVarEditModel::hasDuplicates, this, &TwinTable::containsDuplicates);
}

void TwinTable::setWindowStyleHint (const QString& hint) {
	RK_TRACE (EDITOR);
	if (hint == "preview") { // preview skin: Squeeze header as much as possible
		metaview->horizontalHeader ()->hide ();
		metaview->setMinimumHeight (metaview->rowHeight (0));
		// Now, I just don't understand QSplitter sizing, here... Despite stretch factors being set, metaview continues to be the first to grow.
		// Forcing minimum height of dataview helps allocating initial size to the dataview, though.
		dataview->setMinimumHeight (dataview->rowHeight (0) * 5);
	}
	RKMDIWindow::setWindowStyleHint (hint);
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
	if (!rw) caption = i18n ("%1 [read-only]", caption);
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

	RK_ASSERT (context_menu_table == nullptr);
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

					action_delete_rows->setEnabled (bottom > top);
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

	context_menu_table = nullptr;
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
		return nullptr;
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

	rw = main_object ? on && main_object->canWrite () : on;  // NOTE: File->New->Dataset creates an Editor window, first, then sets the object to edit afterwards (TODO which looks like silly design)
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
	action_tb_lock_editing->setChecked (!rw);
	action_tb_unlock_editing->setChecked (rw);

	if (main_object) objectMetaChanged (main_object);	// update_caption;
}

void TwinTable::showRownames (bool show) {
	RK_TRACE (EDITOR);
	RK_ASSERT (show == action_show_rownames->isChecked ());

	metaview->setColumnHidden (0, !show);
	dataview->setColumnHidden (0, !show);
	datamodel->lockHeader (!show);
}

