/***************************************************************************
                          rksaveagent  -  description
                             -------------------
    begin                : Sun Aug 29 2004
    copyright            : (C) 2004, 2009, 2010, 2011, 2012 by Thomas Friedrichsmeier
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
#include "rksaveagent.h"

#include <KLocalizedString>
#include <kmessagebox.h>

#include <qapplication.h>
#include <QFileDialog>

#include "../rbackend/rinterface.h"
#include "../core/robjectlist.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../windows/rkworkplace.h"

#include "../debug.h"

RKSaveAgent::RKSaveAgent (QUrl url, bool save_file_as, DoneAction when_done, QUrl load_url) : QObject () {
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
	RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (" + RObject::rQuote (save_url.toLocalFile ()) + ')', RCommand::App, QString (), this), save_chain);
}

RKSaveAgent::~RKSaveAgent () {
	RK_TRACE (APP);
}

// We save to several files at once, meaning the standard overwrite check is not quite good enough for us.
// More importantly, it is entirely broken in KF5 < 5.22.0 (https://bugs.kde.org/show_bug.cgi?id=360666)
// So check for overwriting ourselves.
bool checkOverwriteWorkspace (QUrl url, QWidget *parent) {
	if (url.isEmpty () || !url.isLocalFile ()) {
		return true;
	}

	QString mainfile = url.toLocalFile ();
//	QString addfile = mainfile.left (mainfile.lastIndexOf ('.')) + ".rkward";
	QString addfile = mainfile + ".rkworkplace";
	QFileInfo info (mainfile);
	if (!info.exists ()) mainfile.clear (); // signifies: not a problem
	else mainfile = info.fileName ();

	info.setFile (addfile);
	if (!info.exists ()) addfile.clear ();
	else addfile = info.fileName ();

	if (mainfile.isEmpty () && addfile.isEmpty ()) {
		return true;
	}

	QString warning;
	if (addfile.isEmpty ()) {
		warning = i18n ("A file named \"%1\" already exists. Are you sure you want to overwrite it?", mainfile);
	} else if (mainfile.isEmpty ()) {
		warning = i18n ("A file named \"%1\" already exists, and will be overwritten when saving to \"%2\". Are you sure you want to overwrite it?", addfile, mainfile);
	} else {
		warning = i18n ("Files named \"%1\" and \"%2\" already exist, and will both be overwritten. Are you sure you want to overwrite them?", mainfile, addfile);
	}

	return KMessageBox::Cancel != KMessageBox::warningContinueCancel (parent, warning, i18n ("Overwrite File?"), KStandardGuiItem::overwrite (),
	                                                                  KStandardGuiItem::cancel (), QString (), KMessageBox::Options (KMessageBox::Notify | KMessageBox::Dangerous));
}

bool RKSaveAgent::askURL () {
	RK_TRACE (APP);
	save_url = QUrl::fromLocalFile (QFileDialog::getSaveFileName (RKWardMainWindow::getMain (), QString (), save_url.toLocalFile (), i18n ("R Workspace Files [%1](%1);;All files [*](*)", RKSettingsModuleGeneral::workspaceFilenameFilter ()), 0, QFileDialog::DontConfirmOverwrite));
	if (!checkOverwriteWorkspace (save_url, RKWardMainWindow::getMain ())) save_url.clear ();

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
				RKGlobals::rInterface ()->issueCommand (new RCommand ("save.image (\"" + save_url.toLocalFile () + "\")", RCommand::App, QString (), this), save_chain);
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
	RKWardMainWindow::getMain ()->setWorkspaceMightBeModified (false);
	if (save_chain) {
		RKGlobals::rInterface ()->closeChain (save_chain);
	}
	if (when_done == Load) {
		RKWardMainWindow::getMain ()->askOpenWorkspace (load_url);
	}
	deleteLater ();
}
