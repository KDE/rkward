/*
rkrinterface.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Fri Nov 1 2002
SPDX-FileCopyrightText: 2002-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrinterface.h"

#include "rcommandstack.h"
#include "rkrbackendprotocol_frontend.h"
#include "../rkward.h"
#include "../rkconsole.h"
#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettingsmoduleoutput.h"
#include "../settings/rksettingsmodulegraphics.h"
#include "../settings/rksettingsmoduleplugins.h"
#include "../settings/rksettingsmoduleobjectbrowser.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rkmodificationtracker.h"
#include "../dialogs/rkloadlibsdialog.h"
#include "../dialogs/rkselectlistdialog.h"
#include "../dialogs/rkreadlinedialog.h"
#include "../dialogs/rkerrordialog.h"
#include "../agents/showedittextfileagent.h"
#include "../agents/rkeditobjectagent.h"
#include "../agents/rkprintagent.h"
#include "../agents/rkdebughandler.h"
#include "../windows/rcontrolwindow.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkcommandlog.h"
#include "../windows/rkhtmlwindow.h"
#include "../plugin/rkcomponentmap.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkmessagecatalog.h"
#include "../misc/rkoutputdirectory.h"
#include "rksessionvars.h"
#include "../windows/rkwindowcatcher.h"

#include "../version.h"
#include "../debug.h"

#include <kmessagebox.h>
#include <KLocalizedString>

#include <qdir.h>
#include <qvalidator.h>

#include <stdlib.h>
#include <QFileDialog>
#include <QApplication>
#include <QPushButton>
#include <QElapsedTimer>

// flush new pieces of output after this period of time:
#define FLUSH_INTERVAL 100

#define GET_LIB_PATHS 1
#define GET_HELP_BASE 2
#define SET_RUNTIME_OPTS 3
#define STARTUP_PHASE2_COMPLETE 4
#define GET_R_VERSION 5
#define RSTARTUP_COMPLETE 6

// statics
double RInterface::na_real;
int RInterface::na_int;
RInterface *RInterface::_instance = nullptr;

void RInterface::create() {
	RK_TRACE (RBACKEND);
	RK_ASSERT(_instance == nullptr);
	_instance = new RInterface();
}

RInterface::RInterface () {
	RK_TRACE (RBACKEND);

	new RCommandStackModel (this);
	RCommandStack::regular_stack = new RCommandStack ();
	startup_phase2_error = false;
	command_logfile_mode = NotRecordingCommands;
	previously_idle = false;
	locked = 0;
	backend_dead = false;
	flush_timer_id = 0;
	dummy_command_on_stack = 0;

	// create a fake init command
	RCommand *fake = new RCommand (i18n ("R Startup"), RCommand::App | RCommand::Sync | RCommand::ObjectListUpdate, i18n ("R Startup"), this, STARTUP_PHASE2_COMPLETE);
	_issueCommand(fake);

	new RKSessionVars (this);
	new RKDebugHandler (this);
	new RKRBackendProtocolFrontend (this);
	RKRBackendProtocolFrontend::instance ()->setupBackend ();

	/////// Further initialization commands, which do not necessarily have to run before everything else can be queued, here. ///////
	// NOTE: will receive the list as a call plain generic request from the backend ("updateInstalledPackagesList")
	_issueCommand(new RCommand(".rk.get.installed.packages()", RCommand::App | RCommand::Sync));

	_issueCommand(new RCommand(QString(), RCommand::App | RCommand::Sync | RCommand::EmptyCommand, QString(), this, RSTARTUP_COMPLETE));
}

void RInterface::issueCommand (const QString &command, int type, const QString &rk_equiv, RCommandReceiver *receiver, int flags, RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	issueCommand (new RCommand (command, type, rk_equiv, receiver, flags), chain);
}

RInterface::~RInterface(){
	RK_TRACE (RBACKEND);

	// Don't wait for QObject d'tor to destroy the backend transmitter. It might still try to call functions in the RInterface
	// (noteably, it does call qApp->processEvents().
	delete RKRBackendProtocolFrontend::instance ();
	RKWindowCatcher::discardInstance ();
}

bool RInterface::backendIsIdle () {
	RK_TRACE (RBACKEND);

	return (RCommandStack::regular_stack->isEmpty() && (!runningCommand()));
}

RCommand *RInterface::popPreviousCommand (int id) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (!all_current_commands.isEmpty ());
	for (int i = all_current_commands.size () - 1; i >= 0; --i) {
		RCommand *ret = all_current_commands[i];
		if (ret->id () == id) {
			RCommandStack::pop (ret);
			all_current_commands.removeAt (i);
			return ret;
		}
	}
	RK_ASSERT (false);
	return 0;
}

RCommandChain* RInterface::openSubcommandChain (RCommand* parent_command) {
	RK_TRACE (RBACKEND);

	current_commands_with_subcommands.append (parent_command);
	return RCommandStack::startChain (parent_command);
}

void RInterface::closeSubcommandChain (RCommand* parent_command) {
	RK_TRACE (RBACKEND);

	if (current_commands_with_subcommands.contains (parent_command)) {
		current_commands_with_subcommands.removeAll (parent_command);
		doNextCommand (0);
	}
	if (parent_command && (parent_command == dummy_command_on_stack)) {
		all_current_commands.removeAll (dummy_command_on_stack);
		RCommandStack::pop (dummy_command_on_stack);
		handleCommandOut (dummy_command_on_stack);
		dummy_command_on_stack = 0;
	}
}

void RInterface::tryNextCommand () {
	RK_TRACE (RBACKEND);
	RCommand *command = RCommandStack::currentCommand ();
	if (command_requests.isEmpty ()) {
		// if the backend is not requesting anything, only priority commands will be pushed
		if (!command) return;
		if (!(command->type () & RCommand::PriorityCommand)) return;
		if (all_current_commands.contains (command)) return;
	}

	bool priority = command && (command->type () & RCommand::PriorityCommand);
	bool on_top_level = all_current_commands.isEmpty ();
	if (!(on_top_level && locked && !(priority))) {                                     // do not respect locks for sub-commands
		if ((!on_top_level) && all_current_commands.contains (command)) {  // all sub-commands of the current command have finished, it became the top-most item of the RCommandStack, again
			closeSubcommandChain (command);
			return;
		}

		if (command) {
			all_current_commands.append (command);

			if (command->status & RCommand::Canceled) {
				// avoid passing cancelled commands to R
				command->status |= RCommand::Failed;

				// notify ourselves...
				RCommand* dummy = popPreviousCommand (command->id ());
				RK_ASSERT (dummy == command);
				handleCommandOut (command);
				return;
			}

			if (previously_idle) emit backendStatusChanged(Busy);
			previously_idle = false;

			doNextCommand (command);
			return;
		}
	}

	if (on_top_level) {
		if (!previously_idle) emit backendStatusChanged(Idle);
		previously_idle = true;
	}
}

void RInterface::handleCommandOut (RCommand *command) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (command);

	#ifdef RKWARD_DEBUG
		int dl = DL_WARNING;		// failed application commands are an issue worth reporting, failed user commands are not
		if (command->type () & RCommand::User) dl = DL_DEBUG;
		if (command->failed ()) {
			command->status |= RCommand::WasTried | RCommand::Failed;
			if (command->status & RCommand::ErrorIncomplete) {
				RK_DEBUG (RBACKEND, dl, "Command failed (incomplete)");
			} else if (command->status & RCommand::ErrorSyntax) {
				RK_DEBUG (RBACKEND, dl, "Command failed (syntax)");
			} else if (command->status & RCommand::Canceled) {
				RK_DEBUG (RBACKEND, dl, "Command failed (interrupted)");
			} else {
				RK_DEBUG (RBACKEND, dl, "Command failed (other)");
			}
			RK_DEBUG (RBACKEND, dl, "failed command was: '%s'", qPrintable (command->command ()));
			RK_DEBUG (RBACKEND, dl, "- error message was: '%s'", qPrintable (command->error ()));
		}
	#endif

	if (command->status & RCommand::Canceled) {
		command->status |= RCommand::HasError;
		ROutput *out = new ROutput;
		out->type = ROutput::Error;
		out->output = ("--- interrupted ---");
		command->output_list.append (out);
		command->newOutput (out);
	}
	command->finished ();
	delete command;
}

void RInterface::doNextCommand (RCommand *command) {
	RK_TRACE (RBACKEND);
	RBackendRequest* command_request = currentCommandRequest ();
	if (!command_request) {
		if (!(command && (command->type () & RCommand::PriorityCommand))) return;
	}
	// importantly, this point is not reached for the fake startup command

	if (RK_Debug::RK_Debug_CommandStep) {
		QElapsedTimer t;
		t.start();
		while (t.elapsed() < RK_Debug::RK_Debug_CommandStep) {}
	}

	flushOutput (true);
	RCommandProxy *proxy = 0;
	if (command) {
		proxy = command->makeProxy ();

		RK_DEBUG (RBACKEND, DL_DEBUG, "running command: %s", command->command ().toLatin1().data ());
		command->status |= RCommand::Running;
		RCommandStackModel::getModel ()->itemChange (command);

		RKCommandLog::getLog ()->addInput (command);

		if (command_logfile_mode != NotRecordingCommands) {
			bool record = true;
			if (command_logfile_mode != RecordingCommandsUnfiltered) {
				if (command->type () & (RCommand::Silent | RCommand::Sync)) record = false;
			}
			if (record) {
				command_logfile.write (command->command ().toUtf8 ());
				command_logfile.write ("\n");
			}
		}
	}

	if (command && (command->type () & RCommand::PriorityCommand)) {
		RKRBackendProtocolFrontend::sendPriorityCommand (proxy);
	} else {
		RK_ASSERT (command_request);
		command_request->command = proxy;
		RKRBackendProtocolFrontend::setRequestCompleted (command_request);
		command_requests.pop_back ();
	}
}

void RInterface::rCommandDone (RCommand *command) {
	RK_TRACE (RBACKEND);

	if (command->failed ()) {
		startup_phase2_error = true;
		return;
	}

	if (command->getFlags () == GET_LIB_PATHS) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		RKSettingsModuleRPackages::r_libs_user = command->stringVector ().value (0);
		RKSettingsModuleRPackages::defaultliblocs += command->stringVector ().mid (1);

		RCommandChain *chain = command->parent;
		RK_ASSERT (chain);
		RK_ASSERT (!chain->isClosed ());

		// apply user configurable run time options
		QStringList commands = RKSettingsModuleR::makeRRunTimeOptionCommands () + RKSettingsModuleRPackages::makeRRunTimeOptionCommands () + RKSettingsModuleOutput::makeRRunTimeOptionCommands () + RKSettingsModuleGraphics::makeRRunTimeOptionCommands ();
		for (QStringList::const_iterator it = commands.cbegin (); it != commands.cend (); ++it) {
			issueCommand (*it, RCommand::App | RCommand::Sync, QString (), this, SET_RUNTIME_OPTS, chain);
		}
		// initialize output file
		RKOutputDirectory::getCurrentOutput(chain);

#ifdef Q_OS_MACOS
		// On MacOS, the backend is started from inside R home to allow resolution of dynamic libs. Re-set to frontend wd, here.
		issueCommand ("setwd (" + RKRSharedFunctionality::quote (QDir::currentPath ()) + ")\n", RCommand::App | RCommand::Sync, QString (), this, SET_RUNTIME_OPTS, chain);
#endif
		// Workaround for https://bugs.kde.org/show_bug.cgi?id=421958
		if (RKSessionVars::compareRVersion("4.0.0") < 1 && RKSessionVars::compareRVersion("4.0.1") > 0) {
			issueCommand ("if(compiler::enableJIT(-1) > 2) compiler::enableJIT(2)\n", RCommand::App | RCommand::Sync, QString (), this, SET_RUNTIME_OPTS, chain);
		}

		closeChain (chain);
	} else if (command->getFlags () == GET_R_VERSION) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		RK_ASSERT (command->getDataLength () == 1);
		RKSessionVars::setRVersion (command->stringVector ().value (0));
	} else if (command->getFlags () == GET_HELP_BASE) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		RK_ASSERT (command->getDataLength () == 1);
		RKSettingsModuleR::help_base_url = command->stringVector ().value (0);
	} else if (command->getFlags () == SET_RUNTIME_OPTS) {
		// no special handling. In case of failures, staturt_fail was set to true, above.
	} else if (command->getFlags () == STARTUP_PHASE2_COMPLETE) {
		QString message = startup_errors;
		if (startup_phase2_error) message.append (i18n ("<p>\t-An unspecified error occurred that is not yet handled by RKWard. Likely RKWard will not function properly. Please check your setup.</p>\n"));
		if (!message.isEmpty ()) {
			message.prepend (i18n ("<p>There was a problem starting the R backend. The following error(s) occurred:</p>\n"));

			QString details = command->fullOutput().replace('<', "&lt;").replace('\n', "<br>");
			if (!details.isEmpty ()) {
				// WORKAROUND for stupid KMessageBox behavior. (kdelibs 4.2.3)
				// If length of details <= 512, it tries to show the details as a QLabel.
				details = details.leftJustified (513);
			}
			KMessageBox::detailedError (0, message, details, i18n ("Error starting R"), KMessageBox::Notify | KMessageBox::AllowLink);
		}

		startup_errors.clear ();
	} else if (command->getFlags () == RSTARTUP_COMPLETE) {
		RKSettings::validateSettingsInteractive ();
	}
}

void RInterface::handleRequest (RBackendRequest* request) {
	RK_TRACE (RBACKEND);

	if (request->type == RBackendRequest::OutputStartedNotification) {
		RK_ASSERT (flush_timer_id == 0);
		flush_timer_id = startTimer (FLUSH_INTERVAL);	// calls flushOutput (false); see timerEvent ()
		RKRBackendProtocolFrontend::setRequestCompleted (request);
		return;
	}

	flushOutput (true);
	if (request->type == RBackendRequest::CommandOut) {
		RCommandProxy *cproxy = request->takeCommand ();
		if (cproxy) {
			RK_DEBUG (RBACKEND, DL_DEBUG, "Command out \"%s\", id %d", qPrintable (cproxy->command), cproxy->id);
		} else {
			RK_DEBUG (RBACKEND, DL_DEBUG, "Fake command out");
		}
		RCommand *command = 0;
		// NOTE: the order of processing is: first try to submit the next command, then handle the old command.
		// The reason for doing it this way, instead of the reverse, is that this allows the backend thread / process to continue working, concurrently
		// NOTE: cproxy should only ever be 0 in the very first cycle
		if (cproxy) command = popPreviousCommand (cproxy->id);
		if (request->synchronous) command_requests.append (request);
		tryNextCommand ();
		if (cproxy) {
			RK_ASSERT (command);
			command->mergeAndDeleteProxy (cproxy);
			handleCommandOut (command);
		}
		tryNextCommand ();
	} else if (request->type == RBackendRequest::GenericRequestWithSubcommands) {
		RCommandProxy *cproxy = request->takeCommand();
		RCommand *parent = 0;
		for (int i = all_current_commands.size () - 1; i >= 0; --i) {
			if (all_current_commands[i]->id () == cproxy->id) {
				parent = all_current_commands[i];
				break;
			}
		}
		delete cproxy;
		RK_ASSERT(request->subcommandrequest);
		command_requests.append(request->subcommandrequest);
		request->subcommandrequest = nullptr;  // it is now a separate request. Make sure we won't try to send it back as part of this one.
		processHistoricalSubstackRequest(request->params["call"].toStringList(), parent, request);
		RKRBackendProtocolFrontend::setRequestCompleted(request);
	} else if (request->type == RBackendRequest::PlainGenericRequest) {
		request->setResult(processPlainGenericRequest(request->params["call"].toStringList()));
		RKRBackendProtocolFrontend::setRequestCompleted (request);
	} else if (request->type == RBackendRequest::Started) {
		// The backend thread has finished basic initialization, but we still have more to do...
		startup_errors = request->params["message"].toString ();

		command_requests.append (request);
		RCommandChain *chain = openSubcommandChain (runningCommand ());

		issueCommand ("paste (R.version[c (\"major\", \"minor\")], collapse=\".\")\n", RCommand::GetStringVector | RCommand::App | RCommand::Sync, QString (), this, GET_R_VERSION, chain);
		// find out about standard library locations
		issueCommand ("c(path.expand(Sys.getenv(\"R_LIBS_USER\")), .libPaths())\n", RCommand::GetStringVector | RCommand::App | RCommand::Sync, QString (), this, GET_LIB_PATHS, chain);
		// start help server / determined help base url
		issueCommand (".rk.getHelpBaseUrl ()\n", RCommand::GetStringVector | RCommand::App | RCommand::Sync, QString (), this, GET_HELP_BASE, chain);

		// NOTE: more initialization commands get run *after* we have determined the standard library locations (see rCommandDone())
	} else {
		processRBackendRequest (request);
	}
}

void RInterface::timerEvent (QTimerEvent *) {
// do not trace. called periodically
	flushOutput (false);
}

void RInterface::flushOutput (bool forced) {
// do not trace. called periodically
//	RK_TRACE (RBACKEND);
	ROutputList list = RKRBackendProtocolFrontend::instance ()->flushOutput (forced);

	// this must come _after_ the output has been flushed.
	if (forced || !list.isEmpty ()) {
		if (flush_timer_id != 0) {
			killTimer (flush_timer_id);
			flush_timer_id = 0;
		}
	}

	foreach (ROutput *output, list) {
		if (all_current_commands.isEmpty ()) {
			RK_DEBUG (RBACKEND, DL_DEBUG, "output without receiver'%s'", qPrintable (output->output));
			if (RKConsole::mainConsole()) RKConsole::mainConsole()->insertSpontaneousROutput(output);  // the "if" is to prevent crash, should output arrive during exit
			delete output;
			continue;	// to delete the other output pointers, too
		} else {
			RK_DEBUG (RBACKEND, DL_DEBUG, "output '%s'", qPrintable (output->output));
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
	instance()->_issueCommand(command, chain);
}

void RInterface::_issueCommand(RCommand *command, RCommandChain *chain) { 
	RK_TRACE (RBACKEND);

	if (command->command ().isEmpty ()) command->_type |= RCommand::EmptyCommand;
	if (RKCarbonCopySettings::shouldCarbonCopyCommand (command)) {
		command->_type |= RCommand::CCCommand;
		if (RKCarbonCopySettings::includeOutputInCarbonCopy ()) command->_type |= RCommand::CCOutput;
	}
	RCommandStack::issueCommand (command, chain);
	tryNextCommand ();
}

RCommandChain *RInterface::startChain(RCommandChain *parent) {
	RK_TRACE (RBACKEND);

	return RCommandStack::startChain(parent);
};

void RInterface::closeChain(RCommandChain *chain) {
	RK_TRACE(RBACKEND);

	RCommandStack::closeChain(chain);
	instance()->tryNextCommand();
};

void RInterface::cancelAll () {
	RK_TRACE (RBACKEND);

	QList<RCommand*> all_commands = RCommandStack::regular_stack->allCommands ();
	foreach (RCommand* command, all_commands) cancelCommand (command);
}

bool RInterface::softCancelCommand (RCommand* command) {
	RK_TRACE (RBACKEND);

	if (!(command->status & RCommand::Running)) {
		cancelCommand (command);
	}
	return command->status & RCommand::Canceled;
}

void RInterface::cancelCommand (RCommand *command) {
	RK_TRACE (RBACKEND);

	if (!(command->type () & RCommand::Sync)) {
		command->status |= RCommand::Canceled;
		if (command->status & RCommand::Running) {
			if ((RKDebugHandler::instance ()->state () == RKDebugHandler::InDebugPrompt) && (command == RKDebugHandler::instance ()->command ())) {
				RKDebugHandler::instance ()->sendCancel ();
			} else {
				RKRBackendProtocolFrontend::instance ()->interruptCommand (command->id ());
			}
		}
		RCommandStackModel::getModel ()->itemChange (command);
	} else {
		RK_ASSERT (false);
	}
}

void RInterface::pauseProcessing (bool pause) {
	RK_TRACE (RBACKEND);

	if (pause) locked |= User;
	else locked -= locked & User;
}

GenericRRequestResult RInterface::processPlainGenericRequest(const QStringList &calllist) {
	RK_TRACE (RBACKEND);

	QString call = calllist.value (0);
	if (call == "set.output.file") {
		RK_ASSERT (calllist.count () == 2);
		RKOutputWindowManager::self ()->setCurrentOutputPath (calllist.value (1));
	} else if (call == "wdChange") {
		// in case of separate processes, apply new working directory in frontend, too.
		QDir::setCurrent (calllist.value (1));
		emit backendWorkdirChanged();
	} else if (call == "highlightRCode") {
		return GenericRRequestResult(RKCommandHighlighter::commandToHTML(calllist.value(1)));
	} else if (call == "quit") {
		RKWardMainWindow::getMain ()->close ();
		// if we're still alive, quitting was canceled
		return GenericRRequestResult::makeError(i18n("Quitting was canceled"));
	} else if (call == "preLocaleChange") {
		int res = KMessageBox::warningContinueCancel (0, i18n ("A command in the R backend is trying to change the character encoding. While RKWard offers support for this, and will try to adjust to the new locale, this operation may cause subtle bugs, if data windows are currently open. Also the feature is not well tested, yet, and it may be advisable to save your workspace before proceeding.\nIf you have any data editor opened, or in any doubt, it is recommended to close those first (this will probably be auto-detected in later versions of RKWard). In this case, please choose 'Cancel' now, then close the data windows, save, and retry."), i18n ("Locale change"));
		if (res != KMessageBox::Continue) return GenericRRequestResult::makeError(i18n("Changing the locale was canceled by user"));
	} else if (call == "listPlugins") {
		RK_ASSERT (calllist.count () == 1);
		return GenericRRequestResult(RKComponentMap::getMap()->listPlugins());
	} else if (call == "setPluginStatus") {
		QStringList params = calllist.mid (1);
		RK_ASSERT ((params.size () % 3) == 0);
		const int rows = params.size () / 3;
		QStringList ids = params.mid (0, rows);
		QStringList contexts = params.mid (rows, rows);
		QStringList visible = params.mid (rows*2, rows);
		RKComponentMap::getMap ()->setPluginStatus (ids, contexts, visible);
	} else if (call == "loadPluginMaps") {
		bool force = (calllist.value (1) == "force");
		bool reload = (calllist.value (2) == "reload");
		RKSettingsModulePlugins::registerPluginMaps (calllist.mid (3), force ? RKSettingsModulePlugins::ManualAddition : RKSettingsModulePlugins::AddIfNewAndDefault, reload);
	} else if (call == "updateInstalledPackagesList") {
		RKSessionVars::instance ()->setInstalledPackages (calllist.mid (1));
	} else if (call == "showHTML") {
		RK_ASSERT (calllist.count () == 2);
		RKWorkplace::mainWorkplace ()->openHelpWindow (QUrl::fromUserInput (calllist.value (1), QDir::currentPath (), QUrl::AssumeLocalFile));
	} else if (call == "select.list") {
		QString title = calllist.value (1);
		bool multiple = (calllist.value (2) == "multi");
		int num_preselects = calllist.value (3).toInt ();
		QStringList preselects = calllist.mid (4, num_preselects);
		QStringList choices = calllist.mid (4 + num_preselects);

		QStringList results = RKSelectListDialog::doSelect (QApplication::activeWindow(), title, choices, preselects, multiple);
		if (results.isEmpty ()) results.append ("");	// R wants to have it that way
		return GenericRRequestResult(results);
	} else if (call == "commandHistory") {
		if (calllist.value (1) == "get") {
			return GenericRRequestResult(RKConsole::mainConsole()->commandHistory());
		} else {
			RKConsole::mainConsole ()->setCommandHistory (calllist.mid (2), calllist.value (1) == "append");
		}
	} else if (call == "getWorkspaceUrl") {
		QUrl url = RKWorkplace::mainWorkplace ()->workspaceURL ();
		if (!url.isEmpty()) return GenericRRequestResult(url.url());
	} else if (call == "workplace.layout") {
		if (calllist.value (1) == "set") {
			if (calllist.value (2) == "close") RKWorkplace::mainWorkplace ()->closeAll ();
			QStringList list = calllist.mid (3);
			RKWorkplace::mainWorkplace ()->restoreWorkplace (list);
		} else {
			RK_ASSERT (calllist.value (1) == "get");
			return GenericRRequestResult(RKWorkplace::mainWorkplace ()->makeWorkplaceDescription ());
		}
	} else if (call == "set.window.placement.hint") {
		RKWorkplace::mainWorkplace ()->setWindowPlacementOverrides (calllist.value (1), calllist.value (2), calllist.value (3));
	} else if (call == "getSessionInfo") {
		// Non-translatable on purpose. This is meant for posting to the bug tracker, mostly.
		QStringList lines ("-- Frontend --");
		lines.append (RKSessionVars::frontendSessionInfo ());
		lines.append (QString ());
		lines.append ("-- Backend --");
		lines.append ("Debug message file (this may contain relevant diagnostic output in case of trouble):");
		lines.append (calllist.value (1));
		lines.append (QString ());
		lines.append ("R version (compile time): " + calllist.value (2));
		return GenericRRequestResult(lines);
	} else if (call == "recordCommands") {
		RK_ASSERT (calllist.count () == 3);
		QString filename = calllist.value (1);
		bool unfiltered = (calllist.value (2) == "include.all");

		if (filename.isEmpty ()) {
			command_logfile_mode = NotRecordingCommands;
			command_logfile.close ();
		} else {
			if (command_logfile_mode != NotRecordingCommands) {
				return GenericRRequestResult(QVariant(), i18n("Attempt to start recording, while already recording commands. Ignoring."));
			} else {
				command_logfile.setFileName (filename);
				bool ok = command_logfile.open (QIODevice::WriteOnly | QIODevice::Truncate);
				if (ok) {
					if (unfiltered) command_logfile_mode = RecordingCommandsUnfiltered;
					else command_logfile_mode = RecordingCommands;
				} else {
					return GenericRRequestResult::makeError(i18n("Could not open file for writing. Not recording commands"));
				}
			}
		}
	} else if (call == "printPreview") {
		RKPrintAgent::printPostscript (calllist.value (1), true);
	} else if (call == "endBrowserContext") {
		RKDebugHandler::instance ()->endDebug ();
	} else if (call == "switchLanguage") {
		RKMessageCatalog::switchLanguage (calllist.value (1));
	} else {
		return GenericRRequestResult::makeError(i18n("Error: unrecognized request '%1'", call));
	}

	// for those calls which were recognized, but do not return anything
	return GenericRRequestResult();
}

void RInterface::processHistoricalSubstackRequest (const QStringList &calllist, RCommand *parent_command, RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	RCommandChain *in_chain;
	if (!parent_command) {
		// This can happen for Tcl events. Create a dummy command on the stack to keep things looping.
		parent_command = new RCommand (QString (), RCommand::App | RCommand::EmptyCommand | RCommand::Sync);
		RCommandStack::issueCommand (parent_command, 0);
		all_current_commands.append (parent_command);
		dummy_command_on_stack = parent_command;	// so we can get rid of it again, after it's sub-commands have finished
	}
	in_chain = openSubcommandChain (parent_command);
	RK_DEBUG (RBACKEND, DL_DEBUG, "started sub-command chain (%p) for command %s", in_chain, qPrintable (parent_command->command ()));

	QString call = calllist.value (0);
	if (call == "sync") {
		RK_ASSERT (calllist.count () >= 2);

		for (int i = 1; i < calllist.count (); ++i) {
			QString object_name = calllist[i];
			RObject *obj = RObjectList::getObjectList ()->findObject (object_name);
			if (obj) {
				RK_DEBUG (RBACKEND, DL_DEBUG, "triggering update for symbol %s", object_name.toLatin1 ().data());
				obj->markDataDirty ();
				obj->updateFromR (in_chain);
			} else {
				RK_DEBUG (RBACKEND, DL_WARNING, "lookup failed for changed symbol %s", object_name.toLatin1 ().data());
			}
			if (!RKSettingsModuleObjectBrowser::inWorkspaceModifiedIgnoreList(object_name)) {
				RObjectList::getObjectList()->setWorkspaceModified(true);
			}
		}
	} else if (call == "syncenvs") {
		RK_DEBUG (RBACKEND, DL_DEBUG, "triggering update of object list");
		int search_len = calllist.value (1).toInt ();
		RObjectList::getObjectList ()->updateFromR (in_chain, calllist.mid (2, search_len), calllist.mid (2 + search_len));
	} else if (call == "syncglobal") {
		RK_DEBUG (RBACKEND, DL_DEBUG, "triggering update of globalenv");
		RObjectList::getGlobalEnv ()->updateFromR (in_chain, calllist.mid (1));
#ifndef DISABLE_RKWINDOWCATCHER
	// NOTE: WARNING: When converting these to PlainGenericRequests, the occasional "error, figure margins too large" starts coming up, again. Not sure, why.
 	} else if (call == "startOpenX11") {
		RK_ASSERT (calllist.count () == 2);
		RKWindowCatcher::instance ()->start (calllist.value (1).toInt ());
 	} else if (call == "endOpenX11") {
		RK_ASSERT (calllist.count () == 2);
		RKWindowCatcher::instance ()->stop (calllist.value (1).toInt ());
	} else if (call == "updateDeviceHistory") {
		if (calllist.count () >= 2) {
			RKWindowCatcher::instance ()->updateHistory (calllist.mid (1));
		}
	} else if (call == "killDevice") {
		RK_ASSERT (calllist.count () == 2);
		RKWindowCatcher::instance ()->killDevice (calllist.value (1).toInt ());
#endif // DISABLE_RKWINDOWCATCHER
	} else if (call == "edit") {
		RK_ASSERT (calllist.count () >= 2);

		QStringList object_list = calllist.mid (1);
		new RKEditObjectAgent (object_list, in_chain);
	} else if (call == "require") {
		RK_ASSERT (calllist.count () == 2);
		QString lib_name = calllist.value (1);
		KMessageBox::information (0, i18n ("The R-backend has indicated that in order to carry out the current task it needs the package '%1', which is not currently installed. We will open the package-management tool, and there you can try to locate and install the needed package.", lib_name), i18n ("Require package '%1'", lib_name));
		RKLoadLibsDialog::showInstallPackagesModal (0, in_chain, QStringList(lib_name));
	} else if (call == "doPlugin") {
		if (calllist.count () >= 3) {
			QString message;
			bool ok;
			RKComponentMap::ComponentInvocationMode mode = RKComponentMap::ManualSubmit;
			if (calllist[2] == "auto") mode = RKComponentMap::AutoSubmit;
			else if (calllist[2] == "submit") mode = RKComponentMap::AutoSubmitOrFail;
			ok = RKComponentMap::invokeComponent (calllist[1], calllist.mid (3), mode, &message, in_chain);

			if (!message.isEmpty ()) {
				request->setResult(GenericRRequestResult(QVariant(), ok ? message : QString(), !ok ? message : QString()));
			}
		} else {
			RK_ASSERT (false);
		}
	} else if (call == QStringLiteral ("output")) {
		request->setResult(RKOutputDirectory::handleRCall(calllist.mid(1), in_chain));
	} else {
		request->setResult(GenericRRequestResult::makeError(i18n("Unrecognized call '%1'", call)));
	}
	
	closeChain (in_chain);
}

int addButtonToBox (QDialog *dialog, QDialogButtonBox *box, QDialogButtonBox::StandardButton which, const QString &text, const QString &def_text, bool is_default) {
	if (text.isEmpty ()) return 0;
	QPushButton *button = box->addButton (which);
	if (text != def_text) button->setText (text);
	if (is_default) button->setDefault (true);
	QObject::connect(button, &QPushButton::clicked, dialog, [dialog, which]() { dialog->done(which); });
	return 1;
}

void RInterface::processRBackendRequest (RBackendRequest *request) {
	RK_TRACE (RBACKEND);

	// first, copy out the type. Allows for easier typing below
	RBackendRequest::RCallbackType type = request->type;

	if (type == RBackendRequest::CommandLineIn) {
		int id = request->params["commandid"].toInt ();
		RCommand *command = all_current_commands.value (0, 0);	// User command will always be the first.
		if ((command == 0) || (command->id () != id)) {
			RK_ASSERT (false);
		} else {
			command->commandLineIn ();
		}
	} else if (type == RBackendRequest::ShowMessage) {
		QString caption = request->params["caption"].toString ();
		QString message = request->params["message"].toString ();
		QString button_yes = request->params["button_yes"].toString ();
		QString button_no = request->params["button_no"].toString ();
		QString button_cancel = request->params["button_cancel"].toString ();
		QString def_button = request->params["default"].toString ();

		// NOTE: In order to support non-modal (information) dialogs, we cannot use KMessageBox or QMessgaeBox, below.
		QDialog* dialog = new QDialog ();
		dialog->setResult (-1);  // We use this to stand for cancelled
		QDialogButtonBox *button_box = new QDialogButtonBox (dialog);
		int button_count = 0;
		button_count += addButtonToBox (dialog, button_box, QDialogButtonBox::Yes, button_yes, "yes", def_button == button_yes);
		button_count += addButtonToBox (dialog, button_box, QDialogButtonBox::No, button_no, "no", def_button == button_no);
		button_count += addButtonToBox (dialog, button_box, QDialogButtonBox::Cancel, button_cancel, "cancel", def_button == button_cancel);
		if (!button_count) { // cannot have no button defined at all
			button_count += addButtonToBox (dialog, button_box, QDialogButtonBox::Ok, "ok", "ok", true);
		}

		bool synchronous = request->synchronous || (button_count > 1);
		KMessageBox::createKMessageBox (dialog, button_box, button_count < 2 ? QMessageBox::Information : QMessageBox::Question, message, QStringList (), QString (), 0, KMessageBox::Notify | KMessageBox::NoExec | KMessageBox::AllowLink);
		dialog->setWindowTitle (caption);

		if (!synchronous) {
			dialog->setAttribute (Qt::WA_DeleteOnClose);
			dialog->show();

			RKRBackendProtocolFrontend::setRequestCompleted (request);
			return;
		} else {
			int result = dialog->exec ();
			QString result_string;
			if (result == QDialogButtonBox::Yes || result == QDialogButtonBox::Ok) result_string = "yes";
			else if (result == QDialogButtonBox::No) result_string = "no";
			else result_string = "cancel";
			request->params["result"] = result_string;
			delete dialog;
		}
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
	} else if (type == RBackendRequest::Debugger) {
		RKDebugHandler::instance ()->debugCall (request, runningCommand ());
		return;		// request will be closed by the debug handler
	} else if ((type == RBackendRequest::ShowFiles) || (type == RBackendRequest::EditFiles)) {
		ShowEditTextFileAgent::showEditFiles (request);
		return;		// we are not done, yet!
	} else if (type == RBackendRequest::ChooseFile) {
		QString filename;
		if (request->params["new"].toBool ()) {
			filename = QFileDialog::getSaveFileName ();
		} else {
			filename = QFileDialog::getOpenFileName ();
		}
		request->params["result"] = QVariant (filename);
	} else if (type == RBackendRequest::SetParamsFromBackend) {
			na_real = request->params["na_real"].toDouble ();
			na_int = request->params["na_int"].toInt ();
	} else if (type == RBackendRequest::BackendExit) {
		if (request->params.value ("regular", QVariant (false)).toBool ()) backend_dead = true;		// regular exit via QuitCommand
		if (!backend_dead) {
			backend_dead = true;
			QString message = request->params["message"].toString ();
			message += i18n ("\nThe R backend will be shut down immediately. This means, you can not use any more functions that rely on it. I.e. you can do hardly anything at all, not even save the workspace (but if you're lucky, R already did that). What you can do, however, is save any open command-files, the output, or copy data out of open data editors. Quit RKWard after that. Sorry!");
			RKErrorDialog::reportableErrorMessage (0, message, QString (), i18n ("R engine has died"), "r_engine_has_died");
			emit backendStatusChanged(Dead);
		}
	} else {
		RK_ASSERT (false);
	}

	RKRBackendProtocolFrontend::setRequestCompleted (request);
}

