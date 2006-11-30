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
#include "../settings/rksettingsmoduleoutput.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../rbackend/rinterface.h"
#include "../windows/rkwindowcatcher.h"
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

	window->prepareToBeAttached ();
	window->state = RKMDIWindow::Attached;
	view ()->addPage (window);

	RK_ASSERT (window->getPart ());
	RKWardMainWindow::getMain ()->partManager ()->addPart (window->getPart ());
}

void RKWorkplace::detachWindow (RKMDIWindow *window, bool was_attached) {
	RK_TRACE (APP);
	RK_ASSERT (windows.find (window) != windows.end ());		// Can't detach a window that is not registered

	window->prepareToBeDetached ();
	window->state = RKMDIWindow::Detached;

	RK_ASSERT (window->getPart ());
	if (was_attached) {
		RKWardMainWindow::getMain ()->partManager ()->removePart (window->getPart ());
		view ()->removePage (window);
	}

	DetachedWindowContainer *detached = new DetachedWindowContainer (window);
	detached->show ();
}

void RKWorkplace::addWindow (RKMDIWindow *window, bool attached) {
	RK_TRACE (APP);

	windows.append (window);
	connect (window, SIGNAL (destroyed (QObject *)), this, SLOT (windowDestroyed (QObject *)));
	if (attached) attachWindow (window);
	else detachWindow (window, false);
}

bool RKWorkplace::openScriptEditor (const KURL &url, bool use_r_highlighting, bool read_only, const QString &force_caption) {
	RK_TRACE (APP);

// is this url already opened?
	if (!url.isEmpty ()) {
		for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
			if ((*it)->type == RKMDIWindow::CommandEditorWindow) {
				KURL ourl = static_cast<RKCommandEditorWindow *> (*it)->url ();
				if (url.equals (ourl, true)) {
					(*it)->activate ();
					return true;
				}
			}
		}
	}

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

	RKOutputWindow::refreshOutput (true, true, false);
	if (windows.find (RKOutputWindow::getCurrentOutput ()) == windows.end ()) {
		addWindow (RKOutputWindow::getCurrentOutput ());
	}
}

void RKWorkplace::newOutput (bool only_if_modified) {
	RK_TRACE (APP);
	RKOutputWindow *window = RKOutputWindow::refreshOutput (RKSettingsModuleOutput::autoShow (), RKSettingsModuleOutput::autoRaise (), only_if_modified);
	if (window) {
		if (windows.find (window) == windows.end ()) {
			addWindow (window);
		}
	}
}

void RKWorkplace::newX11Window (WId window_to_embed, int device_number) {
	RK_TRACE (APP);

	RKCaughtX11Window *window = new RKCaughtX11Window (window_to_embed, device_number);
	window->state = RKMDIWindow::Detached;
	addWindow (window, false);
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
	RKEditor *existing_editor = object->objectOpened ();
	if (!existing_editor) {
		if (object->isDataFrame ()) {
			part = new RKEditorDataFramePart (0);		// TODO: reverse creation logic, just as in the other classes!
			ed = part->getEditor ();
			// TODO: add child objects, too?
			ed->openObject (object, initialize_to_empty);
		} else if (object->isVariable () && object->getContainer ()->isDataFrame ()) {
			existing_editor = object->getContainer ()->objectOpened ();
			if (!existing_editor) {
				iobj = object->getContainer ();
				part = new RKEditorDataFramePart (0);
				ed = part->getEditor ();
				// TODO: add child objects, too?
				ed->openObject (iobj, initialize_to_empty);
				// ed->focusObject (obj);
			}
		}

		if (ed) {
			ed->setCaption (iobj->getShortName ());		// TODO: move to editor
			ed->setIcon (SmallIcon ("spreadsheet"));
			addWindow (ed);
			ed->setFocus ();		// somehow we need to call this explicitely
		}
	}

	if (existing_editor) {		// not strictly an else. existing_editor may be reset inside the above if
		existing_editor->activate ();
		ed = existing_editor;
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
	if (window->isAttached ()) view ()->removePage (window, true);
	windows.remove (window);
}

RKMDIWindow *RKWorkplace::activeAttachedWindow () {
	RK_TRACE (APP);

	return (static_cast<RKMDIWindow *> (view ()->activePage ()));
}

QString RKWorkplace::makeWorkplaceDescription () {
	RK_TRACE (APP);

	QString workplace_description;
	bool first = true;
	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if (first) first = false;
		else workplace_description.append ("\n");
		workplace_description.append ((*it)->getDescription ());
	}
	return workplace_description;
}

void RKWorkplace::saveWorkplace (RCommandChain *chain) {
	RK_TRACE (APP);
	if (RKSettingsModuleGeneral::workplaceSaveMode () != RKSettingsModuleGeneral::SaveWorkplaceWithWorkspace) return;

	QString workplace_description = "c (";

	bool first = true;
	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if (first) first = false;
		else workplace_description.append (", ");
		workplace_description.append (RObject::rQuote ((*it)->getDescription ()));
	}
	workplace_description = ".rk.workplace.save <- " + workplace_description + ")";

	RKGlobals::rInterface ()->issueCommand (workplace_description, RCommand::App | RCommand::Sync, i18n ("Save Workplace layout"), 0, 0, chain); 
}

void RKWorkplace::restoreWorkplace (RCommandChain *chain) {
	RK_TRACE (APP);
	if (RKSettingsModuleGeneral::workplaceSaveMode () != RKSettingsModuleGeneral::SaveWorkplaceWithWorkspace) return;

	RKGlobals::rInterface ()->issueCommand (".rk.workplace.save", RCommand::App | RCommand::Sync | RCommand::GetStringVector, i18n ("Restore Workplace layout"), this, RESTORE_WORKPLACE_COMMAND, chain);
}

void RKWorkplace::restoreWorkplace (const QString &description) {
	RK_TRACE (APP);

	QStringList list = QStringList::split ("\n", description);
	for (QStringList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
		restoreWorkplaceItem (*it);
	}
}

void RKWorkplace::clearWorkplaceDescription (RCommandChain *chain) {
	RK_TRACE (APP);
	if (RKSettingsModuleGeneral::workplaceSaveMode () != RKSettingsModuleGeneral::SaveWorkplaceWithWorkspace) return;

	RKGlobals::rInterface ()->issueCommand ("remove (.rk.workplace.save)", RCommand::App | RCommand::Sync | RCommand::ObjectListUpdate, QString::null, 0, 0, chain); 
}

void RKWorkplace::rCommandDone (RCommand *command) {
	RK_TRACE (APP);

	RK_ASSERT (command->getFlags () == RESTORE_WORKPLACE_COMMAND);
	for (unsigned int i = 0; i < command->getDataLength (); ++i) {
		restoreWorkplaceItem (command->getStringVector ()[i]);
	}
}

void RKWorkplace::restoreWorkplaceItem (const QString &desc) {
	RK_TRACE (APP);

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


#include "rkworkplace.moc"
