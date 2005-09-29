/***************************************************************************
                          rkloadagent  -  description
                             -------------------
    begin                : Sun Sep 5 2004
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
#include "rkloadagent.h"

#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdeversion.h>

#include <qstring.h>

#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../rbackend/rinterface.h"
#include "../rkeditormanager.h"
#include "../rkward.h"

#include "../debug.h"

#define WORKSPACE_LOAD_COMMAND 1

RKLoadAgent::RKLoadAgent (const KURL &url, bool merge) {
	RK_TRACE (APP);
	RKGlobals::rkApp ()->setCaption (i18n ("Loading Workspace ..."), false);

#if !KDE_IS_VERSION (3, 2, 0)
	KIO::NetAccess::download (url, tmpfile);
#else
	KIO::NetAccess::download (url, tmpfile, RKGlobals::rkApp ());
#endif

	update_was_delete = false;
	RCommand *command;
	
	if (!merge) {
		RKGlobals::rkApp ()->slotCloseAllEditors ();
		update_was_delete = true;
		RKGlobals::rObjectList ()->updateFromR ();
		command = new RCommand ("remove (list=ls (all.names=TRUE))", RCommand::App);
		RKGlobals::rInterface ()->issueCommand (command);
	}

	command = new RCommand ("load (\"" + url.path () + "\")", RCommand::App, QString::null, this, WORKSPACE_LOAD_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command);

	connect (RKGlobals::rObjectList (), SIGNAL (updateComplete ()), this, SLOT (listUpdateComplete ()));
	
	RKGlobals::rObjectList ()->setWorkspaceURL (url);
}

RKLoadAgent::~RKLoadAgent () {
	RK_TRACE (APP);
}

void RKLoadAgent::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	
	if (command->getFlags () == WORKSPACE_LOAD_COMMAND) {
		KIO::NetAccess::removeTempFile (tmpfile);
		if (command->failed ()) {
			KMessageBox::error (0, i18n ("There has been an error opening file '%1':\n%2").arg (RKGlobals::rObjectList ()->getWorkspaceURL ().path ()).arg (command->error ()), i18n ("Error loading workspace"));
			RKGlobals::rkApp ()->setCaption (i18n ("Untitled"), false);
		} else {
			RKGlobals::rkApp ()->setCaption (RKGlobals::rObjectList ()->getWorkspaceURL ().filename (), false);
		}
	}
}

void RKLoadAgent::listUpdateComplete () {
	RK_TRACE (APP);
	if (update_was_delete) {
		update_was_delete = false;
		RKGlobals::rObjectList ()->updateFromR ();
		return;
	}
	RKGlobals::editorManager ()->restoreEditors ();
	deleteThis ();
}
