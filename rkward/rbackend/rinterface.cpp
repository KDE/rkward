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
#include "../settings/rksettingsmoduleoutput.h"
#include "../settings/rksettingsmodulegraphics.h"
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

#define GET_LIB_PATHS 1
#define GET_HELP_BASE 2
#define SET_RUNTIME_OPTS 3
#define STARTUP_PHASE2_COMPLETE 4

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
	startup_phase2_error = false;
	command_logfile_mode = NotRecordingCommands;
	previously_idle = false;
	previous_command = 0;
	locked = 0;

	// create a fake init command
	RCommand *fake = new RCommand (i18n ("R Startup"), RCommand::App | RCommand::Sync | RCommand::ObjectListUpdate, i18n ("R Startup"), this, STARTUP_PHASE2_COMPLETE);
	issueCommand (fake);

	flush_timer = new QTimer (this);
	connect (flush_timer, SIGNAL (timeout ()), this, SLOT (flushOutput ()));
	flush_timer->start (FLUSH_INTERVAL);

	r_thread = new RThread ();
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
	idle = (RCommandStack::regular_stack->isEmpty() && (!runningCommand()));
	return (idle);
}

bool RInterface::backendIsLocked () {
	return (RKGlobals::rInterface ()->locked != 0);
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

void RInterface::popPreviousCommand () {
	RK_TRACE (RBACKEND);

	RK_ASSERT (previous_command == 0);
	RK_ASSERT (!all_current_commands.isEmpty ());
	previous_command = all_current_commands.takeLast ();
	RCommandStack::currentStack ()->pop ();
}

void RInterface::tryNextCommand () {
	RK_TRACE (RBACKEND);
	if (!currentCommandRequest ()) return;

	RCommandStack *stack = RCommandStack::currentStack ();
	bool main_stack = (stack == RCommandStack::regular_stack);

	if ((!(main_stack && locked)) && stack->isActive ()) {		// do not respect locks in substacks
		RCommand *command = stack->currentCommand ();

		if (command) {
			all_current_commands.append (command);

			if (command->status & RCommand::Canceled) {
				// avoid passing cancelled commands to R
				command->status |= RCommand::Failed;

				// notify ourselves...
				popPreviousCommand ();
				handleCommandOut (new RCommandProxy (command));
				return;
			}

			if (previously_idle) RKWardMainWindow::getMain ()->setRStatus (RKWardMainWindow::Busy);
			previously_idle = false;

			doNextCommand (command);
			return;
		}
	}

	if ((!stack->isActive ()) && stack->isEmpty () && !main_stack) {
		// a substack was depleted
		delete stack;
		doNextCommand (0);
	} else if (main_stack) {
		if (!previously_idle) RKWardMainWindow::getMain ()->setRStatus (RKWardMainWindow::Idle);
		previously_idle = true;
	}
}

void RInterface::handleCommandOut (RCommandProxy *proxy) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (proxy);
	RK_ASSERT (previous_command);
	RCommand* command = previous_command;
	proxy->mergeAndDelete (command);

	#ifdef RKWARD_DEBUG
		int dl = DL_WARNING;		// failed application commands are an issue worth reporting, failed user commands are not
		if (command->type () & RCommand::User) dl = DL_DEBUG;
		if (command->failed ()) {
			command->status |= RCommand::WasTried | RCommand::Failed;
			if (command->status & RCommand::ErrorIncomplete) {
				RK_DO (qDebug ("Command failed (incomplete)"), RBACKEND, dl);
			} else if (command->status & RCommand::ErrorSyntax) {
				RK_DO (qDebug ("Command failed (syntax)"), RBACKEND, dl);
			} else if (command->status & RCommand::Canceled) {
				RK_DO (qDebug ("Command failed (interrupted)"), RBACKEND, dl);
			} else {
				RK_DO (qDebug ("Command failed (other)"), RBACKEND, dl);
			}
			RK_DO (qDebug ("failed command was: '%s'", qPrintable (command->command ())), RBACKEND, dl);
			RK_DO (qDebug ("- error message was: '%s'", qPrintable (command->error ())), RBACKEND, dl);
		}
	#endif

	if ((command->status & RCommand::Canceled) || (command == running_command_canceled)) {
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
			locked -= locked & Cancel;
		}
	}
	command->finished ();
	RCommandStackModel::getModel ()->itemChange (command);
	delete command;
	previous_command = 0;
}

void RInterface::doNextCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	RBackendRequest* command_request = currentCommandRequest ();
	RK_ASSERT (command_request);

	flushOutput (true);
	RCommandProxy *proxy = 0;
	if (command) {
		proxy = new RCommandProxy (command);

		RK_DO (qDebug ("running command: %s", command->command ().toLatin1().data ()), RBACKEND, DL_DEBUG);
		command->status |= RCommand::Running;
		RCommandStackModel::getModel ()->itemChange (command);

		RKCommandLog::getLog ()->addInput (command);

		if (command_logfile_mode != NotRecordingCommands) {
			if ((!(command->type () & RCommand::Sync)) || command_logfile_mode == RecordingCommandsWithSync) {
				command_logfile.write (command->command ().toUtf8 ());
				command_logfile.write ("\n");
			}
		}
	}

	command_request->command = proxy;
	command_request->completed ();
	command_requests.pop_back ();
	QThread::yieldCurrentThread ();
}

void RInterface::rCommandDone (RCommand *command) {
	RK_TRACE (RBACKEND);

	if (command->failed ()) {
		startup_phase2_error = true;
		return;
	}

	if (command->getFlags () == GET_LIB_PATHS) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		for (unsigned int i = 0; i < command->getDataLength (); ++i) {
			RKSettingsModuleRPackages::defaultliblocs.append (command->getStringVector ()[i]);
		}
	} else if (command->getFlags () == GET_HELP_BASE) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		RK_ASSERT (command->getDataLength () == 1);
		RKSettingsModuleR::help_base_url = command->getStringVector ()[0];
	} else if (command->getFlags () == SET_RUNTIME_OPTS) {
		// no special handling. In case of failures, staturt_fail was set to true, above.
	} else if (command->getFlags () == STARTUP_PHASE2_COMPLETE) {
		QString message = startup_errors;
		if (startup_phase2_error) message.append (i18n ("<p>\t-An unspecified error occurred that is not yet handled by RKWard. Likely RKWard will not function properly. Please check your setup.</p>\n"));
		if (!message.isEmpty ()) {
			message.prepend (i18n ("<p>There was a problem starting the R backend. The following error(s) occurred:</p>\n"));

			QString details = runningCommand()->fullOutput().replace('<', "&lt;").replace('\n', "<br>");
			if (!details.isEmpty ()) {
				// WORKAROUND for stupid KMessageBox behavior. (kdelibs 4.2.3)
				// If length of details <= 512, it tries to show the details as a QLabel.
				details = details.leftJustified (513);
			}
			KMessageBox::detailedError (0, message, details, i18n ("Error starting R"), KMessageBox::Notify | KMessageBox::AllowLink);
		}

		startup_errors.clear ();
	}
}

void RInterface::customEvent (QEvent *e) {
	RK_TRACE (RBACKEND);

	RKRBackendEvent *ev;
	if (((int) e->type ()) == ((int) RKRBackendEvent::RKWardEvent)) {
		ev = static_cast<RKRBackendEvent*> (e);
	} else {
		RK_ASSERT (false);
		return;
	}

	flushOutput (true);
	RBackendRequest *request = ev->data ();
	if (request->type == RBackendRequest::CommandOut) {
		RCommandProxy *cproxy = request->command;

		// NOTE: the order of processing is: first try to submit the next command, then handle the old command.
		// The reason for doing it this way, instead of the reverse, is that this allows the backend thread / process to continue working, concurrently
		// NOTE: cproxy should only even be 0 in the very first cycle
		if (cproxy) popPreviousCommand ();
		command_requests.append (request);
		tryNextCommand ();
		if (cproxy) handleCommandOut (cproxy);
	} else if (request->type == RBackendRequest::HistoricalSubstackRequest) {
		processHistoricalSubstackRequest (request);
	} else if ((request->type == RBackendRequest::Started)) {
		// The backend thread has finished basic initialization, but we still have more to do...
		startup_errors = request->params["message"].toString ();

		command_requests.append (request);
		RCommandStack *stack = new RCommandStack (runningCommand ());
		RCommandChain *chain = stack->startChain (stack);

		// find out about standard library locations
		issueCommand (".libPaths ()\n", RCommand::GetStringVector | RCommand::App | RCommand::Sync, QString (), this, GET_LIB_PATHS, chain);
		// start help server / determined help base url
		issueCommand (".rk.getHelpBaseUrl ()\n", RCommand::GetStringVector | RCommand::App | RCommand::Sync, QString (), this, GET_HELP_BASE, chain);

		// apply user configurable run time options
		QStringList commands = RKSettingsModuleR::makeRRunTimeOptionCommands () + RKSettingsModuleRPackages::makeRRunTimeOptionCommands () + RKSettingsModuleOutput::makeRRunTimeOptionCommands () + RKSettingsModuleGraphics::makeRRunTimeOptionCommands ();
		for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
			issueCommand (*it, RCommand::App | RCommand::Sync, QString (), this, SET_RUNTIME_OPTS, chain);
		}
		// initialize output file
		issueCommand ("rk.set.output.html.file (\"" + RKSettingsModuleGeneral::filesPath () + "/rk_out.html\")\n", RCommand::App | RCommand::Sync, QString (), this, SET_RUNTIME_OPTS, chain);

		closeChain (chain);
		request->completed ();
	} else {
		processRBackendRequest (request);
	}
}

void RInterface::flushOutput () {
// do not trace. called periodically
//	RK_TRACE (RBACKEND);

	flushOutput (false);
}

void RInterface::flushOutput (bool forced) {
// do not trace. called periodically
//	RK_TRACE (RBACKEND);

	ROutputList list = r_thread->flushOutput (forced);

	foreach (ROutput *output, list) {
		if (all_current_commands.isEmpty ()) {
			RK_DO (qDebug ("output without receiver'%s'", qPrintable (output->output)), RBACKEND, DL_WARNING);
			delete output;
			continue;	// to delete the other output pointers, too
		} else {
			RK_DO (qDebug ("output '%s'", qPrintable (output->output)), RBACKEND, DL_DEBUG);
		}

		bool first = true;
		foreach (RCommand* command, all_current_commands) {
			ROutput *coutput = output;
			if (!first) {		// this output belongs to several commands at once. So we need to copy it.
				coutput = new ROutput;
				coutput->type = output->type;
				coutput->output = output->output;
			}
			first = false;

			if (coutput->type == ROutput::Output) {
				command->status |= RCommand::HasOutput;
				command->output_list.append (coutput);
			} else if (coutput->type == ROutput::Warning) {
				command->status |= RCommand::HasWarnings;
				command->output_list.append (coutput);
			} else if (coutput->type == ROutput::Error) {
				command->status |= RCommand::HasError;
				// An error output is typically just the copy of the previous output, so merge if possible
				if (command->output_list.isEmpty ()) {
					command->output_list.append (coutput);
				}
				if (command->output_list.last ()->output == coutput->output) {
					command->output_list.last ()->type = ROutput::Error;
					continue;	// don't call command->newOutput(), again!
				}
			}
			command->newOutput (coutput);
		}
	}
}

void RInterface::issueCommand (RCommand *command, RCommandChain *chain) { 
	RK_TRACE (RBACKEND);

	if (command->command ().isEmpty ()) command->_type |= RCommand::EmptyCommand;
	RCommandStack::issueCommand (command, chain);
	tryNextCommand ();
}

RCommandChain *RInterface::startChain (RCommandChain *parent) {
	RK_TRACE (RBACKEND);

	RCommandChain *ret;
	ret = RCommandStack::startChain (parent);
	return ret;
};

void RInterface::closeChain (RCommandChain *chain) {
	RK_TRACE (RBACKEND);

	RCommandStack::closeChain (chain);
	tryNextCommand ();
};

void RInterface::cancelCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	
	if (!(command->type () & RCommand::Sync)) {
		command->status |= RCommand::Canceled;
		if (command->type () && RCommand::Running) {
			if (running_command_canceled != command) {
				RK_ASSERT (!running_command_canceled);
				locked |= Cancel;
				running_command_canceled = command;
				r_thread->interruptProcessing (true);
			}
		}
	} else {
		RK_ASSERT (false);
	}

	RCommandStackModel::getModel ()->itemChange (command);
}

void RInterface::pauseProcessing (bool pause) {
	RK_TRACE (RBACKEND);

	if (pause) locked |= User;
	else locked -= locked & User;
}

void RInterface::processHistoricalSubstackRequest (RBackendRequest* request) {
	RK_TRACE (RBACKEND);

	command_requests.append (request);
	RK_ASSERT (!all_current_commands.isEmpty ());
	RCommandStack *reply_stack = new RCommandStack (all_current_commands.last ());
	RCommandChain *in_chain = reply_stack->startChain (reply_stack);

	QStringList calllist = request->params["call"].toStringList ();

	if (calllist.isEmpty ()) {
		RK_ASSERT (false);
		closeChain (in_chain);
		return;
	}

	QString call = calllist[0];
	if (call == "get.tempfile.name") {
		if (calllist.count () >= 3) {
			QString file_prefix = calllist[1];
			QString file_extension = calllist[2];

			issueCommand (".rk.set.reply (\"" + RKCommonFunctions::getUseableRKWardSavefileName (file_prefix, file_extension) + "\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
		} else {
			issueCommand (".rk.set.reply (\"Too few arguments in call to get.tempfile.name.\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
		}
	} else if (call == "set.output.file") {
		RK_ASSERT (calllist.count () == 2);

		RKOutputWindowManager::self ()->setCurrentOutputPath (calllist[1]);
	} else if (call == "sync") {
		RK_ASSERT (calllist.count () >= 2);

		for (int i = 1; i < calllist.count (); ++i) {
			QString object_name = calllist[i];
			RObject *obj = RObjectList::getObjectList ()->findObject (object_name);
			if (obj) {
				RK_DO (qDebug ("triggering update for symbol %s", object_name.toLatin1 ().data()), RBACKEND, DL_DEBUG);
				obj->markDataDirty ();
				obj->updateFromR (in_chain);
			} else {
				RK_DO (qDebug ("lookup failed for changed symbol %s", object_name.toLatin1 ().data()), RBACKEND, DL_WARNING);
			}
		}
	} else if (call == "syncenvs") {
		RK_DO (qDebug ("triggering update of object list"), RBACKEND, DL_DEBUG);
		RObjectList::getObjectList ()->updateFromR (in_chain, calllist.mid (1));
	} else if (call == "syncglobal") {
		RK_DO (qDebug ("triggering update of globalenv"), RBACKEND, DL_DEBUG);
		RObjectList::getGlobalEnv ()->updateFromR (in_chain, calllist.mid (1));
	} else if (call == "edit") {
		RK_ASSERT (calllist.count () >= 2);

		QStringList object_list = calllist.mid (1);
		new RKEditObjectAgent (object_list, in_chain);
	} else if (call == "require") {
		if (calllist.count () >= 2) {
			QString lib_name = calllist[1];
			KMessageBox::information (0, i18n ("The R-backend has indicated that in order to carry out the current task it needs the package '%1', which is not currently installed. We will open the package-management tool, and there you can try to locate and install the needed package.", lib_name), i18n ("Require package '%1'", lib_name));
			RKLoadLibsDialog::showInstallPackagesModal (0, in_chain, lib_name);
			issueCommand (".rk.set.reply (\"\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
		} else {
			issueCommand (".rk.set.reply (\"Too few arguments in call to require.\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
		}
	} else if (call == "quit") {
		RKWardMainWindow::getMain ()->close ();
		// if we're still alive, quitting was cancelled
		issueCommand (".rk.set.reply (\"Quitting was cancelled\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
#ifndef DISABLE_RKWINDOWCATCHER
 	} else if (call == "startOpenX11") {
		// TODO: error checking/handling (wrong parameter count/type)
		if (calllist.count () >= 2) {
			window_catcher->start (QString (calllist[1]).toInt ());
		}
 	} else if (call == "endOpenX11") {
		// TODO: error checking/handling (wrong parameter count/type)
		if (calllist.count () >= 2) {
			window_catcher->stop (QString (calllist[1]).toInt ());
		}
	} else if (call == "updateDeviceHistory") {
		if (calllist.count () >= 2) {
			window_catcher->updateHistory (calllist.mid (1));
		}
	} else if (call == "killDevice") {
		if (calllist.count () >= 2) {
			window_catcher->killDevice (calllist[1].toInt ());
		}
#endif // DISABLE_RKWINDOWCATCHER
	} else if (call == "wdChange") {
		RKWardMainWindow::getMain ()->updateCWD ();
	} else if (call == "preLocaleChange") {
		int res = KMessageBox::warningContinueCancel (0, i18n ("A command in the R backend is trying to change the character encoding. While RKWard offers support for this, and will try to adjust to the new locale, this operation may cause subtle bugs, if data windows are currently open. Also the feature is not well tested, yet, and it may be advisable to save your workspace before proceeding.\nIf you have any data editor opened, or in any doubt, it is recommended to close those first (this will probably be auto-detected in later versions of RKWard). In this case, please chose 'Cancel' now, then close the data windows, save, and retry."), i18n ("Locale change"));
		if (res != KMessageBox::Continue) {
			issueCommand (".rk.set.reply (FALSE)", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
		}
	} else if (call == "doPlugin") {
		if (calllist.count () >= 3) {
			QString message;
			bool ok;
			RKComponentMap::ComponentInvocationMode mode = RKComponentMap::ManualSubmit;
			if (calllist[2] == "auto") mode = RKComponentMap::AutoSubmit;
			else if (calllist[2] == "submit") mode = RKComponentMap::AutoSubmitOrFail;
			ok = RKComponentMap::invokeComponent (calllist[1], calllist.mid (3), mode, &message, in_chain);

			if (!message.isEmpty ()) {
				QString type = "warning";
				if (!ok) type = "error";
				issueCommand (".rk.set.reply (list (type=\"" + type + "\", message=\"" + RKCommonFunctions::escape (message) + "\"))", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
			}
		} else {
			RK_ASSERT (false);
		}
	} else if (call == "listPlugins") {
		if (calllist.count () == 1) {
			QStringList list = RKComponentMap::getMap ()->allComponentIds ();
			issueCommand (".rk.set.reply (c (\"" + list.join ("\", \"") + "\"))\n", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
		} else {
			RK_ASSERT (false);
		}
	} else if (call == "showHTML") {
		if (calllist.count () == 2) {
			RKWorkplace::mainWorkplace ()->openHelpWindow (calllist[1]);
		} else {
			RK_ASSERT (false);
		}
	} else if (call == "select.list") {
		QString title = calllist[1];
		bool multiple = (calllist[2] == "multi");
		int num_preselects = calllist[3].toInt ();
		QStringList preselects = calllist.mid (4, num_preselects);
		QStringList choices = calllist.mid (4 + num_preselects);

		QStringList results = RKSelectListDialog::doSelect (0, title, choices, preselects, multiple);
		if (results.isEmpty ()) results.append ("");	// R wants to have it that way

		QString command = ".rk.set.reply (c (";
		for (int i = 0; i < results.count (); ++i) {
			if (i > 0) command.append (", ");
			command.append (RObject::rQuote (results[i]));
		}
		command.append ("))");

		issueCommand (command, RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
	} else if (call == "recordCommands") {
		if (calllist.count () == 3) {
			QString filename = calllist[1];
			bool with_sync = (calllist[2] == "include.sync");

			if (filename.isEmpty ()) {
				command_logfile_mode = NotRecordingCommands;
				command_logfile.close ();
			} else {
				if (command_logfile_mode != NotRecordingCommands) {
					issueCommand (".rk.set.reply (\"Attempt to start recording, while already recording commands. Ignoring.\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
				} else {
					command_logfile.setFileName (filename);
					bool ok = command_logfile.open (QIODevice::WriteOnly | QIODevice::Truncate);
					if (ok) {
						command_logfile_mode = RecordingCommands;
						if (with_sync) command_logfile_mode = RecordingCommandsWithSync;
					} else {
						issueCommand (".rk.set.reply (\"Could not open file for writing. Not recording commands.\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
					}
				}
			}
		} else {
			RK_ASSERT (false);
		}
	} else {
		issueCommand (".rk.set.reply (\"Unrecognized call '" + call + "'. Ignoring\")", RCommand::App | RCommand::Sync, QString::null, 0, 0, in_chain);
	}
	
	closeChain (in_chain);
}

void RInterface::processRBackendRequest (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	// first, copy out the type. Allows for easier typing below
	RBackendRequest::RCallbackType type = request->type;

	if (type == RBackendRequest::ShowMessage) {
		QString caption = request->params["caption"].toString ();
		QString message = request->params["message"].toString ();
		QString button_yes = request->params["button_yes"].toString ();;
		QString button_no = request->params["button_no"].toString ();;
		QString button_cancel = request->params["button_cancel"].toString ();;

		KGuiItem button_yes_item = KStandardGuiItem::yes ();
		if (button_yes != "yes") button_yes_item.setText (button_yes);
		KGuiItem button_no_item = KStandardGuiItem::no ();
		if (button_no != "no") button_no_item.setText (button_no);
		KGuiItem button_cancel_item = KStandardGuiItem::cancel ();
		if (button_cancel != "cancel") button_cancel_item.setText (button_cancel);

		KMessageBox::DialogType dialog_type = KMessageBox::QuestionYesNoCancel;
		if (button_cancel.isEmpty ()) dialog_type = KMessageBox::QuestionYesNo;
		if (button_no.isEmpty () && button_cancel.isEmpty ()) {
			dialog_type = KMessageBox::Information;
			if (!request->synchronous) {	// non-modal dialogs are not supported out of the box by KMessageBox;
				KDialog* dialog = new KDialog ();
				KMessageBox::createKMessageBox (dialog, QMessageBox::Information, message, QStringList (), QString (), 0, KMessageBox::Notify | KMessageBox::NoExec);
				dialog->setWindowTitle (caption);
				dialog->setAttribute (Qt::WA_DeleteOnClose);
				dialog->setButtons (KDialog::Ok);
				dialog->show();

				request->completed ();
				return;
			}
		}

		int result = KMessageBox::messageBox (0, dialog_type, message, caption, button_yes_item, button_no_item, button_cancel_item);

		QString result_string;
		if ((result == KMessageBox::Yes) || (result == KMessageBox::Ok)) result_string = "yes";
		else if (result == KMessageBox::No) result_string = "no";
		else if (result == KMessageBox::Cancel) result_string = "cancel";
		else RK_ASSERT (false);

		request->params["result"] = result_string;
	} else if (type == RBackendRequest::ReadLine) {
		QString result;

		// yes, readline *can* be called outside of a current command (e.g. from tcl/tk)
		bool dummy_command = false;
		RCommand *command = runningCommand ();
		if (!command) {
			command = new RCommand ("", RCommand::EmptyCommand);
			dummy_command = true;
		}

		bool ok = RKReadLineDialog::readLine (0, i18n ("R backend requests information"), request->params["prompt"].toString (), command, &result);
		request->params["result"] = QVariant (result);

		if (dummy_command) delete command;
		if (!ok) request->params["cancelled"] = QVariant (true);
	} else if ((type == RBackendRequest::ShowFiles) || (type == RBackendRequest::EditFiles)) {
		ShowEditTextFileAgent::showEditFiles (request);
		return;		// we are not done, yet!
	} else if (type == RBackendRequest::ChooseFile) {
		QString filename;
		if (request->params["new"].toBool ()) {
			filename = KFileDialog::getSaveFileName ();
		} else {
			filename = KFileDialog::getOpenFileName ();
		}
		request->params["result"] = QVariant (filename);
	} else if (type == RBackendRequest::BackendExit) {
		QString message = request->params["message"].toString ();
		message += i18n ("\nIt will be shut down immediately. This means, you can not use any more functions that rely on the R backend. I.e. you can do hardly anything at all, not even save the workspace (but if you're lucky, R already did that). What you can do, however, is save any open command-files, the output, or copy data out of open data editors. Quit RKWard after that.\nSince this should never happen, please write a mail to rkward-devel@lists.sourceforge.net, and tell us, what you were trying to do, when this happened. Sorry!");
		KMessageBox::error (0, message, i18n ("R engine has died"));
		r_thread->kill ();
	}

	request->completed ();
}

#include "rinterface.moc"
