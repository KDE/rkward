/***************************************************************************
                          rinterface.cpp  -  description
                             -------------------
    begin                : Fri Nov 1 2002
    copyright            : (C) 2002, 2004, 2005, 2006, 2007, 2009, 2010 by Thomas Friedrichsmeier
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

#include "rembedinternal.h"
#include "rcommandstack.h"
#include "../rkward.h"
#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rkmodificationtracker.h"
#include "../dialogs/rkloadlibsdialog.h"
#include "../dialogs/rkselectlistdialog.h"
#include "../dialogs/rkreadlinedialog.h"
#include "../agents/showedittextfileagent.h"
#include "../agents/rkeditobjectagent.h"
#include "../windows/rcontrolwindow.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkcommandlog.h"
#include "../windows/rkhtmlwindow.h"
#include "../plugin/rkcomponentmap.h"
#include "../misc/rkcommonfunctions.h"

#include "../windows/rkwindowcatcher.h"

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

#include <stdlib.h>

// update output (for immediate output commands) at least this often (msecs):
#define FLUSH_INTERVAL 100

//static
QMutex RInterface::mutex (QMutex::Recursive);
#ifdef DEBUG_MUTEX
	int RInterface::mutex_counter;
#endif // DEBUG_MUTEX

RInterface::RInterface () {
	RK_TRACE (RBACKEND);
	
#ifndef DISABLE_RKWINDOWCATCHER
	window_catcher = new RKWindowCatcher ();
#endif // DISABLE_RKWINDOWCATCHER

// If R_HOME is not set, most certainly the user called the binary without the wrapper script
	if (!getenv ("R_HOME")) {
		RK_DO (qDebug ("No R_HOME environment variable set. RKWard will quit in a moment. Always start rkward in the default way unless you know what you're doing."), RBACKEND, DL_ERROR);
	}

	new RCommandStackModel (this);
	RCommandStack::regular_stack = new RCommandStack (0);
	running_command_canceled = 0;
	command_logfile_mode = NotRecordingCommands;

	r_thread = new RThread ();

	// create a fake init command
	issueCommand (new RCommand (i18n ("R Startup"), RCommand::App | RCommand::Sync | RCommand::ObjectListUpdate, i18n ("R Startup")));

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

	if (r_thread->isRunning ()) {
		RK_DO (qDebug ("Waiting for R thread to finish up..."), RBACKEND, DL_INFO);
		r_thread->interruptProcessing (true);
		r_thread->kill ();
		r_thread->wait (1000);
		if (r_thread->isRunning ()) {
			RK_DO (qDebug ("Backend thread is still running. It will be killed, now."), RBACKEND, DL_WARNING);
			r_thread->terminate ();
			RK_ASSERT (false);
		}
	}
	delete r_thread;
	delete flush_timer;
	delete window_catcher;
}

bool RInterface::backendIsDead () {
	RK_TRACE (RBACKEND);

	return (!r_thread->isRunning ());
}

bool RInterface::backendIsIdle () {
	RK_TRACE (RBACKEND);

	bool idle;
	MUTEX_LOCK;
	idle = (RCommandStack::regular_stack->isEmpty() && (!r_thread->current_command));
	MUTEX_UNLOCK;
	return (idle);
}

bool RInterface::inRThread () {
	return (QThread::currentThread () == RKGlobals::rInterface ()->r_thread);
}

void RInterface::tryToDoEmergencySave () {
	RK_TRACE (RBACKEND);
	if (!inRThread ()) {
		RKGlobals::rInterface ()->r_thread->terminate ();
		RKGlobals::rInterface ()->r_thread->wait (1000);
	}
	RKGlobals::rInterface ()->r_thread->tryToDoEmergencySave ();
}

void RInterface::startThread () {
	RK_TRACE (RBACKEND);

	r_thread->start ();
}

RCommand *RInterface::runningCommand () {
	 return r_thread->current_command;
}

void RInterface::customEvent (QEvent *e) {
	RK_TRACE (RBACKEND);

	RKRBackendEvent *ev;
	if (((int) e->type ()) >= ((int) RKRBackendEvent::Base)) {
		ev = static_cast<RKRBackendEvent*> (e);
	} else {
		RK_ASSERT (false);
		return;
	}

	if (ev->etype () == RKRBackendEvent::RCommandOutput) {
		RThread::ROutputContainer *container = (static_cast <RThread::ROutputContainer *> (ev->data ()));
		container->command->newOutput (container->output);
		delete container;

// TODO: not quite good, yet, leads to staggering output (but overall throughput is the same):
	// output events can easily stack up in the hundreds, not allowing GUI events to get through. Let's block further output events for a minute and then catch up with the event queue
		if (qApp->hasPendingEvents ()) {
			r_thread->pauseOutput (true);
			qApp->processEvents ();
			r_thread->pauseOutput (false);
		}
	} else if (ev->etype () == RKRBackendEvent::RCommandIn) {
		RCommand *command = static_cast <RCommand *> (ev->data ());
		RKCommandLog::getLog ()->addInput (command);

		if (command_logfile_mode != NotRecordingCommands) {
			if ((!(command->type () & RCommand::Sync)) || command_logfile_mode == RecordingCommandsWithSync) {
				command_logfile.write (command->command ().toUtf8 ());
				command_logfile.write ("\n");
			}
		}
	} else if (ev->etype () == RKRBackendEvent::RCommandOut) {
		RCommand *command = static_cast <RCommand *> (ev->data ());
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
		command->finished ();
		delete command;
	} else if ((ev->etype () == RKRBackendEvent::RIdle)) {
		RKWardMainWindow::getMain ()->setRStatus (RKWardMainWindow::Idle);	
	} else if ((ev->etype () == RKRBackendEvent::RBusy)) {
		RKWardMainWindow::getMain ()->setRStatus (RKWardMainWindow::Busy);
	} else if ((ev->etype () == RKRBackendEvent::REvalRequest)) {
		r_thread->pauseOutput (false); // we may be recursing downwards into event loops here. Hence we need to make sure, we don't create a deadlock
		processREvalRequest (static_cast<REvalRequest *> (ev->data ()));
	} else if ((ev->etype () == RKRBackendEvent::RCallbackRequest)) {
		r_thread->pauseOutput (false); // see above
		processRCallbackRequest (static_cast<RCallbackArgs *> (ev->data ()));
	} else if ((ev->etype () == RKRBackendEvent::RStarted)) {
		r_thread->unlock (RThread::Startup);
		RKWardMainWindow::discardStartupOptions ();
	} else if ((ev->etype () == RKRBackendEvent::RStartupError)) {
		int* err_p = static_cast<int*> (ev->data ());
		int err = *err_p;
		delete err_p;
		QString message = i18n ("<p>There was a problem starting the R backend. The following error(s) occurred:</p>\n");
		if (err & RThread::LibLoadFail) {
			message.append (i18n ("</p>\t- The 'rkward' R-library either could not be loaded at all, or not in the correct version. This may lead to all sorts of errors, from single missing features to complete failure to function. The most likely cause is that the last installation did not place all files in the correct place. However, in some cases, left-overs from a previous installation that was not cleanly removed may be the cause.</p>\
			<p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://p.sf.net/rkward/compiling\">http://p.sf.net/rkward/compiling</a>.</p>\n"));
		}
		if (err & RThread::SinkFail) {
			message.append (i18n ("<p>\t-There was a problem setting up the communication with R. Most likely this is due to an incorrect version of the 'rkward' R-library or failure to find that at all. This indicates a broken installation.</p>\
			<p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://p.sf.net/rkward/compiling\">http://p.sf.net/rkward/compiling</a>.</p></p>\n"));
		}
		if (err & RThread::OtherFail) {
			message.append (i18n ("<p>\t-An unspecified error occurred that is not yet handled by RKWard. Likely RKWard will not function properly. Please check your setup.</p>\n"));
		}
		QString details = runningCommand()->fullOutput();
		if (!details.isEmpty ()) {
			// WORKAROUND for stupid KMessageBox behavior. (kdelibs 4.2.3)
			// If length of details <= 512, it tries to show the details as a QLabel.
			details = details.replace('<', "&lt;").replace('\n', "<br>").leftJustified (513);
		}
		KMessageBox::detailedError (0, message, details, i18n ("Error starting R"), KMessageBox::Notify | KMessageBox::AllowLink);
	} else {
		RK_ASSERT (false);
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

void RInterface::closeChain (RCommandChain *chain) {
	RK_TRACE (RBACKEND);

	MUTEX_LOCK;
	RCommandStack::closeChain (chain);
	MUTEX_UNLOCK;
};

void RInterface::cancelCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	MUTEX_LOCK;
	
	if (!(command->type () & RCommand::Sync)) {
		command->status |= RCommand::Canceled;
		if (command->type () && RCommand::Running) {
			if (running_command_canceled != command) {
				RK_ASSERT (!running_command_canceled);
				r_thread->lock (RThread::Cancel);
				running_command_canceled = command;
				r_thread->interruptProcessing (true);
			}
		}
	} else {
		RK_ASSERT (false);
	}

	RCommandStackModel::getModel ()->itemChange (command);
	MUTEX_UNLOCK;
}

void RInterface::pauseProcessing (bool pause) {
	RK_TRACE (RBACKEND);

	if (pause) r_thread->lock (RThread::User);
	else r_thread->unlock (RThread::User);
}

void RInterface::processREvalRequest (REvalRequest *request) {
	RK_TRACE (RBACKEND);

	// clear reply object
	issueCommand (".rk.set.reply (NULL)", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
	if (request->call.isEmpty ()) {
		RK_ASSERT (false);
		closeChain (request->in_chain);
		return;
	}
	
	QString call = request->call[0];
	if (call == "get.tempfile.name") {
		if (request->call.count () >= 3) {
			QString file_prefix = request->call[1];
			QString file_extension = request->call[2];

			issueCommand (".rk.set.reply (\"" + RKCommonFunctions::getUseableRKWardSavefileName (file_prefix, file_extension) + "\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		} else {
			issueCommand (".rk.set.reply (\"Too few arguments in call to get.tempfile.name.\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		}
	} else if (call == "set.output.file") {
		RK_ASSERT (request->call.count () == 2);

		RKOutputWindowManager::self ()->setCurrentOutputPath (request->call[1]);
	} else if (call == "sync") {
		RK_ASSERT (request->call.count () >= 2);

		for (int i = 1; i < request->call.count (); ++i) {
			QString object_name = request->call[i];
			RObject *obj = RObjectList::getObjectList ()->findObject (object_name);
			if (obj) {
				RK_DO (qDebug ("triggering update for symbol %s", object_name.toLatin1 ().data()), RBACKEND, DL_DEBUG);
				obj->markDataDirty ();
				obj->updateFromR (request->in_chain);
			} else {
				RK_DO (qDebug ("lookup failed for changed symbol %s", object_name.toLatin1 ().data()), RBACKEND, DL_WARNING);
			}
		}
	} else if (call == "syncenvs") {
		RK_DO (qDebug ("triggering update of object list"), RBACKEND, DL_DEBUG);
		RObjectList::getObjectList ()->updateFromR (request->in_chain, request->call.mid (1));
	} else if (call == "syncglobal") {
		RK_DO (qDebug ("triggering update of globalenv"), RBACKEND, DL_DEBUG);
		RObjectList::getGlobalEnv ()->updateFromR (request->in_chain, request->call.mid (1));
	} else if (call == "edit") {
		RK_ASSERT (request->call.count () >= 2);

		QStringList object_list = request->call.mid (1);
		new RKEditObjectAgent (object_list, request->in_chain);
	} else if (call == "require") {
		if (request->call.count () >= 2) {
			QString lib_name = request->call[1];
			KMessageBox::information (0, i18n ("The R-backend has indicated that in order to carry out the current task it needs the package '%1', which is not currently installed. We will open the package-management tool, and there you can try to locate and install the needed package.", lib_name), i18n ("Require package '%1'", lib_name));
			RKLoadLibsDialog::showInstallPackagesModal (0, request->in_chain, lib_name);
			issueCommand (".rk.set.reply (\"\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		} else {
			issueCommand (".rk.set.reply (\"Too few arguments in call to require.\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		}
	} else if (call == "quit") {
		RKWardMainWindow::getMain ()->close ();
		// if we're still alive, quitting was cancelled
		issueCommand (".rk.set.reply (\"Quitting was cancelled\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
#ifndef DISABLE_RKWINDOWCATCHER
 	} else if (call == "startOpenX11") {
		// TODO: error checking/handling (wrong parameter count/type)
		if (request->call.count () >= 2) {
			MUTEX_LOCK;
			window_catcher->start (QString (request->call[1]).toInt ());
			MUTEX_UNLOCK;
		}
 	} else if (call == "endOpenX11") {
		// TODO: error checking/handling (wrong parameter count/type)
		if (request->call.count () >= 2) {
			MUTEX_LOCK;
			window_catcher->stop (QString (request->call[1]).toInt ());
			MUTEX_UNLOCK;
		}
	} else if (call == "updateDeviceHistory") {
		if (request->call.count () >= 2) {
			window_catcher->updateHistory (request->call.mid (1));
		}
	} else if (call == "killDevice") {
		if (request->call.count () >= 2) {
			window_catcher->killDevice (request->call[1].toInt ());
		}
#endif // DISABLE_RKWINDOWCATCHER
	} else if (call == "wdChange") {
		RKWardMainWindow::getMain ()->updateCWD ();
	} else if (call == "preLocaleChange") {
		int res = KMessageBox::warningContinueCancel (0, i18n ("A command in the R backend is trying to change the character encoding. While RKWard offers support for this, and will try to adjust to the new locale, this operation may cause subtle bugs, if data windows are currently open. Also the feature is not well tested, yet, and it may be advisable to save your workspace before proceeding.\nIf you have any data editor opened, or in any doubt, it is recommended to close those first (this will probably be auto-detected in later versions of RKWard). In this case, please chose 'Cancel' now, then close the data windows, save, and retry."), i18n ("Locale change"));
		if (res != KMessageBox::Continue) {
			issueCommand (".rk.set.reply (FALSE)", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		}
	} else if (call == "doPlugin") {
		if (request->call.count () >= 3) {
			QString message;
			bool ok;
			RKComponentMap::ComponentInvocationMode mode = RKComponentMap::ManualSubmit;
			if (request->call[2] == "auto") mode = RKComponentMap::AutoSubmit;
			else if (request->call[2] == "submit") mode = RKComponentMap::AutoSubmitOrFail;
			ok = RKComponentMap::invokeComponent (request->call[1], request->call.mid (3), mode, &message, request->in_chain);

			if (message.isEmpty ()) {
				issueCommand (".rk.set.reply (NULL)", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
			} else {
				QString type = "warning";
				if (!ok) type = "error";
				issueCommand (".rk.set.reply (list (type=\"" + type + "\", message=\"" + RKCommonFunctions::escape (message) + "\"))", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
			}
		} else {
			RK_ASSERT (false);
		}
	} else if (call == "listPlugins") {
		if (request->call.count () == 1) {
			QStringList list = RKComponentMap::getMap ()->allComponentIds ();
			issueCommand (".rk.set.reply (c (\"" + list.join ("\", \"") + "\"))\n", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
		} else {
			RK_ASSERT (false);
		}
	} else if (call == "showHTML") {
		if (request->call.count () == 2) {
			RKWorkplace::mainWorkplace ()->openHelpWindow (request->call[1]);
		} else {
			RK_ASSERT (false);
		}
	} else if (call == "select.list") {
		QString title = request->call[1];
		bool multiple = (request->call[2] == "multi");
		int num_preselects = request->call[3].toInt ();
		QStringList preselects = request->call.mid (4, num_preselects);
		QStringList choices = request->call.mid (4 + num_preselects);

		QStringList results = RKSelectListDialog::doSelect (0, title, choices, preselects, multiple);
		if (results.isEmpty ()) results.append ("");	// R wants to have it that way

		QString command = ".rk.set.reply (c (";
		for (int i = 0; i < results.count (); ++i) {
			if (i > 0) command.append (", ");
			command.append (RObject::rQuote (results[i]));
		}
		command.append ("))");

		issueCommand (command, RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
	} else if (call == "recordCommands") {
		if (request->call.count () == 3) {
			QString filename = request->call[1];
			bool with_sync = (request->call[2] == "include.sync");

			if (filename.isEmpty ()) {
				command_logfile_mode = NotRecordingCommands;
				command_logfile.close ();
			} else {
				if (command_logfile_mode != NotRecordingCommands) {
					issueCommand (".rk.set.reply (\"Attempt to start recording, while already recording commands. Ignoring.\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
				} else {
					command_logfile.setFileName (filename);
					bool ok = command_logfile.open (QIODevice::WriteOnly | QIODevice::Truncate);
					if (ok) {
						command_logfile_mode = RecordingCommands;
						if (with_sync) command_logfile_mode = RecordingCommandsWithSync;
					} else {
						issueCommand (".rk.set.reply (\"Could not open file for writing. Not recording commands.\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
					}
				}
			}
		} else {
			RK_ASSERT (false);
		}
	} else {
		issueCommand (".rk.set.reply (\"Unrecognized call '" + call + "'. Ignoring\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, request->in_chain);
	}
	
	closeChain (request->in_chain);
}

void RInterface::processRCallbackRequest (RCallbackArgs *args) {
	RK_TRACE (RBACKEND);

	// first, copy out the type. Allows for easier typing below
	RCallbackArgs::RCallbackType type = args->type;

	if (type == RCallbackArgs::RShowMessage) {
		QString caption = args->params["caption"].toString ();
		QString message = args->params["message"].toString ();
		QString button_yes = args->params["button_yes"].toString ();;
		QString button_no = args->params["button_no"].toString ();;
		QString button_cancel = args->params["button_cancel"].toString ();;

		KGuiItem button_yes_item = KStandardGuiItem::yes ();
		if (button_yes != "yes") button_yes_item.setText (button_yes);
		KGuiItem button_no_item = KStandardGuiItem::no ();
		if (button_no != "no") button_no_item.setText (button_no);
		KGuiItem button_cancel_item = KStandardGuiItem::cancel ();
		if (button_cancel != "cancel") button_cancel_item.setText (button_cancel);

		bool shown = false;
		KMessageBox::DialogType dialog_type = KMessageBox::QuestionYesNoCancel;
		if (button_cancel.isEmpty ()) dialog_type = KMessageBox::QuestionYesNo;
		if (button_no.isEmpty () && button_cancel.isEmpty ()) {
			dialog_type = KMessageBox::Information;
			if (args->params["wait"].toString () != "1") {	// non-modal dialogs are not supported out of the box by KMessageBox;
				KDialog* dialog = new KDialog ();
				KMessageBox::createKMessageBox (dialog, QMessageBox::Information, message, QStringList (), QString (), 0, KMessageBox::Notify | KMessageBox::NoExec);
				dialog->setWindowTitle (caption);
				dialog->setAttribute (Qt::WA_DeleteOnClose);
				dialog->setButtons (KDialog::Ok);
				dialog->show();
				shown = true;
			}
		}

		int result = KMessageBox::Ok;
		if (!shown) {
			result = KMessageBox::messageBox (0, dialog_type, message, caption, button_yes_item, button_no_item, button_cancel_item);
		}

		QString result_string;
		if ((result == KMessageBox::Yes) || (result == KMessageBox::Ok)) result_string = "yes";
		else if (result == KMessageBox::No) result_string = "no";
		else if (result == KMessageBox::Cancel) result_string = "cancel";
		else RK_ASSERT (false);

		args->params["result"] = result_string;
	} else if (type == RCallbackArgs::RReadLine) {
		QString result;

		// yes, readline *can* be called outside of a current command (e.g. from tcl/tk)
		bool dummy_command = false;
		RCommand *command = runningCommand ();
		if (!command) {
			command = new RCommand ("");
			dummy_command = true;
		}

		bool ok = RKReadLineDialog::readLine (0, i18n ("R backend requests information"), args->params["prompt"].toString (), command, &result);
		args->params["result"] = QVariant (result);

		if (dummy_command) delete command;
		if (!ok) args->params["cancelled"] = QVariant (true);
	} else if ((type == RCallbackArgs::RShowFiles) || (type == RCallbackArgs::REditFiles)) {
		ShowEditTextFileAgent::showEditFiles (args);
		return;		// we are not done, yet!
	} else if (type == RCallbackArgs::RChooseFile) {
		QString filename;
		if (args->params["new"].toBool ()) {
			filename = KFileDialog::getSaveFileName ();
		} else {
			filename = KFileDialog::getOpenFileName ();
		}
		args->params["result"] = QVariant (filename);
	} else if (type == RCallbackArgs::RBackendExit) {
		QString message = args->params["message"].toString ();
		message += i18n ("\nIt will be shut down immediately. This means, you can not use any more functions that rely on the R backend. I.e. you can do hardly anything at all, not even save the workspace (but if you're lucky, R already did that). What you can do, however, is save any open command-files, the output, or copy data out of open data editors. Quit RKWard after that.\nSince this should never happen, please write a mail to rkward-devel@lists.sourceforge.net, and tell us, what you were trying to do, when this happened. Sorry!");
		KMessageBox::error (0, message, i18n ("R engine has died"));
		r_thread->kill ();
	}

	args->done = true;
}

#include "rinterface.moc"
