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
#include "../rkward.h"
#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rkmodificationtracker.h"
#include "../dialogs/rkloadlibsdialog.h"
#include "../dialogs/rkreadlinedialog.h"
#include "../agents/showedittextfileagent.h"
#include "../windows/rcontrolwindow.h"
#include "../windows/rkworkplace.h"

#include "rkwindowcatcher.h"
#ifndef DISABLE_RKWINDOWCATCHER
// putting this here instead of the class-header so I'm able to mess with it often without long recompiles. Fix when it works!
RKWindowCatcher *window_catcher;
#endif // DISABLE_RKWINDOWCATCHER

#include "../rkglobals.h"
#include "../debug.h"

#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>

#include <qdir.h>
#include <qtimer.h>
#include <qvalidator.h>

// update output (for immediate output commands) at least this often (msecs):
#define FLUSH_INTERVAL 100

//static
QMutex RInterface::mutex (true);
#ifdef DEBUG_MUTEX
	int RInterface::mutex_counter;
#endif // DEBUG_MUTEX

RInterface::RInterface () {
	RK_TRACE (RBACKEND);
	
#ifndef DISABLE_RKWINDOWCATCHER
	window_catcher = new RKWindowCatcher (0);
	window_catcher->hide ();
#endif // DISABLE_RKWINDOWCATCHER
	
// If R_HOME is not set, most certainly the user called the binary without the wrapper script
	if (!getenv ("R_HOME")) {
		RK_DO (qDebug ("No R_HOME environment variable set. RKWard will quit in a moment. Always start rkward in the default way unless you know what you're doing."), RBACKEND, DL_ERROR);
	}

	RCommandStack::regular_stack = new RCommandStack ();
	running_command_canceled = 0;
	
	r_thread = new RThread ();

	watch = new RKwatch ();

	flush_timer = new QTimer (this);
	connect (flush_timer, SIGNAL (timeout ()), this, SLOT (flushOutput ()));
	flush_timer->start (FLUSH_INTERVAL);
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
		r_thread->interruptProcessing (true);
		MUTEX_UNLOCK
		r_thread->wait (10000);
		// if the thread did not exit, yet - bad luck.
	}

	delete flush_timer;
	delete watch;
}

void RInterface::startThread () {
	RK_TRACE (RBACKEND);
	r_thread->start ();
}

RCommand *RInterface::runningCommand () {
	 return r_thread->current_command;
}

void RInterface::customEvent (QCustomEvent *e) {
	RK_TRACE (RBACKEND);
	if (e->type () == RCOMMAND_OUTPUT_EVENT) {
		RThread::ROutputContainer *container = (static_cast <RThread::ROutputContainer *> (e->data ()));
		container->command->newOutput (container->output);
		delete container;

// TODO: not quite good, yet, leads to staggering output (but overall throughput is the same):
	// output events can easily stack up in the hundreds, not allowing GUI events to get through. Let's block further output events for a minute and then catch up with the event queue
		if (qApp->hasPendingEvents ()) {
			r_thread->pauseOutput (true);
			qApp->processEvents ();
			r_thread->pauseOutput (false);
		}
	} else if (e->type () == RCOMMAND_IN_EVENT) {
		watch->addInput (static_cast <RCommand *> (e->data ()));
		RKGlobals::controlWindow ()->setCommandRunning (static_cast <RCommand *> (e->data ()));
	} else if (e->type () == RCOMMAND_OUT_EVENT) {
		RCommand *command = static_cast <RCommand *> (e->data ());
		if (command->status & RCommand::Canceled) {
			command->status |= RCommand::HasError;
			ROutput *out = new ROutput;
			out->type = ROutput::Error;
			out->output = ("--- interrupted ---");
			command->output_list.append (out);
			command->newOutput (out);
			if (running_command_canceled) {
				RK_ASSERT (command == running_command_canceled);
				running_command_canceled = 0;
				r_thread->interruptProcessing (false);
				r_thread->unlock (RThread::Cancel);
			}
		}
		RKGlobals::controlWindow ()->removeCommand (command);
		command->finished ();
		if (command->type () & RCommand::DirectToOutput) {
			RKWorkplace::mainWorkplace ()->newOutput ();
		}
		delete command;
	} else if ((e->type () == RIDLE_EVENT)) {
		RKwardApp::getApp ()->setRStatus (false);	
	} else if ((e->type () == RBUSY_EVENT)) {
		RKwardApp::getApp ()->setRStatus (true);
	} else if ((e->type () == RSEARCHLIST_CHANGED_EVENT)) {
		RK_DO (qDebug ("triggering update of object list"), RBACKEND, DL_DEBUG);
		RObjectList::getObjectList ()->updateFromR ();
	} else if ((e->type () == RGLOBALENV_SYMBOLS_CHANGED_EVENT)) {
		RK_DO (qDebug ("triggering update of globalenv"), RBACKEND, DL_DEBUG);
		// TODO: maybe this should be put inside a chain
		RObjectList::getGlobalEnv ()->updateFromR ();
	} else if ((e->type () == RINDIVIDUAL_SYMBOLS_CHANGED_EVENT)) {
		RK_DO (qDebug ("triggering update of some symbols"), RBACKEND, DL_DEBUG);
		QStringList *list = static_cast <QStringList *> (e->data ());
		for (QStringList::const_iterator it = list->constBegin (); it != list->constEnd (); ++it) {
			RObject *obj = RObjectList::getGlobalEnv ()->findObject (*it);
			if (obj) {
				// TODO: maybe this should be put inside a chain
				RK_DO (qDebug ("update triggered for %s", (*it).latin1 ()), RBACKEND, DL_DEBUG);
				obj->updateFromR ();
			} else {
				RK_DO (qDebug ("lookup failed for changed symbol %s", (*it).latin1 ()), RBACKEND, DL_WARNING);
			}
		}
		delete list;
	} else if ((e->type () == R_EVAL_REQUEST_EVENT)) {
		r_thread->pauseOutput (false); // we may be recursing downwards into event loops here. Hence we need to make sure, we don't create a deadlock
		processREvalRequest (static_cast<REvalRequest *> (e->data ()));
	} else if ((e->type () == R_CALLBACK_REQUEST_EVENT)) {
		r_thread->pauseOutput (false); // see above
		processRCallbackRequest (static_cast<RCallbackArgs *> (e->data ()));
	} else if ((e->type () == RSTARTED_EVENT)) {
		r_thread->unlock (RThread::Startup);
	} else if ((e->type () > RSTARTUP_ERROR_EVENT)) {
		int err = e->type () - RSTARTUP_ERROR_EVENT;
		QString message = i18n ("There was a problem starting the R backend. The following error(s) occurred:\n");
		if (err & RThread::LibLoadFail) {
			message.append (i18n ("\t- The 'rkward' R-library could not be loaded. This library is needed for communication between R and RKWard and many things will not work properly if this library is not present. Likely RKWard will even crash. The 'rkward' R-library should have been included in your distribution or RKWard, and should have been set up when you ran 'make install'. Please try 'make install' again and check for any errors. You should quit RKWard now.\n"));
		}
		if (err & RThread::SinkFail) {
			message.append (i18n ("\t-There was a problem opening the files needed for communication with R. Most likely this is due to an incorrect setting for the location of these files. Check whether you have correctly configured the location of the log-files (Settings->Configure Settings->Logfiles) and restart RKWard.\n"));
		}
		if (err & RThread::OtherFail) {
			message.append (i18n ("\t-An unspecified error occured that is not yet handled by RKWard. Likely RKWard will not function properly. Please check your setup.\n"));
		}
		KMessageBox::error (0, message, i18n ("Error starting R"));
		r_thread->unlock (RThread::Startup);
	}
}

void RInterface::flushOutput () {
// do not trace. called periodically
//	RK_TRACE (RBACKEND);
	MUTEX_LOCK;
	r_thread->flushOutput ();
	MUTEX_UNLOCK;
}

void RInterface::issueCommand (RCommand *command, RCommandChain *chain) { 
	RK_TRACE (RBACKEND);
	MUTEX_LOCK;
	if (command->command ().isEmpty ()) command->_type |= RCommand::EmptyCommand;
	RCommandStack::issueCommand (command, chain);
	RKGlobals::controlWindow ()->addCommand (command, chain);
	MUTEX_UNLOCK;
}

RCommandChain *RInterface::startChain (RCommandChain *parent) {
	RK_TRACE (RBACKEND);
	RCommandChain *ret;
	MUTEX_LOCK;
	ret = RCommandStack::startChain (parent);
	RKGlobals::controlWindow ()->addChain (ret);
	MUTEX_UNLOCK;
	return ret;
};

RCommandChain *RInterface::closeChain (RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	RCommandChain *ret;
	MUTEX_LOCK;
	ret = RCommandStack::closeChain (chain);
	RKGlobals::controlWindow ()->updateChain (chain);
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
			r_thread->lock (RThread::Cancel);
			running_command_canceled = command;
			r_thread->interruptProcessing (true);
		}
	} else {
		RK_ASSERT (false);
	}
	
	RKGlobals::controlWindow ()->updateCommand (command);
	MUTEX_UNLOCK;
}

void RInterface::pauseProcessing (bool pause) {
	RK_TRACE (RBACKEND);

	if (pause) r_thread->lock (RThread::User);
	else r_thread->unlock (RThread::User);
}

void RInterface::processREvalRequest (REvalRequest *request) {
	RK_TRACE (RBACKEND);

	RKGlobals::controlWindow()->addChain (request->in_chain);

	// clear reply object
	issueCommand (".rk.rkreply <- NULL", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
	if (!request->call_length) {
		closeChain (request->in_chain);
		return;
	}
	
	QString call = request->call[0];
	if (call == "get.tempfile.name") {
		if (request->call_length >= 3) {
			QString file_prefix = request->call[1];
			QString file_extension = request->call[2];
			QDir dir (RKSettingsModuleGeneral::filesPath ());
		
			int i=0;
			while (dir.exists (file_prefix + QString::number (i) + file_extension)) {
				i++;
			}
			issueCommand (".rk.rkreply <- \"" + dir.filePath (file_prefix + QString::number (i) + file_extension) + "\"", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		} else {
			issueCommand (".rk.rkreply <- \"Too few arguments in call to get.tempfile.name.\"", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		}
	} else if (call == "get.output.html.file") {
		QDir dir (RKSettingsModuleGeneral::filesPath ());
		// TODO: make more generic, get filename sanely
		issueCommand (".rk.rkreply <- \"" + dir.filePath ("rk_out.html") + "\"", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
	} else if (call == "sync") {
		RObject *obj = 0;
		if (request->call_length >= 2) {
			QString object_name = request->call[1];
			obj = RObjectList::getObjectList ()->findObject (object_name);
		}
		if (obj) {
			RObject::ChangeSet *set = new RObject::ChangeSet;
			set->from_index = -1;
			set->to_index = -1;
			// for now a complete update is needed, in case new objects were added
			RObjectList::getObjectList ()->updateFromR ();
			RKGlobals::tracker ()->objectDataChanged (obj, set);
			
			issueCommand (".rk.rkreply <- \"Sync scheduled for object '" + obj->getFullName () + "'\"", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		} else {
			issueCommand (".rk.rkreply <- \"Object not recognized or not specified in call to sync. Ignoring\"", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		}
	} else if (call == "require") {
		if (request->call_length >= 2) {
			QString lib_name = request->call[1];
			issueCommand (".rk.rkreply <- NULL", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
			KMessageBox::information (0, i18n ("The R-backend has indicated that in order to carry out the current task it needs the package '%1', which is not currently installed. We'll open the package-management tool, and there you can try to locate and install the needed package.").arg (lib_name), i18n ("Require package '%1'").arg (lib_name));
			RKLoadLibsDialog::showInstallPackagesModal (0, request->in_chain, lib_name);
			issueCommand (".rk.rkreply <- \"\"", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		} else {
			issueCommand (".rk.rkreply <- \"Too few arguments in call to require.\"", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		}
	} else if (call == "quit") {
		RKwardApp::getApp ()->close ();
		// if we're still alive, quitting was cancelled
		issueCommand (".rk.rkreply <- \"Quitting was cancelled\"", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
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
		issueCommand (".rk.rkreply <- \"Unrecognized call '" + call + "'. Ignoring\"", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
	}
	
	closeChain (request->in_chain);
}

void RInterface::processRCallbackRequest (RCallbackArgs *args) {
	RK_TRACE (RBACKEND);

	// first, copy out the type. Allows for easier typing below
	RCallbackArgs::RCallbackType type = args->type;

	if (type == RCallbackArgs::RShowMessage) {
		KMessageBox::information (0, QString (*(args->chars_a)), i18n ("Message from the R backend"));
	} else if (type == RCallbackArgs::RReadConsole) {
		QString result;

		bool ok = RKReadLineDialog::readLine (0, i18n ("R backend requests information"), *(args->chars_a), runningCommand (), &result);

		result = result.left (args->int_a - 2) + "\n";
		qstrcpy (*(args->chars_b), result.latin1 ());

		if (!ok) {
			args->done = true;		// need to do this at once. Else we risk getting stuck in the standard callback event loop
			cancelCommand (runningCommand ());
			return;
		}
	} else if ((type == RCallbackArgs::RShowFiles) || (type == RCallbackArgs::REditFiles)) {
		if ((type == RCallbackArgs::RShowFiles) && (QString (*(args->chars_d)) == "rkwardhtml")) {
			// not to worry, just some help file to display
			// TODO: maybe move this to ShowEditTextFileAgent instead
			for (int n=0; n < args->int_a; ++n) {
				RKwardApp::getApp ()->openHTML (args->chars_a[n]);
			}
		} else {
			ShowEditTextFileAgent::showEditFiles (args);
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
}

#include "rinterface.moc"
