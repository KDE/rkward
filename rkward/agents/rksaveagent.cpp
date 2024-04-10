/*
rksaveagent - This file is part of RKWard (https://rkward.kde.org). Created: Sun Aug 29 2004
SPDX-FileCopyrightText: 2004-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksaveagent.h"

#include <KLocalizedString>
#include <kmessagebox.h>

#include <qapplication.h>
#include <QFileDialog>

#include "../rbackend/rkrinterface.h"
#include "../misc/rkprogresscontrol.h"
#include "../core/robjectlist.h"
#include "../rkward.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../windows/rkworkplace.h"

#include "../debug.h"

// We save to several files at once, meaning the standard overwrite check is not quite good enough for us.
// More importantly, it is entirely broken in KF5 < 5.22.0 (https://bugs.kde.org/show_bug.cgi?id=360666)
// So check for overwriting ourselves.
bool checkOverwriteWorkspace (const QUrl &url, QWidget *parent) {
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

bool RKSaveAgent::saveWorkspaceAs(const QUrl& previous_url) {
	RK_TRACE(APP);

	QUrl save_url = QUrl::fromLocalFile(QFileDialog::getSaveFileName(RKWardMainWindow::getMain(), QString(), previous_url.toLocalFile(), i18n("R Workspace Files [%1](%1);;All files [*](*)", RKSettingsModuleGeneral::workspaceFilenameFilter()), nullptr, QFileDialog::DontConfirmOverwrite));
	if (save_url.isEmpty()) return false;
	if (!checkOverwriteWorkspace(save_url, RKWardMainWindow::getMain())) return false;

	return saveWorkspace(save_url);
}

bool RKSaveAgent::saveWorkspace(const QUrl& _url) {
	RK_TRACE(APP);

	QUrl url = _url;
	if (url.isEmpty()) url = RKWorkplace::mainWorkplace()->workspaceURL();
	if (url.isEmpty()) return saveWorkspaceAs();

	RKWorkplace::mainWorkplace()->flushAllData();
	auto save_chain = RInterface::startChain(nullptr);

	RKWorkplace::mainWorkplace()->saveWorkplace(url, save_chain);
	auto command = new RCommand("save.image(" + RObject::rQuote(url.toLocalFile()) + ')', RCommand::App);
	RKProgressControl control(RKWardMainWindow::getMain(), i18n("Workspace is being saved. <b>Hint:</b> Should this take an unusually long time, on a regular sized workspace, this may be due to other commands still pending completion in the R backend."), i18n("Saving workspace"), RKProgressControl::CancellableNoProgress);
	control.addRCommand(command, true);
	bool success = false;
	QObject::connect(command->notifier(), &RCommandNotifier::commandFinished, [&success, command]() { success = command->succeeded(); });  // clazy:exclude=lambda-in-connect (-> doModal(), below)
	RInterface::issueCommand(command, save_chain);
	control.doModal(false);
	RInterface::closeChain(save_chain);

	if (!success) KMessageBox::error(RKWardMainWindow::getMain(), i18n("Save failed"), i18n("An error occurred while trying to save the workspace. You data was <b>not</b> saved."));
	RKWorkplace::mainWorkplace()->setWorkspaceURL(url, true);

	return success;
}
