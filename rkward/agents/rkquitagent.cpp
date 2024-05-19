/*
rkquitagent - This file is part of RKWard (https://rkward.kde.org). Created: Thu Jan 18 2007
SPDX-FileCopyrightText: 2007-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkquitagent.h"

#include <KLocalizedString>

#include <qtimer.h>

#include "../rbackend/rkrinterface.h"
#include "../rkward.h"
#include "../misc/rkprogresscontrol.h"

#include "../debug.h"

//static
bool RKQuitAgent::quitting = false;

RKQuitAgent::RKQuitAgent (QObject *parent) : QObject (parent) {
	RK_TRACE (APP);

	quitting = true;
	RCommand *command = new RCommand(QString("# Quit"), RCommand::App | RCommand::EmptyCommand | RCommand::QuitCommand);

	RKWardMainWindow::getMain ()->hide ();
	cancel_dialog = new RKProgressControl (this, i18n ("Waiting for remaining R commands to finish. To quit immediately, press Cancel (WARNING: This may result in loss of data)"), i18n ("Waiting for R to finish"), RKProgressControl::AllowCancel | RKProgressControl::ShowAtOnce);
	cancel_dialog->addRCommand (command, true);
	connect (cancel_dialog, &RKProgressControl::cancelled, this, &RKQuitAgent::doQuitNow);

	if (RInterface::instance()->backendIsDead()) {	// nothing to loose
		QTimer::singleShot(0, this, &RKQuitAgent::doQuitNow);
		return;
	} else if (RInterface::instance()->backendIsIdle()) {
		// there should be no problem while quitting. If there is, show the dialog after 300 msec
		QTimer::singleShot(300, this, &RKQuitAgent::showWaitDialog);
	} else {
		showWaitDialog ();
	}

	command->whenFinished(this, [this]() {
		RK_TRACE(APP);
		QTimer::singleShot(0, this, &RKQuitAgent::doQuitNow);
	});
	RInterface::issueCommand (command);
}

RKQuitAgent::~RKQuitAgent () {
	RK_TRACE (APP);
}

void RKQuitAgent::showWaitDialog () {
	RK_TRACE (APP);

	cancel_dialog->doNonModal (true);
}

void RKQuitAgent::doQuitNow () {
	RK_TRACE (APP);

	RKWardMainWindow::getMain ()->close ();		// this will kill the agent as well.
}


