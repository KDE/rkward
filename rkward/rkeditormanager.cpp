/***************************************************************************
                          rkeditormanager  -  description
                             -------------------
    begin                : Fri Aug 20 2004
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
#include "rkeditormanager.h"

#include <qtabwidget.h>
#include <qlayout.h>

#include "dataeditor/rkeditor.h"
#include "dataeditor/rkeditordataframe.h"
#include "core/robject.h"
#include "core/rcontainerobject.h"
#include "core/robjectlist.h"
#include "rbackend/rinterface.h"
#include "rkglobals.h"

#include "debug.h"

#define RESTORE_COMMAND 1

RKEditorManager::RKEditorManager (QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);	

	QGridLayout *grid = new QGridLayout (this, 1, 1);
	tabbook = new QTabWidget (this);
	grid->addWidget (tabbook, 0, 0);
	
	restore_chain = 0;
}

RKEditorManager::~RKEditorManager () {
	RK_TRACE (APP);	
}

RKEditor *RKEditorManager::editObject (RObject *object, bool initialize_to_empty) {
	RK_TRACE (APP);	
	RObject *iobj = object;
	RKEditor *ed = 0;
	if (!object->objectOpened()) {
		if (object->isDataFrame ()) {
			ed = new RKEditorDataFrame (tabbook);
			// TODO: add child objects, too?
			ed->openObject (object, initialize_to_empty);
		} else if (object->isVariable () && object->getContainer ()->isDataFrame ()) {
			if (!object->getContainer ()->objectOpened ()) { 
				iobj = object->getContainer ();
				ed = new RKEditorDataFrame (tabbook);
				// TODO: add child objects, too?
				ed->openObject (iobj, initialize_to_empty);
				// ed->focusObject (obj);
			} else {
				tabbook->showPage (object->getContainer ()->objectOpened ());
			}
		}

		if (ed) {
			hide ();
			tabbook->insertTab (ed, iobj->getShortName ());
			tabbook->showPage (ed);
			show ();
			emit (editorOpened ());
			
			RCommand *command = new RCommand (".rk.editor.opened (" + iobj->getFullName() + ")", RCommand::App | RCommand::Sync);
			RKGlobals::rInterface ()->issueCommand (command, restore_chain);
		}
	} else {
		tabbook->showPage (object->objectOpened ());
	}
	
	return ed;
}

void RKEditorManager::restoreEditors () {
	RK_TRACE (APP);	
	restore_chain = RKGlobals::rInterface ()->startChain (restore_chain);
	
	RCommand *command = new RCommand ("if (exists (\".rk.editing\")) { .rk.editingtemp <- .rk.editing; remove (.rk.editing); .rk.editingtemp; }", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, RESTORE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, restore_chain);
}

void RKEditorManager::rCommandDone (RCommand *command) {
	RK_TRACE (APP);	
	if (command->getFlags () == RESTORE_COMMAND) {
		for (int i=0; i < command->stringVectorLength (); ++i) {
			RObject *object = RKGlobals::rObjectList ()->findObject (command->getStringVector ()[i]);
			if (object) {
				editObject (object);
			}
		}
		
		RCommand *command = new RCommand ("if (exists (\".rk.editingtemp\")) remove (.rk.editingtemp)", RCommand::App | RCommand::Sync);
		RKGlobals::rInterface ()->issueCommand (command, restore_chain);
		restore_chain = RKGlobals::rInterface ()->closeChain (restore_chain);
	} else {
		RK_ASSERT (false);
	}
}

RKEditor *RKEditorManager::currentEditor () {
	RK_TRACE (APP);	
	return static_cast<RKEditor*> (tabbook->currentPage ());
}

int RKEditorManager::numEditors () {
	RK_TRACE (APP);	
	return tabbook->count ();
}

void RKEditorManager::closeEditor (RKEditor *editor) {
	RK_TRACE (APP);
	
	RK_ASSERT (editor);
	
	RObject *object = editor->getObject ();
	delete editor;

	RCommand *command = new RCommand (".rk.editor.closed (" + object->getFullName() + ")", RCommand::App | RCommand::Sync);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	
	emit (editorClosed ());
}

void RKEditorManager::flushAll () {
	RK_TRACE (APP);

	for (int i=0; i < tabbook->count (); ++i) {
		static_cast<RKEditor*> (tabbook->page (i))->flushChanges ();
	}
}

bool RKEditorManager::canEditObject (RObject *object) {
	RK_TRACE (APP);
	
	if (object->isDataFrame ()) {
		return true;
	} else if (object->isVariable () && object->getContainer ()->isDataFrame ()) {
		return true;
	}
	return false;
}

void RKEditorManager::setEditorName (RKEditor *editor, const QString &new_name) {
	tabbook->setTabLabel (editor, new_name);
}

#include "rkeditormanager.moc"
