/***************************************************************************
                          rksaveagent  -  description
                             -------------------
    begin                : Sun Aug 29 2004
    copyright            : (C) 2004, 2009, 2010, 2011, 2012 by Thomas Friedrichsmeier
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
#include "rksaveagent.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <qapplication.h>

#include "../rbackend/rinterface.h"
#include "../core/robjectlist.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../windows/rkworkplace.h"

#include "../debug.h"

RKSaveAgent::RKSaveAgent (KUrl url, bool save_file_as, DoneAction when_done, KUrl load_url) : QObject () {
	RK_TRACE (APP);
	save_url = url;
	RKSaveAgent::when_done = when_done;
	RKSaveAgent::load_url = load_url;
	previous_url = RKWorkplace::mainWorkplace ()->workspaceURL ();
	save_chain = 0;
	if (save_url.isEmpty () || save_file_as) {
		if (!askURL ()) {
			deleteLater();
			return;
		}
	}
	
	RKWorkplace::mainWorkplace ()->flushAllData ();
	save_chain = RKGlobals::rInterface ()->startChain (0);
	
	RKWorkplace::mainWorkplace ()->setWorkspaceURL (save_url, true);
	RKWorkplace::mainWorkplace ()->saveWorkplace (save_chain);
	RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + save_url.toLocalFile () + "\")", RCommand::App, QString::null, this), save_chain);
}

RKSaveAgent::~RKSaveAgent () {
	RK_TRACE (APP);
}

bool RKSaveAgent::askURL () {
	RK_TRACE (APP);
	save_url = KFileDialog::getSaveFileName (save_url, i18n ("%1|R Workspace Files (%1)\n*|All files", RKSettingsModuleGeneral::workspaceFilenameFilter ()));
	if (save_url.isEmpty ()) {
		if (when_done != DoNothing) {
			if (KMessageBox::warningYesNo (0, i18n ("No filename given. Your data was NOT saved. Do you still want to proceed?")) != KMessageBox::Yes) when_done = DoNothing;
		}
		return false;
	}
	return true;
}

void RKSaveAgent::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	if (command->hasError ()) {
		RKWorkplace::mainWorkplace ()->setWorkspaceURL (previous_url);

		int res;
		if (when_done != DoNothing) {
			res = KMessageBox::warningYesNoCancel (0, i18n ("Saving to file '%1' failed. What do you want to do?", save_url.path ()), i18n ("Save failed"), KGuiItem (i18n ("Try saving with a different filename")), KGuiItem (i18n ("Saving failed")));
		} else {
			res = KMessageBox::warningYesNo (0, i18n ("Saving to file '%1' failed. Do you want to try saving to a different filename?", save_url.path ()));
		}

		if (res == KMessageBox::Yes) {
			if (askURL ()) {
				RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + save_url.toLocalFile () + "\")", RCommand::App, QString::null, this), save_chain);
				return;
			}
		} else if (res == KMessageBox::No) {
			done ();
			return;
		}

		// else
		when_done = DoNothing;
	}
	done ();
}

void RKSaveAgent::done () {
	RK_TRACE (APP);
	if (save_chain) {
		RKGlobals::rInterface ()->closeChain (save_chain);
	}
	if (when_done == Load) {
		RKWardMainWindow::getMain ()->fileOpenNoSave (load_url);
	}
	deleteLater ();
}
