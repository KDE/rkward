/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rkcommandeditorwindow.h"

#include <qlayout.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include "../rkcommandeditor.h"
#include "../rbackend/rinterface.h"
#include "../rkeditormanager.h"
#include "../rkglobals.h"

// TODO: use KActions
RKCommandEditorWindow::RKCommandEditorWindow (QWidget *parent) : QWidget (parent) {
	QGridLayout *grid = new QGridLayout (this, 2, 1);
	
	KMenuBar *menu = new KMenuBar (this);
	QPopupMenu *file_menu = new QPopupMenu (this);
	file_menu->insertItem (i18n ("New"), this, SLOT (newWindow ()));
	file_menu->insertItem (i18n ("Open"), this, SLOT (load ()));
	file_menu->insertSeparator ();
	file_menu->insertItem (i18n ("Save"), this, SLOT (save ()));
	file_menu->insertItem (i18n ("Save As"), this, SLOT (saveAs ()));
	file_menu->insertSeparator ();
	file_menu->insertItem (i18n ("Print"), this, SLOT (print ()));	
	file_menu->insertSeparator ();
	file_menu->insertItem (i18n ("Close"), this, SLOT (close ()));
	menu->insertItem (i18n ("File"), file_menu);
	
	QPopupMenu *run_menu = new QPopupMenu (this);
	run_menu->insertItem (i18n ("Run all"), this, SLOT (run ()));
	run_menu->insertSeparator ();
	run_menu->insertItem (i18n ("Run selection"), this, SLOT (runSelection ()));
	run_menu->setItemEnabled (run_menu->insertItem (i18n ("Run up to current line"), this, SLOT (runToCursor ())), false);
	run_menu->setItemEnabled (run_menu->insertItem (i18n ("Run from current line"), this, SLOT (runFromCursor ())), false);
	menu->insertItem (i18n ("Run"), run_menu);
	
	QPopupMenu *settings_menu = new QPopupMenu (this);
	settings_menu->insertItem (i18n ("Word wrap"), this, SLOT (wordWrap ()));
	settings_menu->insertItem (i18n ("Line Numbering"), this, SLOT (lineNumbers ()));
	settings_menu->insertSeparator ();
	settings_menu->insertItem (i18n ("Configure editor"), this, SLOT (configure ()));
	menu->insertItem (i18n ("Settings"), settings_menu);
	
	menu->insertSeparator ();
	
	QPopupMenu *help_menu = new QPopupMenu (this);
	help_menu->setItemEnabled (help_menu->insertItem (i18n ("Sorry, no help available so far"), 0, 0), false);
	menu->insertItem (i18n ("Help"), help_menu);
	
	grid->addWidget (menu, 0, 0);
	
	grid->addWidget (editor = new RKCommandEditor (this, false), 1, 0);

	setCaption (caption = i18n ("Command editor"));
	resize (minimumSizeHint ().expandedTo (QSize (640, 480)));
	show ();
}

RKCommandEditorWindow::~RKCommandEditorWindow () {
	delete editor;
}

void RKCommandEditorWindow::closeEvent (QCloseEvent *e) {
// TODO: call quit in order to try to save changes
	if (checkSave ()) {
		e->accept ();
		delete this;
	}
}

bool RKCommandEditorWindow::checkSave () {
	if (editor->isModified ()) {
		int ret = KMessageBox::warningYesNoCancel (this, i18n ("The code was modified. Do you want to save it?"), i18n ("Save code?"));
		if (ret == KMessageBox::Yes) save ();
		else if (ret == KMessageBox::No) return true;
		else return false;
	}
	return true;
}

void RKCommandEditorWindow::trySave (const KURL &url) {
	if (!editor->save (url)) {
		if (KMessageBox::warningYesNo (this, i18n ("Saving to file '") + url.path () + i18n ("' failed. Try with a different filename?"), i18n ("Saving failed")) == KMessageBox::Yes) saveAs ();
	} else {
		setCaption (caption + " - " + url.filename ());
	}
}

// GUI slots below
void RKCommandEditorWindow::newWindow () {
	new RKCommandEditorWindow (0);
}

void RKCommandEditorWindow::save () {
	if (editor->getURL ().isEmpty ()) {
		saveAs ();
	} else {
		trySave (editor->getURL ());
	}
}

void RKCommandEditorWindow::saveAs () {
	KURL url = KFileDialog::getSaveURL (QString::null, "*.R", this);
	if (!url.isEmpty ()) trySave (url);
}

void RKCommandEditorWindow::print () {
	editor->print ();
}

void RKCommandEditorWindow::close () {
	if (checkSave ()) delete this;
}

void RKCommandEditorWindow::load () {
	if (!checkSave ()) return;
	
	KURL url = KFileDialog::getOpenURL (QString::null, "*.R", this);
	if (!url.isEmpty ()) {
		if (editor->open (url)) {
			setCaption (caption + " - " + url.filename ());
		}
	}
}

void RKCommandEditorWindow::run () {
	RKGlobals::editorManager ()->flushAll ();
	RKGlobals::rInterface ()->issueCommand (new RCommand (editor->text (), RCommand::User, ""));
}

void RKCommandEditorWindow::runSelection () {
	RKGlobals::editorManager ()->flushAll ();
	RKGlobals::rInterface ()->issueCommand (new RCommand (editor->getSelection (), RCommand::User, ""));
}

void RKCommandEditorWindow::runToCursor () {
}

void RKCommandEditorWindow::runFromCursor () {
}

void RKCommandEditorWindow::configure () {
	editor->configure ();
}

void RKCommandEditorWindow::wordWrap () {
	editor->toggleWordWrap ();
}

void RKCommandEditorWindow::lineNumbers () {
	editor->toggleLineNumbers ();
}

#include "rkcommandeditorwindow.moc"
