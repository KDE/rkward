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
#include "../rkglobals.h"
#include "../rkward.h"
#include "../rkeditormanager.h"

#include "../debug.h"

RKSaveAgent::RKSaveAgent (KURL url, bool save_file_as, bool quit_after_save) {
	RK_TRACE (APP);
	save_url = url;
	quit_when_done = quit_after_save;
	save_chain = 0;
	if (save_url.isEmpty () || save_file_as) {
		if (!askURL ()) return;
	}
	
	save_chain = RKGlobals::rInterface ()->startChain (0);
	RKGlobals::editorManager ()->syncAllToR (save_chain);
	
	RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + save_url.path () + "\")", RCommand::App, "", this), save_chain);
}

RKSaveAgent::~RKSaveAgent () {
	RK_TRACE (APP);
}

bool RKSaveAgent::askURL () {
	RK_TRACE (APP);
	save_url = KFileDialog::getSaveFileName (save_url.path (), "*.R");
	if (save_url.isEmpty ()) {
		if (quit_when_done) {
			if (KMessageBox::warningYesNo (0, i18n ("No filename given. Your data was NOT saved. Do you still want to quit?")) == KMessageBox::No) quit_when_done = false;
		}
		done ();
		return false;
	}
	return true;
}

void RKSaveAgent::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	if (command->hasError ()) {
		if (quit_when_done) {
			int res;
			res = KMessageBox::warningYesNoCancel (0, i18n ("Saving to file '") + save_url.path () + i18n ("' failed. What do you want to do?"), i18n ("Save failed"), KGuiItem (i18n ("Try saving with a different filename")), KGuiItem (i18n ("Quit without saving")));
			if (res == KMessageBox::Yes) {
				if (askURL ()) RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + save_url.path () + "\")", RCommand::App, "", this), save_chain);
				return;
			} else if (res == KMessageBox::No) {
				done ();
				return;
			} else {
				quit_when_done = false;
				done ();
				return;
			}
		} else {
			if (KMessageBox::warningYesNo (0, i18n ("Saving to file '") + save_url.path () + i18n ("' failed. Do you want to try saving to a different filename?")) == KMessageBox::Yes) {
				if (askURL ()) RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + save_url.path () + "\")", RCommand::App, "", this), save_chain);
				return;
			} else {
				done ();
				return;
			}
		}
	} else {
		RKGlobals::rkApp ()->setCaption (save_url.filename ());
		done ();
		return;
	}
}

void RKSaveAgent::done () {
	RK_TRACE (APP);
	if (save_chain) {
		RKGlobals::rInterface ()->closeChain (save_chain);
	}
	if (quit_when_done) {
		delete RKGlobals::rkApp ();
		qApp->quit ();
	} else {
		delete this;
	}
}

