/***************************************************************************
                          rinterface.cpp  -  description
                             -------------------
    begin                : Fri Nov 1 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#include "rinterface.h"

#include "../rkwatch.h"
#include "rthread.h"
#include "rembed.h"
#include "../rkward.h"
#include "../settings/rksettingsmoduler.h"

#include "../rkglobals.h"
#include "../debug.h"

#include <kmessagebox.h>
#include <kinputdialog.h>
#include <klocale.h>

#include <stdlib.h>

RInterface::RInterface(){
	RK_TRACE (RBACKEND);
// note: we can safely mess with RKSettingsModuleR::r_home_dir, since if the setting is bad, the app will exit without anything being saved. If the
// setting is good, everything is fine anyway.
	
	char *env_r_home = getenv ("R_HOME");
	if (!env_r_home) {
		if (RKSettingsModuleR::r_home_dir == "") {
			qDebug ("guess");
			RKSettingsModuleR::r_home_dir = "/usr/lib/R";
			RKSettingsModuleR::r_home_dir = KInputDialog::getText (i18n ("R_HOME not set"), i18n ("Could not find an R_HOME-environment variable and don't have a stored setting for that either.\nThe R backend requires that variable. Please enter your R_HOME directory below.\nIf you don't get it right, the application will quit immediately and you'll have to start RKWard again."), RKSettingsModuleR:: r_home_dir);
		}
	} else {
		if (env_r_home != RKSettingsModuleR::r_home_dir) {
			qDebug ("conflict");
			if (KMessageBox::warningYesNo (0, i18n ("RKWard has a stored setting for the R_HOME-variable. However, in the environment, that variable is currently set to a different value. It's probably safe to assume the environment-setting is correct/more up to date. Using a wrong setting however will cause the application to quit immediately. Do you want to use the stored setting instead of the environment-setting?"), i18n ("Conflicting settings for R_HOME")) != KMessageBox::Yes) {
				RKSettingsModuleR::r_home_dir = env_r_home;
			}
		}
	}

	r_thread = new RThread (this);
	r_thread->start ();
	
	watch = new RKwatch (this);
	//watch->show ();
	//watch->lower ();
}

RInterface::~RInterface(){
	RK_TRACE (RBACKEND);
	delete watch;
}

void RInterface::customEvent (QCustomEvent *e) {
	RK_TRACE (RBACKEND);
	if (e->type () == RCOMMAND_IN_EVENT) {
		watch->addInput (static_cast <RCommand *> (e->data ()));
	} else if (e->type () == RCOMMAND_OUT_EVENT) {
		RCommand *command = static_cast <RCommand *> (e->data ());
		watch->addOutput (command);
		command->finished ();
		delete command;
	} else if ((e->type () == RIDLE_EVENT)) {
		RKGlobals::rkApp ()->setRStatus (false);	
	} else if ((e->type () == RBUSY_EVENT)) {
		RKGlobals::rkApp ()->setRStatus (true);	
	} else if ((e->type () == RSTARTED_EVENT)) {
		r_thread->unlock ();
	} else if ((e->type () > RSTARTUP_ERROR_EVENT)) {
		int err = e->type () - RSTARTUP_ERROR_EVENT;
		QString message = i18n ("There was a problem starting the R backend. The following errors:\n");
		if (err & REmbed::LibLoadFail) {
			message.append (i18n ("\t- The 'rkward' R-library could not be loaded. This library is needed for communication between R and RKWard and many things will not work properly if this library is not present. Likely RKWard will even crash. The 'rkward' R-library should have been included in your distribution or RKWard, and should have been set up when you ran 'make install'. Please try 'make install' again and check for any errors. You should quit RKWard now.\n"));
		}
		if (err & REmbed::SinkFail) {
			message.append (i18n ("\t-There was a problem opening the files needed for communication with R. Most likely this is due to an incorrect setting for the location of these files. Check whether you have correctly configured the location of the log-files (Settings->Configure Settings->Logfiles) and restart RKWard.\n"));
		}
		if (err & REmbed::OtherFail) {
			message.append (i18n ("\t-An unspecified error occured that is not yet handled by RKWard. Likely RKWard will not function properly. Please check your setup.\n"));
		}
		KMessageBox::error (0, message, i18n ("Error starting R"));
		r_thread->unlock ();
	}
}
