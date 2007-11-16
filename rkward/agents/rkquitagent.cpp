/***************************************************************************
                          rkquitagent  -  description
                             -------------------
    begin                : Thu Jan 18 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include "rkquitagent.h"

#include <klocale.h>

#include <qtimer.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../rkward.h"
#include "../misc/rkprogresscontrol.h"

#include "../debug.h"

//static
bool RKQuitAgent::quitting = false;

RKQuitAgent::RKQuitAgent (QObject *parent) : QObject (parent) {
	RK_TRACE (APP);

	quitting = true;
	RCommand *command = new RCommand (QString (), RCommand::EmptyCommand | RCommand::QuitCommand, QString (), this);

	RKWardMainWindow::getMain ()->hide ();
	cancel_dialog = new RKProgressControl (this, i18n ("Waiting for remaining R commands to finish. To quit immediately, press Cancel (WARNING: This may result in loss of data)"), i18n ("Waiting for R to finish"), RKProgressControl::AllowCancel | RKProgressControl::ShowAtOnce);
	cancel_dialog->addRCommand (command, true);
	connect (cancel_dialog, SIGNAL (cancelled ()), this, SLOT (doQuitNow ()));

	if (RKGlobals::rInterface ()->backendIsDead ()) {	// nothing to loose
		QTimer::singleShot (0, this, SLOT (doQuitNow ()));
		return;
	} else if (RKGlobals::rInterface ()->backendIsIdle ()) {
		// there should be no problem while quitting. If there is, show the dialog after 300 msec
		QTimer::singleShot (300, this, SLOT (showWaitDialog ()));
	} else {
		showWaitDialog ();
	}

	RKGlobals::rInterface ()->issueCommand (command);
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

void RKQuitAgent::rCommandDone (RCommand *) {
	RK_TRACE (APP);

	QTimer::singleShot (0, this, SLOT (doQuitNow ()));
}


#include "rkquitagent.moc"
