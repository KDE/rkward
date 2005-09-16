/***************************************************************************
                          rkeditordataframepart  -  description
                             -------------------
    begin                : Wed Sep 14 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

#include <kinstance.h>
#include <kaction.h>
#include <klocale.h>

#include "rkeditordataframe.h"
#include "rkdrag.h"
#include "../rkward.h"
#include "../rkglobals.h"
#include "../debug.h"

RKEditorDataFramePart::RKEditorDataFramePart (QWidget *parent, RKEditorDataFrame *editor_widget) : KParts::ReadWritePart (parent) {
	RK_TRACE (EDITOR);
	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);
 
	setWidget (editor_widget);
	editor = editor_widget;

	setXMLFile ("rkeditordataframepart.rc");

	setReadWrite (true);

	initializeActions ();
}

RKEditorDataFramePart::~RKEditorDataFramePart () {
	RK_TRACE (EDITOR);
}

void RKEditorDataFramePart::initializeActions () {
	editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection(), "cut");
	editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection(), "copy");
	editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection(), "paste");
	editPasteToTable = new KAction(i18n("Paste inside Table"), 0, 0, this, SLOT(slotEditPasteToTable()), actionCollection(), "paste_to_table");
	editPasteToTable->setIcon("frame_spreadsheet");
	editPasteToSelection = new KAction(i18n("Paste inside Selection"), 0, 0, this, SLOT(slotEditPasteToSelection()), actionCollection(), "paste_to_selection");
	editPasteToSelection->setIcon("frame_edit");
	//windowClose = new KAction (i18n("Close"), 0, KShortcut ("Ctrl+W"), this, SLOT (slotCloseWindow()), actionCollection(), "window_close");

	editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
	editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
	editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
	editPasteToTable->setStatusText(i18n("Pastes the clipboard contents to actual position, but not beyond the table's boundaries"));
	editPasteToSelection->setStatusText(i18n("Pastes the clipboard contents to actual position, but not beyond the boundaries of the current selection"));
}

void RKEditorDataFramePart::slotEditCut () {
	RK_TRACE (EDITOR);
	
	RKGlobals::rkApp ()->slotStatusMsg(i18n("Cutting selection..."));
	slotEditCopy ();
	editor->clearSelected ();
	RKGlobals::rkApp ()->slotStatusReady ();
}

void RKEditorDataFramePart::slotEditCopy() {
	RK_TRACE (EDITOR);
	
	RKGlobals::rkApp ()->slotStatusMsg(i18n("Copying selection to clipboard..."));
	QApplication::clipboard()->setData(editor->makeDrag ());
	RKGlobals::rkApp ()->slotStatusReady ();
}


void RKEditorDataFramePart::slotEditPaste() {
	RK_TRACE (EDITOR);
	
	editor->setPasteMode (RKEditor::PasteEverywhere);
	doPaste ();
}

void RKEditorDataFramePart::slotEditPasteToTable() {
	RK_TRACE (EDITOR);
	
	editor->setPasteMode (RKEditor::PasteToTable);
	doPaste();
}

void RKEditorDataFramePart::slotEditPasteToSelection() {
	RK_TRACE (EDITOR);
	
	editor->setPasteMode (RKEditor::PasteToSelection);
	doPaste();
}

void RKEditorDataFramePart::doPaste () {
	RK_TRACE (EDITOR);

	RKGlobals::rkApp ()->slotStatusMsg(i18n("Inserting clipboard contents..."));

	// actually, we don't care, whether tsv or plain gets pasted - it's both
	// treated the same. We should however encourage external senders to
	// provided the two in order.
	if (QApplication::clipboard()->data()->provides ("text/tab-separated-values")) {
		RK_DO (qDebug ("paste tsv"), EDITOR, DL_DEBUG);
		editor->paste (QApplication::clipboard()->data()->encodedData ("text/tab-separated-values"));
	} else if (QApplication::clipboard()->data()->provides ("text/plain")) {
		RK_DO (qDebug ("paste plaing"), EDITOR, DL_DEBUG);
		editor->paste (QApplication::clipboard()->data()->encodedData ("text/plain"));
	}

	RKGlobals::rkApp ()->slotStatusMsg(i18n("Ready."));
}
/*
void RKEditorDataFramePart::slotCloseWindow () {
	RK_TRACE (EDITOR);

	RKGlobals::rkApp ()->closeWindow (editor);
}*/

#include "rkeditordataframepart.moc"
