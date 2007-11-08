/***************************************************************************
                          rkeditordataframepart  -  description
                             -------------------
    begin                : Wed Sep 14 2005
    copyright            : (C) 2005, 2006, 2007 by Thomas Friedrichsmeier
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

#include "rkeditordataframepart.h"

#include <qclipboard.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>

#include "rkeditordataframe.h"
#include "../rkward.h"
#include "../debug.h"

RKEditorDataFramePart::RKEditorDataFramePart (QObject *parent, RKEditorDataFrame* editor) : KParts::Part (parent) {
	RK_TRACE (EDITOR);
	setComponentData (KGlobal::mainComponent ());
 
	RKEditorDataFramePart::editor = editor;
	setWidget (editor);

	setXMLFile ("rkeditordataframepart.rc");

	initializeActions ();
}

RKEditorDataFramePart::~RKEditorDataFramePart () {
	RK_TRACE (EDITOR);
}

void RKEditorDataFramePart::initializeActions () {
	editCut = actionCollection ()->addAction (KStandardAction::Cut, "cut", this, SLOT(slotEditCut()));
	editCopy = actionCollection ()->addAction (KStandardAction::Copy, "copy", this, SLOT(slotEditCopy()));
	editPaste = actionCollection ()->addAction (KStandardAction::Paste, "paste", this, SLOT(slotEditPaste()));
	editPasteToTable = actionCollection ()->addAction ("paste_to_table", this, SLOT(slotEditPasteToTable()));
	editPasteToTable->setText (i18n("Paste inside Table"));
	editPasteToTable->setIcon (KIcon ("frame_spreadsheet"));

	editPasteToSelection = actionCollection ()->addAction ("paste_to_selection", this, SLOT(slotEditPasteToSelection()));
	editPasteToSelection->setText (i18n("Paste inside Selection"));
	editPasteToSelection->setIcon (KIcon ("frame_edit"));

	editCut->setStatusTip (i18n("Cuts the selected section and puts it to the clipboard"));
	editCopy->setStatusTip (i18n("Copies the selected section to the clipboard"));
	editPaste->setStatusTip (i18n("Pastes the clipboard contents to actual position"));
	editPasteToTable->setStatusTip (i18n("Pastes the clipboard contents to actual position, but not beyond the table's boundaries"));
	editPasteToSelection->setStatusTip (i18n("Pastes the clipboard contents to actual position, but not beyond the boundaries of the current selection"));
}

void RKEditorDataFramePart::slotEditCut () {
	RK_TRACE (EDITOR);
	
	RKWardMainWindow::getMain ()->slotSetStatusBarText (i18n ("Cutting selection..."));
	slotEditCopy ();
	editor->clearSelected ();
	RKWardMainWindow::getMain ()->slotSetStatusReady ();
}

void RKEditorDataFramePart::slotEditCopy() {
	RK_TRACE (EDITOR);
	
	RKWardMainWindow::getMain ()->slotSetStatusBarText (i18n ("Copying selection to clipboard..."));
	editor->copy ();
	RKWardMainWindow::getMain ()->slotSetStatusReady ();
}


void RKEditorDataFramePart::slotEditPaste() {
	RK_TRACE (EDITOR);
	
	doPaste (RKEditor::PasteEverywhere);
}

void RKEditorDataFramePart::slotEditPasteToTable() {
	RK_TRACE (EDITOR);
	
	doPaste (RKEditor::PasteToTable);
}

void RKEditorDataFramePart::slotEditPasteToSelection() {
	RK_TRACE (EDITOR);
	
	doPaste (RKEditor::PasteToSelection);
}

void RKEditorDataFramePart::doPaste (RKEditor::PasteMode mode) {
	RK_TRACE (EDITOR);

	RKWardMainWindow::getMain ()->slotSetStatusBarText (i18n ("Inserting clipboard contents..."));

	editor->paste (mode);

	RKWardMainWindow::getMain ()->slotSetStatusReady ();
}

#include "rkeditordataframepart.moc"
