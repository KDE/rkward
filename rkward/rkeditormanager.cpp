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

#include "dataeditor/rkeditor.h"
#include "dataeditor/rkeditordataframe.h"
#include "core/robject.h"
#include "core/rcontainerobject.h"
#include "core/robjectlist.h"
#include "rbackend/rinterface.h"
#include "rkglobals.h"
#include "rkward.h"

#include "debug.h"

#include <kiconloader.h>

#define RESTORE_COMMAND 1

RKEditorManager::RKEditorManager () : QObject () {
	RK_TRACE (APP);

	restore_chain = 0;
}

RKEditorManager::~RKEditorManager () {
	RK_TRACE (APP);	
}

RKEditor *RKEditorManager::editObject (RObject *object, bool initialize_to_empty) {
	RK_TRACE (APP);	
	RObject *iobj = object;
	RKEditor *ed = 0;
	if (!object->objectOpened ()) {
		if (object->isDataFrame ()) {
			ed = newRKEditorDataFrame ();
			// TODO: add child objects, too?
			ed->openObject (object, initialize_to_empty);
		} else if (object->isVariable () && object->getContainer ()->isDataFrame ()) {
			if (!object->getContainer ()->objectOpened ()) { 
				iobj = object->getContainer ();
				ed = newRKEditorDataFrame ();
				// TODO: add child objects, too?
				ed->openObject (iobj, initialize_to_empty);
				// ed->focusObject (obj);
			} else {
				if (object->getContainer ()->objectOpened ()) {
					object->getContainer ()->objectOpened ()->show ();
					object->getContainer ()->objectOpened ()->raise ();
				}
			}
		}

		if (ed) {
			setEditorName (ed, iobj->getShortName ());
			emit (editorOpened ());

			RCommand *command = new RCommand (".rk.editor.opened (" + iobj->getFullName() + ")", RCommand::App | RCommand::Sync);
			RKGlobals::rInterface ()->issueCommand (command, restore_chain);
		}
	} else {
		object->objectOpened ()->show ();
		object->objectOpened ()->raise ();
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

void RKEditorManager::closeEditor (RKEditor *editor) {
	RK_TRACE (APP);
	
	RK_ASSERT (editor);
	
	RObject *object = editor->getObject ();
	RKGlobals::rkApp ()->closeWindow (editor);

	RCommand *command = new RCommand (".rk.editor.closed (" + object->getFullName() + ")", RCommand::App | RCommand::Sync);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	
	emit (editorClosed ());
}

void RKEditorManager::flushAll () {
	RK_TRACE (APP);

	for (QValueList<RKEditor*>::const_iterator it = editors.begin (); it != editors.end (); ++it) {
		(*it)->flushChanges ();
	}
}

void RKEditorManager::closeAll () {
	RK_TRACE (APP);

	for (QValueList<RKEditor*>::const_iterator it = editors.begin (); it != editors.end (); ++it) {
		closeEditor (*it);
	}

	editors.clear ();
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
	RK_TRACE (APP);
	editor->setMDICaption (new_name);
}

RKEditorDataFrame *RKEditorManager::newRKEditorDataFrame () {
	RK_TRACE (APP);

	RKEditorDataFrame *ed = new RKEditorDataFrame (0);
	(RKGlobals::rkApp()->m_manager)->addPart (ed->getPart (), false);
	RKGlobals::rkApp ()->addWindow (ed);
	ed->setIcon (SmallIcon ("spreadsheet"));
	editors.append (ed);
	RKGlobals::rkApp ()->activateGUI (ed->getPart ());

	return ed;
}

#include "rkeditormanager.moc"
