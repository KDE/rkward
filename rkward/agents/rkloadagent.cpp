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
#include "../rkward.h"
#include "../windows/rkworkplace.h"

#include "../debug.h"

#define WORKSPACE_LOAD_COMMAND 1

RKLoadAgent::RKLoadAgent (const KURL &url, bool merge) {
	RK_TRACE (APP);
	RKwardApp::getApp ()->slotSetStatusBarText (i18n ("Loading Workspace ..."));

#if !KDE_IS_VERSION (3, 2, 0)
	KIO::NetAccess::download (url, tmpfile);
#else
	KIO::NetAccess::download (url, tmpfile, RKwardApp::getApp ());
#endif

	RCommand *command;
	
	if (!merge) {
		RKwardApp::getApp ()->slotCloseAllEditors ();
		command = new RCommand ("remove (list=ls (all.names=TRUE))", RCommand::App | RCommand::ObjectListUpdate);
		RKGlobals::rInterface ()->issueCommand (command);
	}

	command = new RCommand ("load (\"" + url.path () + "\")", RCommand::App | RCommand::ObjectListUpdate, QString::null, this, WORKSPACE_LOAD_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command);

	connect (RObjectList::getObjectList (), SIGNAL (updateComplete ()), this, SLOT (listUpdateComplete ()));
	
	RObjectList::getObjectList ()->setWorkspaceURL (url);

	RKWorkplace::mainWorkplace ()->restoreWorkplace ();
	RKWorkplace::mainWorkplace ()->clearWorkplaceDescription ();
}

RKLoadAgent::~RKLoadAgent () {
	RK_TRACE (APP);
}

void RKLoadAgent::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	
	if (command->getFlags () == WORKSPACE_LOAD_COMMAND) {
		KIO::NetAccess::removeTempFile (tmpfile);
		if (command->failed ()) {
			KMessageBox::error (0, i18n ("There has been an error opening file '%1':\n%2").arg (RObjectList::getObjectList ()->getWorkspaceURL ().path ()).arg (command->error ()), i18n ("Error loading workspace"));
			RObjectList::getObjectList ()->setWorkspaceURL (QString::null);
		}
		RKwardApp::getApp ()->slotSetStatusReady ();
		RKwardApp::getApp ()->setCaption (QString::null);	// trigger update of caption
	}
}

void RKLoadAgent::listUpdateComplete () {
	RK_TRACE (APP);

	delete this;
}

#include "rkloadagent.moc"
