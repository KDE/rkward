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
#include <qapplication.h>

#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kaccel.h>

#include "../rkcommandeditor.h"
#include "../rbackend/rinterface.h"
#include "../rkeditormanager.h"
#include "../rkglobals.h"

#include "../debug.h"

// TODO: use KActions everywhere. Find a way to use a ui.rc-file
RKCommandEditorWindow::RKCommandEditorWindow (QWidget *parent) : QWidget (parent) {
	RK_TRACE (COMMANDEDITOR);
	connect (qApp, SIGNAL (aboutToQuit ()), this, SLOT (close ()));
	QGridLayout *grid = new QGridLayout (this, 2, 1);
	
	KActionCollection *action_collection = new KActionCollection (this);
	
	KMenuBar *menu = new KMenuBar (this);
	QPopupMenu *file_menu = new QPopupMenu (this);
	file_new = KStdAction::openNew (this, SLOT (newWindow ()), action_collection);
	file_new->plug (file_menu);
	file_open = KStdAction::open (this, SLOT (load ()), action_collection);
	file_open->plug (file_menu);
	file_menu->insertSeparator ();
	file_save = KStdAction::save (this, SLOT (save ()), action_collection);
	file_save->plug (file_menu);
	file_save_as = KStdAction::saveAs (this, SLOT (saveAs ()), action_collection);
	file_save_as->plug (file_menu);
	file_menu->insertSeparator ();
	file_print = KStdAction::print (this, SLOT (print ()), action_collection);
	file_print->plug (file_menu);
	file_menu->insertSeparator ();
	file_close = KStdAction::close (this, SLOT (close ()), action_collection);
	file_close->plug (file_menu);
	menu->insertItem (i18n ("File"), file_menu);
	
	QPopupMenu *run_menu = new QPopupMenu (this);
	run_all = new KAction (i18n ("Run all"), KShortcut ("Alt+R"), this, SLOT (run ()), action_collection);
	run_all->plug (run_menu);
	run_menu->insertSeparator ();
	run_menu->insertItem (i18n ("Run selection"), this, SLOT (runSelection ()));
	run_menu->setItemEnabled (run_menu->insertItem (i18n ("Run up to current line"), this, SLOT (runToCursor ())), false);
	run_menu->setItemEnabled (run_menu->insertItem (i18n ("Run from current line"), this, SLOT (runFromCursor ())), false);
	menu->insertItem (i18n ("Run"), run_menu);
	
	QPopupMenu *settings_menu = new QPopupMenu (this);
	word_wrap = new KToggleAction (i18n ("Word wrap"), KShortcut ("F10"), this, SLOT (wordWrap ()), action_collection);
	word_wrap->plug (settings_menu);
	line_numbers = new KToggleAction (i18n ("Line Numbering"), KShortcut ("F11"), this, SLOT (lineNumbers ()), action_collection);
	line_numbers->plug (settings_menu);
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
	RK_TRACE (COMMANDEDITOR);
	delete editor;
}

void RKCommandEditorWindow::closeEvent (QCloseEvent *e) {
	RK_TRACE (COMMANDEDITOR);
// TODO: call quit in order to try to save changes
	if (checkSave ()) {
		e->accept ();
		delete this;
	}
}

bool RKCommandEditorWindow::checkSave () {
	RK_TRACE (COMMANDEDITOR);
	if (editor->isModified ()) {
		int ret = KMessageBox::warningYesNoCancel (this, i18n ("The code was modified. Do you want to save it?"), i18n ("Save code?"));
		if (ret == KMessageBox::Yes) save ();
		else if (ret == KMessageBox::No) return true;
		else return false;
	}
	return true;
}

void RKCommandEditorWindow::trySave (const KURL &url) {
	RK_TRACE (COMMANDEDITOR);
	if (!editor->save (url)) {
		if (KMessageBox::warningYesNo (this, i18n ("Saving to file '") + url.path () + i18n ("' failed. Try with a different filename?"), i18n ("Saving failed")) == KMessageBox::Yes) saveAs ();
	} else {
		setCaption (caption + " - " + url.filename ());
	}
}

// GUI slots below
void RKCommandEditorWindow::newWindow () {
	RK_TRACE (COMMANDEDITOR);
	new RKCommandEditorWindow (0);
}

void RKCommandEditorWindow::save () {
	RK_TRACE (COMMANDEDITOR);
	if (editor->getURL ().isEmpty ()) {
		saveAs ();
	} else {
		trySave (editor->getURL ());
	}
}

void RKCommandEditorWindow::saveAs () {
	RK_TRACE (COMMANDEDITOR);
	KURL url = KFileDialog::getSaveURL (QString::null, "*.R", this);
	if (!url.isEmpty ()) trySave (url);
}

void RKCommandEditorWindow::print () {
	RK_TRACE (COMMANDEDITOR);
	editor->print ();
}

void RKCommandEditorWindow::close () {
	RK_TRACE (COMMANDEDITOR);
	if (checkSave ()) delete this;
}

void RKCommandEditorWindow::load () {
	RK_TRACE (COMMANDEDITOR);
	if (!checkSave ()) return;
	
	KURL url = KFileDialog::getOpenURL (QString::null, "*.R", this);
	if (!url.isEmpty ()) {
		if (editor->open (url)) {
			setCaption (caption + " - " + url.filename ());
		}
	}
}

void RKCommandEditorWindow::run () {
	RK_TRACE (COMMANDEDITOR);
	RKGlobals::editorManager ()->flushAll ();
	RKGlobals::rInterface ()->issueCommand (new RCommand (editor->text (), RCommand::User, ""));
}

void RKCommandEditorWindow::runSelection () {
	RK_TRACE (COMMANDEDITOR);
	RKGlobals::editorManager ()->flushAll ();
	RKGlobals::rInterface ()->issueCommand (new RCommand (editor->getSelection (), RCommand::User, ""));
}

void RKCommandEditorWindow::runToCursor () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCommandEditorWindow::runFromCursor () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCommandEditorWindow::configure () {
	RK_TRACE (COMMANDEDITOR);
	editor->configure ();
}

void RKCommandEditorWindow::wordWrap () {
	RK_TRACE (COMMANDEDITOR);
	editor->toggleWordWrap ();
}

void RKCommandEditorWindow::lineNumbers () {
	RK_TRACE (COMMANDEDITOR);
	editor->toggleLineNumbers ();
}

#include "rkcommandeditorwindow.moc"
