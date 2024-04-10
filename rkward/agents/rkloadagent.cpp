/*
rkloadagent - This file is part of RKWard (https://rkward.kde.org). Created: Sun Sep 5 2004
SPDX-FileCopyrightText: 2004-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkloadagent.h"

#include <KLocalizedString>
#include <kmessagebox.h>
#include <kio/filecopyjob.h>
#include <KJobWidgets>
#include <KJobUiDelegate>

#include <qstring.h>
#include <QTemporaryFile>

#include "../core/robjectlist.h"
#include "../misc/rkoutputdirectory.h"
#include "../rbackend/rkrinterface.h"
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
	tmpfile = nullptr;
	QString filename;
	if (!url.isLocalFile ()) {
		tmpfile = new QTemporaryFile (this);
		KIO::Job* getjob = KIO::file_copy (url, QUrl::fromLocalFile (tmpfile->fileName()));
		KJobWidgets::setWindow (getjob, RKWardMainWindow::getMain ());
		if (!getjob->exec ()) {
			getjob->uiDelegate ()->showErrorMessage();
			return;
		}
		filename = tmpfile->fileName ();
	} else {
		filename = url.toLocalFile ();
	}

	RCommand *command;
	
	if (!merge) {
		command = new RCommand ("remove (list=ls (all.names=TRUE))", RCommand::App | RCommand::ObjectListUpdate);
		RInterface::issueCommand (command);
	}

	command = new RCommand ("load (\"" + filename + "\")", RCommand::App | RCommand::ObjectListUpdate);
	command->whenFinished(this, [this](RCommand* command) {
		if (command->failed()) {
			KMessageBox::error(nullptr, i18n("There has been an error opening file '%1':\n%2", RKWorkplace::mainWorkplace()->workspaceURL().path(), command->warnings() + command->error()), i18n("Error loading workspace"));
			RKWorkplace::mainWorkplace()->setWorkspaceURL(QUrl());
		} else {
			RKWorkplace::mainWorkplace()->restoreWorkplace(nullptr, _merge);
			if (RKSettingsModuleGeneral::cdToWorkspaceOnLoad ()) {
				if (RKWorkplace::mainWorkplace ()->workspaceURL ().isLocalFile ()) {
					RInterface::issueCommand ("setwd (" + RObject::rQuote (RKWorkplace::mainWorkplace ()->workspaceURL ().adjusted (QUrl::RemoveFilename).path ()) + ')', RCommand::App);
				}
			}
		}

		RInterface::whenAllFinished(this, [this]() {
			RKWardMainWindow::getMain()->slotSetStatusReady();
			RKWardMainWindow::getMain()->setWorkspaceMightBeModified(false);
			RKOutputDirectory::getCurrentOutput();  // make sure some output file exists

			deleteLater();
		});
		RKWardMainWindow::getMain ()->setCaption (QString ());	// trigger update of caption
	});
	RInterface::issueCommand (command);

	RKWorkplace::mainWorkplace ()->setWorkspaceURL (url);
}

RKLoadAgent::~RKLoadAgent () {
	RK_TRACE (APP);
}
