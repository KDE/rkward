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

#include "../windows/detachedwindowcontainer.h"
#include "../windows/rkcommandeditorwindow.h"
#include "../windows/rkhtmlwindow.h"
#include "../core/robject.h"
#include "../core/rcontainerobject.h"
#include "../core/robjectlist.h"
#include "../dataeditor/rkeditor.h"
#include "../dataeditor/rkeditordataframe.h"
#include "../dataeditor/rkeditordataframepart.h"

#include "../debug.h"


RKWorkplace::RKWorkplace (QWidget *parent) : QObject (parent) {
	RK_TRACE (APP);

	wview = new RKWorkplaceView (parent);
	connect (wview, SIGNAL (currentChanged (QWidget *)), this, SLOT (activeAttachedChanged (QWidget *)));
	part_manager = new KParts::PartManager (view ());
}

RKWorkplace::~RKWorkplace () {
	RK_TRACE (APP);

	closeAll ();
}

void RKWorkplace::attachWindow (QWidget *window) {
	RK_TRACE (APP);
	RK_ASSERT (windows.find (window) != windows.end ());		// This should not happen for now.

	RKWorkplaceObjectInfo *info = windows[window];
	info->state = Attached;

	window->reparent (view (), QPoint (0, 0));
	view ()->addTab (window, window->caption ());
	RK_ASSERT (info->part);
	part_manager->addPart (info->part);
}

void RKWorkplace::detachWindow (QWidget *window) {
	RK_TRACE (APP);
	RK_ASSERT (windows.find (window) != windows.end ());		// Can't detach a window that is not attached

	RKWorkplaceObjectInfo *info = windows[window];
	info->state = Detached;

	RK_ASSERT (info->part);
	part_manager->removePart (info->part);
	view ()->removePage (window);

	DetachedWindowContainer *detached = new DetachedWindowContainer (info->part, window);
	detached->show ();
}

void RKWorkplace::addWindow (QWidget *window, RKWorkplaceObjectType type) {
	RK_TRACE (APP);

	connect (window, SIGNAL (destroyed (QWidget *)), this, SLOT (windowDestroyed (QWidget *)));
	// TODO: most views do not emit that signal, yet
	connect (window, SIGNAL (captionChanged (QWidget *)), this, SLOT (updateWindowCaption (QWidget *)));

	RKWorkplaceObjectInfo *info = new RKWorkplaceObjectInfo;
	info->part = 0;
	info->state = Attached;
	info->type = type;
	windows.insert (window, info);

	view ()->addTab (window, window->caption ());
}

bool RKWorkplace::openScriptEditor (const KURL &url) {
	RK_TRACE (APP);

	RKCommandEditorWindow *editor = new RKCommandEditorWindow (view ());

	if (!url.isEmpty ()) {
		if (!editor->openURL (url)) {
			delete editor;
			KMessageBox::messageBox (view (), KMessageBox::Error, i18n ("Unable to open \"%1\"").arg (url.prettyURL ()), i18n ("Could not open command file"));
			return false;
		}
	}

	addWindow (editor, CommandEditorWindow);
	return true;
}

void RKWorkplace::openHelpWindow (const KURL &url) {
	RK_TRACE (APP);

	RKHelpWindow *hw = new RKHelpWindow (view ());
	if (!url.isEmpty ()) {
		hw->openURL (url);
	}

	addWindow (hw, HelpWindow);
}

void RKWorkplace::openOutputWindow (const KURL &url) {
	RK_TRACE (APP);

	RKOutputWindow::refreshOutput (true, true);
	if (windows.find (RKOutputWindow::getCurrentOutput ()) == windows.end ()) {
		addWindow (RKOutputWindow::getCurrentOutput (), OutputWindow);
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
			addWindow (ed, DataEditorWindow);
			ed->setIcon (SmallIcon ("spreadsheet"));
			ed->setFocus ();		// somehow we need to call this explicitely
			registerPart (ed, part);
		}
	} else {
		object->objectOpened ()->show ();
		object->objectOpened ()->raise ();
	}
	
	return ed;
}

void RKWorkplace::flushAllData () {
	RK_TRACE (APP);

	for (RKWorkplaceObjectMap::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if (it.data ()->type == DataEditorWindow) {
			static_cast<RKEditor *> (it.key ())->flushChanges ();
		}
	}
}

void RKWorkplace::closeWindow (QWidget *window) {
	RK_TRACE (APP);
	RK_ASSERT (windows.find (window) != windows.end ());

	delete window;		// all the rest should happen in windowDestroyed ()
}

void RKWorkplace::closeAll (int type, int state) {
	RK_TRACE (APP);

	QValueList<QWidget *> list_to_close;
	for (RKWorkplaceObjectMap::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if ((it.data ()->type & type) && (it.data ()->state & state)) {
			list_to_close.append (it.key ());		// can't inline deletion
		}
	}

	for (QValueList<QWidget *>::const_iterator it = list_to_close.constBegin (); it != list_to_close.constEnd (); ++it) {
		closeWindow (*it);
	}
}

void RKWorkplace::windowDestroyed (QWidget *window) {
	RK_TRACE (APP);

	RK_ASSERT (windows.find (window) != windows.end ());
	delete windows[window];
	windows.remove (window);
}

void RKWorkplace::updateWindowCaption (QWidget *window) {
	RK_TRACE (APP);

	RK_ASSERT (windows.find (window) != windows.end ());
	view ()->changeTab (window, window->caption ());
	if (window == activeAttachedWindow ()) emit (changeCaption (window->caption ()));
}

void RKWorkplace::activeAttachedChanged (QWidget *window) {
	RK_TRACE (APP);
	RK_ASSERT (window);

	emit (changeCaption (window->caption ()));
}

QWidget *RKWorkplace::activeAttachedWindow () {
	RK_TRACE (APP);

	return (view ()->currentPage ());
}

void RKWorkplace::activateWindow (QWidget *window) {
	RK_TRACE (APP);

	window->raise ();		// Does this do the trick?
}


void RKWorkplace::saveWorkplace (RCommandChain *chain) {
	RK_TRACE (APP);

	// TODO
}

void RKWorkplace::restoreWorkplace (RCommandChain *chain) {
	RK_TRACE (APP);

	// TODO
}

void RKWorkplace::rCommandDone (RCommand *command) {
}

#include "rkworkplace.moc"
