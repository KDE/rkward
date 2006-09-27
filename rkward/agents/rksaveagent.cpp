/***************************************************************************
                          rksaveagent  -  description
                             -------------------
    begin                : Sun Aug 29 2004
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
#include "rksaveagent.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <qapplication.h>

#include "../rbackend/rinterface.h"
#include "../core/robjectlist.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../windows/rkworkplace.h"

#include "../debug.h"

RKSaveAgent::RKSaveAgent (KURL url, bool save_file_as, DoneAction when_done, KURL load_url) {
	RK_TRACE (APP);
	save_url = url;
	RKSaveAgent::when_done = when_done;
	RKSaveAgent::load_url = load_url;
	save_chain = 0;
	if (save_url.isEmpty () || save_file_as) {
		if (!askURL ()) return;
	}
	
	RKWorkplace::mainWorkplace ()->flushAllData ();
	save_chain = RKGlobals::rInterface ()->startChain (0);
	
	RKWorkplace::mainWorkplace ()->saveWorkplace (save_chain);
	RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + save_url.path () + "\")", RCommand::App, QString::null, this), save_chain);
	RKWorkplace::mainWorkplace ()->clearWorkplaceDescription (save_chain);
}

RKSaveAgent::~RKSaveAgent () {
	RK_TRACE (APP);
}

bool RKSaveAgent::askURL () {
	RK_TRACE (APP);
	save_url = KFileDialog::getSaveFileName (save_url.path (), "*.R");
	if (save_url.isEmpty ()) {
		if (when_done != DoNothing) {
			if (KMessageBox::warningYesNo (0, i18n ("No filename given. Your data was NOT saved. Do you still want to proceed?")) == KMessageBox::No) when_done = DoNothing;
		}
		done ();
		return false;
	}
	return true;
}

void RKSaveAgent::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	if (command->hasError ()) {
		if (when_done != DoNothing) {
			int res;
			res = KMessageBox::warningYesNoCancel (0, i18n ("Saving to file '%1' failed. What do you want to do?").arg (save_url.path ()), i18n ("Save failed"), KGuiItem (i18n ("Try saving with a different filename")), KGuiItem (i18n ("Saving failed")));
			if (res == KMessageBox::Yes) {
				if (askURL ()) RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + save_url.path () + "\")", RCommand::App, QString::null, this), save_chain);
				return;
			} else if (res == KMessageBox::No) {
				done ();
				return;
			} else {
				when_done = DoNothing;
				done ();
				return;
			}
		} else {
			if (KMessageBox::warningYesNo (0, i18n ("Saving to file '%1' failed. Do you want to try saving to a different filename?").arg (save_url.path ())) == KMessageBox::Yes) {
				if (askURL ()) RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + save_url.path () + "\")", RCommand::App, QString::null, this), save_chain);
				return;
			} else {
				done ();
				return;
			}
		}
	} else {
		RKGlobals::rObjectList ()->setWorkspaceURL (save_url);
		RKwardApp::getApp ()->setCaption (QString::null);	// trigger update of caption
		done ();
		return;
	}
}

void RKSaveAgent::done () {
	RK_TRACE (APP);
	if (save_chain) {
		RKGlobals::rInterface ()->closeChain (save_chain);
	}
	if (when_done == Quit) {
		delete RKwardApp::getApp ();
		qApp->quit ();
	} else if (when_done == Load) {
		RKwardApp::getApp ()->fileOpenNoSave (load_url);
		delete this;
	} else {
		delete this;
	}
}

