/***************************************************************************
                          rkloadagent  -  description
                             -------------------
    begin                : Sun Sep 5 2004
    copyright            : (C) 2004, 2007, 2009, 2011, 2012, 2014 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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

#include <KLocalizedString>
#include <kmessagebox.h>
#include <kdeversion.h>
#include <kio/filecopyjob.h>
#include <KJobWidgets>
#include <KJobUiDelegate>

#include <qstring.h>
#include <QTemporaryFile>

#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../rbackend/rinterface.h"
#include "../rkward.h"
#include "../windows/rkworkplace.h"
#include "../settings/rksettingsmodulegeneral.h"

#include "../debug.h"

#define WORKSPACE_LOAD_COMMAND 1
#define WORKSPACE_LOAD_COMPLETE_COMMAND 2

RKLoadAgent::RKLoadAgent (const QUrl &url, bool merge) {
	RK_TRACE (APP);
	RKWardMainWindow::getMain ()->slotSetStatusBarText (i18n ("Loading Workspace..."));

	_merge = merge;

	// downlad the file, if remote
	tmpfile = 0;
	QString filename;
	if (!url.isLocalFile ()) {
		tmpfile = new QTemporaryFile (this);
		KIO::Job* getjob = KIO::file_copy (url, QUrl::fromLocalFile (tmpfile->fileName()));
		KJobWidgets::setWindow (getjob, RKWardMainWindow::getMain ());
		if (!getjob->exec ()) {
			getjob->ui ()->showErrorMessage();
			return;
		}
		filename = tmpfile->fileName ();
	} else {
		filename = url.toLocalFile ();
	}

	RCommand *command;
	
	if (!merge) {
		RKWardMainWindow::getMain ()->slotCloseAllWindows ();
		command = new RCommand ("remove (list=ls (all.names=TRUE))", RCommand::App | RCommand::ObjectListUpdate);
		RKGlobals::rInterface ()->issueCommand (command);
	}

	command = new RCommand ("load (\"" + filename + "\")", RCommand::App | RCommand::ObjectListUpdate, QString (), this, WORKSPACE_LOAD_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command);

	RKWorkplace::mainWorkplace ()->setWorkspaceURL (url);
}

RKLoadAgent::~RKLoadAgent () {
	RK_TRACE (APP);
}

void RKLoadAgent::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	
	if (command->getFlags () == WORKSPACE_LOAD_COMMAND) {
		if (command->failed ()) {
			KMessageBox::error (0, i18n ("There has been an error opening file '%1':\n%2", RKWorkplace::mainWorkplace ()->workspaceURL ().path (), command->error ()), i18n ("Error loading workspace"));
			RKWorkplace::mainWorkplace ()->setWorkspaceURL (QUrl());
		} else {
			RKWorkplace::mainWorkplace ()->restoreWorkplace (0, _merge);
			if (RKSettingsModuleGeneral::cdToWorkspaceOnLoad ()) {
				if (RKWorkplace::mainWorkplace ()->workspaceURL ().isLocalFile ()) {
					RKGlobals::rInterface ()->issueCommand ("setwd (" + RObject::rQuote (RKWorkplace::mainWorkplace ()->workspaceURL ().adjusted (QUrl::RemoveFilename).path ()) + ')', RCommand::App);
				}
			}
			RKGlobals::rInterface ()->issueCommand (QString (), RCommand::EmptyCommand | RCommand::App, QString (), this, WORKSPACE_LOAD_COMPLETE_COMMAND);
		}
		RKWardMainWindow::getMain ()->setCaption (QString ());	// trigger update of caption
	} else if (command->getFlags () == WORKSPACE_LOAD_COMPLETE_COMMAND) {
		RKWardMainWindow::getMain ()->slotSetStatusReady ();
		RKWardMainWindow::getMain ()->setWorkspaceMightBeModified (false);

		delete this;
		return;
	} else {
		RK_ASSERT (false);
	}
}

