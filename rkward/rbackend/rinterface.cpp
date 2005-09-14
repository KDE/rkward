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
#include "rcommandstack.h"
#include "rembed.h"
#include "../rkward.h"
#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettingsmodulelogfiles.h"
#include "../core/robjectlist.h"
#include "../core/rkmodificationtracker.h"
#include "../dialogs/rkloadlibsdialog.h"
#include "../agents/showedittextfileagent.h"

#include "rkwindowcatcher.h"
#ifndef DISABLE_RKWINDOWCATCHER
// putting this here instead of the class-header so I'm able to mess with it often without long recompiles. Fix when it works!
RKWindowCatcher *window_catcher;
#endif // DISABLE_RKWINDOWCATCHER

#include "../rkglobals.h"
#include "../debug.h"

#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kfiledialog.h>
#include <klocale.h>

#include <qdir.h>

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

extern "C" {
	/** this is the var in R-space that stores an interrupt */
	extern int R_interrupts_pending;
}

//static
QMutex RInterface::mutex;
int RInterface::mutex_counter;

RInterface::RInterface () {
	RK_TRACE (RBACKEND);
	
#ifndef DISABLE_RKWINDOWCATCHER
	window_catcher = new RKWindowCatcher (0);
	window_catcher->hide ();
#endif // DISABLE_RKWINDOWCATCHER
	
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
	
	RCommandStack::regular_stack = new RCommandStack ();
	running_command_canceled = 0;
	
	r_thread = new RThread (this);
	r_thread->start ();

	watch = new RKwatch (this);
}

void RInterface::issueCommand (const QString &command, int type, const QString &rk_equiv, RCommandReceiver *receiver, int flags, RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	issueCommand (new RCommand (command, type, rk_equiv, receiver, flags), chain);
}


RInterface::~RInterface(){
	RK_TRACE (RBACKEND);
	
	// kill the thread gracefully
	MUTEX_LOCK
	r_thread->kill ();
	MUTEX_UNLOCK
	r_thread->wait (5000);
	
	// timeout in wait? Try a little harder
	if (r_thread->running ()) {
		MUTEX_LOCK
		R_interrupts_pending = 1;
		MUTEX_UNLOCK
		r_thread->wait (10000);
		// if the thread did not exit, yet - bad luck.
	}
	
	delete watch;
}

void RInterface::customEvent (QCustomEvent *e) {
	RK_TRACE (RBACKEND);
	if (e->type () == RCOMMAND_IN_EVENT) {
		watch->addInput (static_cast <RCommand *> (e->data ()));
	} else if (e->type () == RCOMMAND_OUT_EVENT) {
		RCommand *command = static_cast <RCommand *> (e->data ());
		if (command->status & RCommand::Canceled) {
			command->status |= RCommand::HasError;
			command->_error.append ("--- interrupted ---");
			if (running_command_canceled) {
				RK_ASSERT (command == running_command_canceled);
				running_command_canceled = 0;
				R_interrupts_pending = 0;
				r_thread->unlock ();
			}
		}
		watch->addOutput (command);
		command->finished ();
		delete command;
	} else if ((e->type () == RIDLE_EVENT)) {
		RKGlobals::rkApp ()->setRStatus (false);	
	} else if ((e->type () == RBUSY_EVENT)) {
		RKGlobals::rkApp ()->setRStatus (true);
	} else if ((e->type () == R_EVAL_REQUEST_EVENT)) {
		processREvalRequest (static_cast<REvalRequest *> (e->data ()));
	} else if ((e->type () == R_CALLBACK_REQUEST_EVENT)) {
		processRCallbackRequest (static_cast<RCallbackArgs *> (e->data ()));
	} else if ((e->type () == RSTARTED_EVENT)) {
		r_thread->unlock ();
	} else if ((e->type () > RSTARTUP_ERROR_EVENT)) {
		int err = e->type () - RSTARTUP_ERROR_EVENT;
		QString message = i18n ("There was a problem starting the R backend. The following error(s) occurred:\n");
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

void RInterface::issueCommand (RCommand *command, RCommandChain *chain) { 
	RK_TRACE (RBACKEND);
	MUTEX_LOCK;
	RCommandStack::issueCommand (command, chain);
	MUTEX_UNLOCK;
}

RCommandChain *RInterface::startChain (RCommandChain *parent) {
	RK_TRACE (RBACKEND);
	RCommandChain *ret;
	MUTEX_LOCK;
	ret = RCommandStack::startChain (parent);
	MUTEX_UNLOCK;
	return ret;
};

RCommandChain *RInterface::closeChain (RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	RCommandChain *ret;
	MUTEX_LOCK;
	ret = RCommandStack::closeChain (chain);
	MUTEX_UNLOCK;
	return ret;
};

void RInterface::cancelCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	MUTEX_LOCK;
	
	if (!(command->type () & RCommand::Sync)) {
		command->status |= RCommand::Canceled;
		if (command == r_thread->current_command) {
			RK_ASSERT (!running_command_canceled);
			r_thread->lock ();
			running_command_canceled = command;
			// this is the var in R-space that stores an interrupt
			R_interrupts_pending = 1;
		}
	} else {
		RK_ASSERT (false);
	}
	
	MUTEX_UNLOCK;
}

void RInterface::processREvalRequest (REvalRequest *request) {
	RK_TRACE (RBACKEND);

	// clear reply object
	issueCommand (".rk.rkreply <- NULL", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
	if (!request->call_length) {
		closeChain (request->in_chain);
		return;
	}
	
	QString call = request->call[0];
	if (call == "get.tempfile.name") {
		if (request->call_length >= 3) {
			QString file_prefix = request->call[1];
			QString file_extension = request->call[2];
			QDir dir (RKSettingsModuleLogfiles::filesPath ());
		
			int i=0;
			while (dir.exists (file_prefix + QString::number (i) + file_extension)) {
				i++;
			}
			issueCommand (".rk.rkreply <- \"" + dir.filePath (file_prefix + QString::number (i) + file_extension) + "\"", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
		} else {
			issueCommand (".rk.rkreply <- \"Too few arguments in call to get.tempfile.name.\"", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
		}
	} else if (call == "get.output.html.file") {
		QDir dir (RKSettingsModuleLogfiles::filesPath ());
		// TODO: make more generic, get filename sanely
		issueCommand (".rk.rkreply <- \"" + dir.filePath ("rk_out.html") + "\"", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
	} else if (call == "sync") {
		RObject *obj = 0;
		if (request->call_length >= 2) {
			QString object_name = request->call[1];
			obj = RKGlobals::rObjectList ()->findObject (object_name);
		}
		if (obj) {
			RObject::ChangeSet *set = new RObject::ChangeSet;
			set->from_index = -1;
			set->to_index = -1;
			// for now a complete update is needed, in case new objects were added
			RKGlobals::rObjectList ()->updateFromR ();
			RKGlobals::tracker ()->objectDataChanged (obj, set);
			
			issueCommand (".rk.rkreply <- \"Sync scheduled for object '" + obj->getFullName () + "'\"", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
		} else {
			issueCommand (".rk.rkreply <- \"Object not recognized or not specified in call to sync. Ignoring\"", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
		}
	} else if (call == "require") {
		if (request->call_length >= 2) {
			QString lib_name = request->call[1];
			KMessageBox::information (0, i18n ("The R-backend has indicated that in order to carry out the current task it needs the package '%1', which is not currently installed. We'll open the package-management tool, and there you can try to locate and install the needed package.").arg (lib_name), i18n ("Require package '%1'").arg (lib_name));
			RKLoadLibsDialog::showInstallPackagesModal (0, request->in_chain);
			issueCommand (".rk.rkreply <- \"\"", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
		} else {
			issueCommand (".rk.rkreply <- \"Too few arguments in call to require.\"", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
		}
	} else if (call == "quit") {
		RKGlobals::rkApp ()->slotFileQuit ();
		// if we're still alive, quitting was cancelled
		issueCommand (".rk.rkreply <- \"Quitting was cancelled\"", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
#ifndef DISABLE_RKWINDOWCATCHER
 // does not work, yet :-( R crashes.
	} else if (call == "catchWindow") {
		// TODO: error checking/handling (wrong parameter count/type)
		if (request->call_length >= 3) {
			MUTEX_LOCK;
			window_catcher->catchWindow (request->call[1], QString (request->call[2]).toInt ());
			MUTEX_UNLOCK;
		}
#endif // DISABLE_RKWINDOWCATCHER
	} else {
		issueCommand (".rk.rkreply <- \"Unrecognized call '" + call + "'. Ignoring\"", RCommand::App | RCommand::Sync, "", 0, 0, request->in_chain);
	}
	
	closeChain (request->in_chain);
}

void RInterface::processRCallbackRequest (RCallbackArgs *args) {
	RK_TRACE (RBACKEND);

	// first, copy out the type while mutex is locked. Allows for easier typing below
	MUTEX_LOCK;
	RCallbackArgs::RCallbackType type = args->type;

	if (type == RCallbackArgs::RShowMessage) {
		KMessageBox::information (0, QString (*(args->chars_a)), i18n ("Message from the R backend"));
	} else if (type == RCallbackArgs::RReadConsole) {
		QString res = KInputDialog::getText (i18n ("R backend requests information"), QString (*(args->chars_a)));
		res = res.left (args->int_a - 2) + "\n";
		qstrcpy (*(args->chars_b), res.latin1 ());
	} else if ((type == RCallbackArgs::RShowFiles) || (type == RCallbackArgs::REditFiles)) {
		if ((type == RCallbackArgs::RShowFiles) && (QString (*(args->chars_d)) == "rkwardhtml")) {
			// not to worry, just some help file to display
			// TODO: maybe move this to ShowEditTextFileAgent instead
			for (int n=0; n < args->int_a; ++n) {
				RKGlobals::rkApp ()->openHTML (args->chars_a[n]);
			}
		} else {
			ShowEditTextFileAgent::showEditFiles (args);
			MUTEX_UNLOCK;
			return;
		}
	} else if (type ==RCallbackArgs::RChooseFile) {
		QString filename;
		if (args->int_a) {
			filename = KFileDialog::getSaveFileName ();
		} else {
			filename = KFileDialog::getOpenFileName ();
		}
		filename = filename.left (args->int_b - 2);
		args->int_c = filename.length ();
		qstrcpy (*(args->chars_a), filename.latin1 ());
	} else if (type ==RCallbackArgs::RSuicide) {
		QString message = i18n ("The R engine has encountered a fatal error:\n") + QString (*(args->chars_a));
		message += i18n ("It will be shut down immediately. This means, you can not use any more functions that rely on the R backend. I.e. you can do hardly anything at all, not even save the workspace. What you can do, however, is save any open command-files, the output, or copy data out of open data editors. Quit RKWard after that. Sorry!");
		KMessageBox::error (0, message, i18n ("R engine has died"));
		r_thread->terminate ();
	} else if (type ==RCallbackArgs::RCleanUp) {
		QString message = i18n ("The R engine has shut down with status: ") + QString::number (args->int_a);
		message += i18n ("\nIt will be shut down immediately. This means, you can not use any more functions that rely on the R backend. I.e. you can do hardly anything at all, not even save the workspace. Hopefully, however, R has already saved the workspace. What you can do, however, is save any open command-files, the output, or copy data out of open data editors. Quit RKWard after that.\nSince this should never happen, please write a mail to rkward-devel@lists.sourceforge.net, and tell us, what you were trying to do, when this happened. Sorry!");
		KMessageBox::error (0, message, i18n ("R engine has died"));
		r_thread->terminate ();
	}

	args->done = true;
	MUTEX_UNLOCK;
}

