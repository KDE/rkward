/***************************************************************************
                          rkloadagent  -  description
                             -------------------
    begin                : Sun Sep 5 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
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

RKLoadAgent::RKLoadAgent (const KUrl &url, bool merge) {
	RK_TRACE (APP);
	RKWardMainWindow::getMain ()->slotSetStatusBarText (i18n ("Loading Workspace ..."));

	QString filename;
	if (!url.isLocalFile ()) {
#if !KDE_IS_VERSION (3, 2, 0)
		KIO::NetAccess::download (url, tmpfile);
#else
		KIO::NetAccess::download (url, tmpfile, RKWardMainWindow::getMain ());
#endif
		filename = tmpfile;
	} else {
		filename = url.path ();
	}
	

	RCommand *command;
	
	if (!merge) {
		RKWardMainWindow::getMain ()->slotCloseAllEditors ();
		command = new RCommand ("remove (list=ls (all.names=TRUE))", RCommand::App | RCommand::ObjectListUpdate);
		RKGlobals::rInterface ()->issueCommand (command);
	}

	command = new RCommand ("load (\"" + filename + "\")", RCommand::App | RCommand::ObjectListUpdate, QString::null, this, WORKSPACE_LOAD_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command);

	RObjectList::getObjectList ()->setWorkspaceURL (url);
}

RKLoadAgent::~RKLoadAgent () {
	RK_TRACE (APP);
}

void RKLoadAgent::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	
	if (command->getFlags () == WORKSPACE_LOAD_COMMAND) {
		if (!tmpfile.isEmpty ()) KIO::NetAccess::removeTempFile (tmpfile);
		if (command->failed ()) {
			KMessageBox::error (0, i18n ("There has been an error opening file '%1':\n%2").arg (RObjectList::getObjectList ()->getWorkspaceURL ().path ()).arg (command->error ()), i18n ("Error loading workspace"));
			RObjectList::getObjectList ()->setWorkspaceURL (QString::null);
		} else {
			RKWorkplace::mainWorkplace ()->restoreWorkplace ();
			RKWorkplace::mainWorkplace ()->clearWorkplaceDescription ();
		}
		RKWardMainWindow::getMain ()->slotSetStatusReady ();
		RKWardMainWindow::getMain ()->setCaption (QString::null);	// trigger update of caption

		delete this;
		return;
	} else {
		RK_ASSERT (false);
	}
}

#include "rkloadagent.moc"
