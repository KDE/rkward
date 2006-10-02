/***************************************************************************
                          rkworkplace  -  description
                             -------------------
    begin                : Thu Sep 21 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#include "rkworkplace.h"

#include <kparts/partmanager.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>

#include "detachedwindowcontainer.h"
#include "rkcommandeditorwindow.h"
#include "rkhtmlwindow.h"
#include "rkworkplaceview.h"
#include "../core/robject.h"
#include "../core/rcontainerobject.h"
#include "../core/robjectlist.h"
#include "../dataeditor/rkeditor.h"
#include "../dataeditor/rkeditordataframe.h"
#include "../dataeditor/rkeditordataframepart.h"
#include "../rbackend/rinterface.h"
#include "../rbackend/rcommand.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

#define RESTORE_WORKPLACE_COMMAND 1

// static
RKWorkplace *RKWorkplace::main_workplace = 0;

RKWorkplace::RKWorkplace (QWidget *parent) : QObject (parent) {
	RK_TRACE (APP);
	RK_ASSERT (main_workplace == 0);

	main_workplace = this;
	wview = new RKWorkplaceView (parent);
}

RKWorkplace::~RKWorkplace () {
	RK_TRACE (APP);

//	closeAll ();	// not needed, as the windows will autodelete themselves using QObject mechanism. Of course, closeAll () should be called *before* quitting.
}

void RKWorkplace::attachWindow (RKMDIWindow *window) {
	RK_TRACE (APP);
	RK_ASSERT (windows.find (window) != windows.end ());		// This should not happen for now.

	window->state = RKMDIWindow::Attached;
	view ()->addPage (window);

	RK_ASSERT (window->getPart ());
	RKwardApp::getApp ()->partManager ()->addPart (window->getPart ());
}

void RKWorkplace::detachWindow (RKMDIWindow *window) {
	RK_TRACE (APP);
	RK_ASSERT (windows.find (window) != windows.end ());		// Can't detach a window that is not attached

	window->state = RKMDIWindow::Detached;

	RK_ASSERT (window->getPart ());
	RKwardApp::getApp ()->partManager ()->removePart (window->getPart ());
	view ()->removePage (window);

	DetachedWindowContainer *detached = new DetachedWindowContainer (window);
	detached->show ();
}

void RKWorkplace::addWindow (RKMDIWindow *window) {
	RK_TRACE (APP);

	windows.append (window);
	connect (window, SIGNAL (destroyed (QObject *)), this, SLOT (windowDestroyed (QObject *)));
	attachWindow (window);
}

bool RKWorkplace::openScriptEditor (const KURL &url, bool use_r_highlighting, bool read_only, const QString &force_caption) {
	RK_TRACE (APP);

	RKCommandEditorWindow *editor = new RKCommandEditorWindow (view (), use_r_highlighting);

	if (!url.isEmpty ()) {
		if (!editor->openURL (url, use_r_highlighting, read_only)) {
			delete editor;
			KMessageBox::messageBox (view (), KMessageBox::Error, i18n ("Unable to open \"%1\"").arg (url.prettyURL ()), i18n ("Could not open command file"));
			return false;
		}
	}

	if (!force_caption.isEmpty ()) editor->setCaption (force_caption);
	addWindow (editor);
	return true;
}

void RKWorkplace::openHelpWindow (const KURL &url) {
	RK_TRACE (APP);

	RKHelpWindow *hw = new RKHelpWindow (view ());
	if (!url.isEmpty ()) {
		hw->openURL (url);
	}

	addWindow (hw);
}

void RKWorkplace::openOutputWindow (const KURL &url) {
	RK_TRACE (APP);

	RKOutputWindow::refreshOutput (true, true);
	if (windows.find (RKOutputWindow::getCurrentOutput ()) == windows.end ()) {
		addWindow (RKOutputWindow::getCurrentOutput ());
	}
}

bool RKWorkplace::canEditObject (RObject *object) {
	RK_TRACE (APP);
	
	if (object->isDataFrame ()) {
		return true;
	} else if (object->isVariable () && object->getContainer ()->isDataFrame ()) {
		return true;
	}
	return false;
}

RKEditor *RKWorkplace::editObject (RObject *object, bool initialize_to_empty) {
	RK_TRACE (APP);

	RObject *iobj = object;
	RKEditor *ed = 0;
	RKEditorDataFramePart *part = 0;
	if (!object->objectOpened ()) {
		if (object->isDataFrame ()) {
			part = new RKEditorDataFramePart (0);		// TODO: reverse creation logic, just as in the other classes!
			ed = part->getEditor ();
			// TODO: add child objects, too?
			ed->openObject (object, initialize_to_empty);
		} else if (object->isVariable () && object->getContainer ()->isDataFrame ()) {
			if (!object->getContainer ()->objectOpened ()) { 
				iobj = object->getContainer ();
				part = new RKEditorDataFramePart (0);
				ed = part->getEditor ();
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
			ed->setCaption (iobj->getShortName ());		// TODO: move to editor
			ed->setIcon (SmallIcon ("spreadsheet"));
			addWindow (ed);
			ed->setFocus ();		// somehow we need to call this explicitely
		}
	} else {
		object->objectOpened ()->show ();
		object->objectOpened ()->raise ();
	}
	
	return ed;
}

void RKWorkplace::flushAllData () {
	RK_TRACE (APP);

	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if ((*it)->type == RKMDIWindow::DataEditorWindow) {
			static_cast<RKEditor *> (*it)->flushChanges ();
		}
	}
}

void RKWorkplace::closeWindow (RKMDIWindow *window) {
	RK_TRACE (APP);
	RK_ASSERT (windows.find (window) != windows.end ());

	window->close (true);		// all the rest should happen in windowDestroyed ()
}

void RKWorkplace::closeActiveWindow () {
	RK_TRACE (APP);

	RKMDIWindow *w = activeAttachedWindow ();
	if (w) closeWindow (w);
	else RK_ASSERT (false);		// this is benign, and maybe even ok, but I'd like to see when this happens
}

RKWorkplace::RKWorkplaceObjectList RKWorkplace::getObjectList (int type, int state) {
	RK_TRACE (APP);

	RKWorkplaceObjectList ret;
	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if (((*it)->type & type) && ((*it)->state & state)) {
			ret.append ((*it));
		}
	}
	return ret;
}

void RKWorkplace::closeAll (int type, int state) {
	RK_TRACE (APP);

	RKWorkplaceObjectList list_to_close = getObjectList (type, state);
	for (RKWorkplaceObjectList::const_iterator it = list_to_close.constBegin (); it != list_to_close.constEnd (); ++it) {
		closeWindow (*it);
	}
}

void RKWorkplace::windowDestroyed (QObject *object) {
	RK_TRACE (APP);
	RKMDIWindow *window = static_cast<RKMDIWindow *> (object);

	RK_ASSERT (windows.find (window) != windows.end ());
	view ()->removePage (window, true);
	windows.remove (window);
}

RKMDIWindow *RKWorkplace::activeAttachedWindow () {
	RK_TRACE (APP);

	return (static_cast<RKMDIWindow *> (view ()->activePage ()));
}

void RKWorkplace::activateWindow (RKMDIWindow *window) {
	RK_TRACE (APP);

	window->raise ();		// Does this do the trick?
}

void RKWorkplace::saveWorkplace (RCommandChain *chain) {
	RK_TRACE (APP);

	QString workplace_description = "c (";

	bool first = true;
	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if (first) first = false;
		else workplace_description.append (", ");
		workplace_description.append ((*it)->getRDescription ());
	}
	workplace_description = ".rk.workplace.save <- " + workplace_description + ")";


	RKGlobals::rInterface ()->issueCommand (workplace_description, RCommand::App | RCommand::Sync, i18n ("Save Workplace layout"), 0, 0, chain); 
}

void RKWorkplace::restoreWorkplace (RCommandChain *chain) {
	RK_TRACE (APP);

	RKGlobals::rInterface ()->issueCommand (".rk.workplace.save", RCommand::App | RCommand::Sync | RCommand::GetStringVector, i18n ("Restore Workplace layout"), this, RESTORE_WORKPLACE_COMMAND, chain);
}

void RKWorkplace::clearWorkplaceDescription (RCommandChain *chain) {
	RK_TRACE (APP);

	RKGlobals::rInterface ()->issueCommand ("remove (.rk.workplace.save)", RCommand::App | RCommand::Sync, QString::null, 0, 0, chain); 
}

void RKWorkplace::rCommandDone (RCommand *command) {
	RK_TRACE (APP);

	RK_ASSERT (command->getFlags () == RESTORE_WORKPLACE_COMMAND);
	for (unsigned int i = 0; i < command->getDataLength (); ++i) {
		QString desc = command->getStringVector ()[i];
		QString type = desc.section (QChar (':'), 0, 0);
		QString specification = desc.section (QChar (':'), 1);

		if (type == "data") {
			RObject *object = RObjectList::getObjectList ()->findObject (specification);
			if (object) editObject (object, false);
		} else if (type == "script") {
			openScriptEditor (specification);
		} else if (type == "output") {
			openOutputWindow (specification);
		} else if (type == "help") {
			openHelpWindow (specification);
		} else {
			RK_ASSERT (false);
		}
	}
}

#include "rkworkplace.moc"
