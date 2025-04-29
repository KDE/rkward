/*
rkrinterface.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Fri Nov 1 2002
SPDX-FileCopyrightText: 2002-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrinterface.h"

#include "../agents/rkdebughandler.h"
#include "../agents/rkeditobjectagent.h"
#include "../agents/rkprintagent.h"
#include "../agents/showedittextfileagent.h"
#include "../core/renvironmentobject.h"
#include "../core/rkmodificationtracker.h"
#include "../core/robjectlist.h"
#include "../dialogs/rkerrordialog.h"
#include "../dialogs/rkloadlibsdialog.h"
#include "../dialogs/rkreadlinedialog.h"
#include "../dialogs/rkselectlistdialog.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkmessagecatalog.h"
#include "../misc/rkoutputdirectory.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkrapimenu.h"
#include "../plugin/rkcomponentmap.h"
#include "../rkconsole.h"
#include "../rkward.h"
#include "../settings/rkrecenturls.h"
#include "../settings/rksettings.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettingsmodulegraphics.h"
#include "../settings/rksettingsmoduleoutput.h"
#include "../settings/rksettingsmoduleplugins.h"
#include "../settings/rksettingsmoduler.h"
#include "../windows/rcontrolwindow.h"
#include "../windows/rkcommandlog.h"
#include "../windows/rkhtmlwindow.h"
#include "../windows/rkwindowcatcher.h"
#include "../windows/rkworkplace.h"
#include "rcommandstack.h"
#include "rkrbackendprotocol_frontend.h"
#include "rksessionvars.h"

#include "../debug.h"
#include "../version.h"

#include <KLocalizedString>
#include <kmessagebox.h>

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QPushButton>
#include <stdlib.h>

// flush new pieces of output after this period of time:
#define FLUSH_INTERVAL 100

// statics
double RInterface::na_real;
int RInterface::na_int;
RInterface *RInterface::_instance = nullptr;

void RInterface::create() {
	RK_TRACE(RBACKEND);
	RK_ASSERT(_instance == nullptr);
	new RInterface();
}

RInterface::RInterface() {
	RK_TRACE(RBACKEND);

	_instance = this;
	new RCommandStackModel(this);
	RCommandStack::regular_stack = new RCommandStack();
	startup_phase2_error = false;
	command_logfile_mode = NotRecordingCommands;
	previously_idle = false;
	locked = 0;
	backend_dead = false;
	backend_started = false;
	dummy_command_on_stack = nullptr;

	// Create a fake init command. This is the top level command designed to capture all output of the startup sequence.
	// The backend will fetch this command, then send a BackendRequest::Started event. In response to this, we will send further
	// (sub)-commands to set everything up (see there).
	auto fake_c = new RCommand(i18n("R Startup"), RCommand::App | RCommand::Sync | RCommand::ObjectListUpdate, i18n("R Startup"));
	fake_c->whenFinished(this, [this](RCommand *command) {
		if (startup_phase2_error || command->failed()) backend_error.message.append(i18n("<p>\t-An unspecified error occurred that is not yet handled by RKWard. Likely RKWard will not function properly. Please check your setup.</p>\n"));
		if (!backend_error.message.isEmpty()) {
			backend_error.message.prepend(i18n("<p>There was a problem starting the R backend. The following error(s) occurred:</p>\n"));
			backend_error.details = command->fullOutput().replace(u'<', u"&lt;"_s).replace(u'\n', u"<br>"_s);
			backend_error.title = i18n("Error starting R");
			// reporting the error will happen via RKWardMainWindow, which calls the setup wizard in this case
		}
	});
	_issueCommand(fake_c);

	new RKSessionVars(this);
	new RKDebugHandler(this);
	backendprotocol = new RKRBackendProtocolFrontend(this);
	backendprotocol->setupBackend();

	/////// Further initialization commands, which do not necessarily have to run before everything else can be queued, here. ///////
	// NOTE: will receive the list as a call plain generic request from the backend ("updateInstalledPackagesList")
	_issueCommand(new RCommand(QStringLiteral(".rk.get.installed.packages()"), RCommand::App | RCommand::Sync));

	whenAllFinished(this, [this]() { backend_started = !backend_dead; RKSettings::validateSettingsInteractive (); });
}

void RInterface::issueCommand(const QString &command, int type, const QString &rk_equiv, RCommandChain *chain) {
	RK_TRACE(RBACKEND);
	issueCommand(new RCommand(command, type, rk_equiv), chain);
}

RInterface::~RInterface() {
	RK_TRACE(RBACKEND);

	// Don't wait for QObject d'tor to destroy the backend transmitter. It might still try to call functions in the RInterface
	// (noteably, it does call qApp->processEvents().
	backend_dead = true;
	RK_ASSERT(_instance == this);
	delete backendprotocol;
	_instance = nullptr;
	RKWindowCatcher::discardInstance();
}

void RInterface::reportFatalError() {
	RK_TRACE(RBACKEND);

	RKErrorDialog::reportableErrorMessage(nullptr, backend_error.message, backend_error.details, backend_error.title, backend_error.id);
}

bool RInterface::backendIsIdle() {
	RK_TRACE(RBACKEND);

	return (RCommandStack::regular_stack->isEmpty() && (!runningCommand()));
}

RCommand *RInterface::popPreviousCommand(int id) {
	RK_TRACE(RBACKEND);

	RK_ASSERT(!all_current_commands.isEmpty());
	for (int i = all_current_commands.size() - 1; i >= 0; --i) {
		RCommand *ret = all_current_commands[i];
		if (ret->id() == id) {
			RCommandStack::pop(ret);
			all_current_commands.removeAt(i);
			return ret;
		}
	}
	RK_ASSERT(false);
	return nullptr;
}

RCommandChain *RInterface::openSubcommandChain(RCommand *parent_command) {
	RK_TRACE(RBACKEND);

	current_commands_with_subcommands.append(parent_command);
	return RCommandStack::startChain(parent_command);
}

void RInterface::closeSubcommandChain(RCommand *parent_command) {
	RK_TRACE(RBACKEND);

	if (current_commands_with_subcommands.contains(parent_command)) {
		current_commands_with_subcommands.removeAll(parent_command);
		doNextCommand(nullptr);
	}
	if (parent_command && (parent_command == dummy_command_on_stack)) {
		all_current_commands.removeAll(dummy_command_on_stack);
		RCommandStack::pop(dummy_command_on_stack);
		handleCommandOut(dummy_command_on_stack);
		dummy_command_on_stack = nullptr;
	}
}

void RInterface::tryNextCommand() {
	RK_TRACE(RBACKEND);

	RCommand *command = RCommandStack::currentCommand();
	if (command_requests.isEmpty()) {
		// if the backend is not requesting anything, only priority commands will be pushed
		if (!command) return;
		if (!(command->type() & RCommand::PriorityCommand)) return;
		if (all_current_commands.contains(command)) return;
	}

	bool priority = command && (command->type() & RCommand::PriorityCommand);
	bool on_top_level = all_current_commands.isEmpty();
	if (!(on_top_level && locked && !(priority))) {                      // do not respect locks for sub-commands
		if ((!on_top_level) && all_current_commands.contains(command)) { // all sub-commands of the current command have finished, it became the top-most item of the RCommandStack, again
			closeSubcommandChain(command);
			return;
		}

		if (command) {
			all_current_commands.append(command);

			if (command->status & RCommand::Canceled) {
				// avoid passing cancelled commands to R
				command->status |= RCommand::Failed;

				// notify ourselves...
				RCommand *dummy = popPreviousCommand(command->id());
				RK_ASSERT(dummy == command);
				RK_DEBUG(RBACKEND, DL_DEBUG, "Not sending already cancelled command to backend");
				handleCommandOut(command);
				tryNextCommand();
				return;
			}

			if (previously_idle) Q_EMIT backendStatusChanged(Busy);
			previously_idle = false;

			doNextCommand(command);
			return;
		}
	}

	if (on_top_level) {
		if (!previously_idle) Q_EMIT backendStatusChanged(Idle);
		previously_idle = true;
	}
}

void RInterface::handleCommandOut(RCommand *command) {
	RK_TRACE(RBACKEND);

	RK_ASSERT(command);

#ifdef RKWARD_DEBUG
	int dl = DL_WARNING; // failed application commands are an issue worth reporting, failed user commands are not
	if (command->type() & RCommand::User) dl = DL_DEBUG;
	if (command->failed()) {
		command->status |= RCommand::WasTried | RCommand::Failed;
		if (command->status & RCommand::ErrorIncomplete) {
			RK_DEBUG(RBACKEND, dl, "Command failed (incomplete)");
		} else if (command->status & RCommand::ErrorSyntax) {
			RK_DEBUG(RBACKEND, dl, "Command failed (syntax)");
		} else if (command->status & RCommand::Canceled) {
			RK_DEBUG(RBACKEND, dl, "Command failed (interrupted)");
		} else {
			RK_DEBUG(RBACKEND, dl, "Command failed (other)");
		}
		RK_DEBUG(RBACKEND, dl, "failed command was %d: '%s'", command->id(), qPrintable(command->command()));
		RK_DEBUG(RBACKEND, dl, "- error message was: '%s'", qPrintable(command->error()));
	}
#endif

	if (command->status & RCommand::Canceled) {
		command->status |= RCommand::HasError;
		ROutput *out = new ROutput;
		out->type = ROutput::Error;
		out->output = u"--- interrupted ---"_s;
		command->output_list.append(out);
		command->newOutput(out);
	}
	command->finished();
	delete command;
}

void RInterface::doNextCommand(RCommand *command) {
	RK_TRACE(RBACKEND);
	RBackendRequest *command_request = currentCommandRequest();
	if (!command_request) {
		if (!(command && (command->type() & RCommand::PriorityCommand))) return;
	}
	// importantly, this point is not reached for the fake startup command

	if (RK_Debug::RK_Debug_CommandStep) {
		QElapsedTimer t;
		t.start();
		while (t.elapsed() < RK_Debug::RK_Debug_CommandStep) {
		}
	}

	flushOutput(true);
	RCommandProxy *proxy = nullptr;
	if (command) {
		RKWardMainWindow::getMain()->setWorkspaceMightBeModified(true);
		proxy = command->makeProxy();

		RK_DEBUG(RBACKEND, DL_DEBUG, "running command %d: %s", command->id(), qPrintable(command->command()));
		command->status |= RCommand::Running;
		RCommandStackModel::getModel()->itemChange(command);

		RKCommandLog::getLog()->addInput(command);

		if (command_logfile_mode != NotRecordingCommands) {
			bool record = true;
			if (command_logfile_mode != RecordingCommandsUnfiltered) {
				if (command->type() & (RCommand::Silent | RCommand::Sync)) record = false;
			}
			if (record) {
				command_logfile.write(command->command().toUtf8());
				command_logfile.write("\n");
			}
		}
	}

	if (command && (command->type() & RCommand::PriorityCommand)) {
		RKRBackendProtocolFrontend::sendPriorityCommand(proxy);
	} else {
		RK_ASSERT(command_request);
		command_request->command = proxy;
		RKRBackendProtocolFrontend::setRequestCompleted(command_request);
		command_requests.pop_back();
	}
}

void RInterface::runStartupCommand(RCommand *command, RCommandChain *chain, std::function<void(RCommand *)> callback) {
	command->whenFinished(this, [this, callback](RCommand *command) {
		RK_TRACE(RBACKEND);
		if (command->failed()) {
			startup_phase2_error = true;
		} else {
			callback(command);
		}
	});
	_issueCommand(command, chain);
}

void RInterface::handleRequest(RBackendRequest *request) {
	RK_TRACE(RBACKEND);

	if (request->type == RBackendRequest::OutputStartedNotification) {
		// We do _not_ flush the output right away, as it is likely to arrive in minuscule chunks. But we _do_ want to check, soon
		QTimer::singleShot(FLUSH_INTERVAL, this, [this]() { flushOutput(false); });
		RKRBackendProtocolFrontend::setRequestCompleted(request);
		return;
	}

	flushOutput(true);
	if (request->type == RBackendRequest::CommandOut) {
		if (backend_dead) return; // Backend may or may not be able to transmit finished commands after exit, and therefore we've already discarded all active commands
		RCommandProxy *cproxy = request->takeCommand();
		if (cproxy) {
			RK_DEBUG(RBACKEND, DL_DEBUG, "Command out \"%s\", id %d", qPrintable(cproxy->command), cproxy->id);
		} else {
			RK_DEBUG(RBACKEND, DL_DEBUG, "Fake command out");
		}
		RCommand *command = nullptr;
		// NOTE: the order of processing is: first try to submit the next command, then handle the old command.
		// The reason for doing it this way, instead of the reverse, is that this allows the backend thread / process to continue working, concurrently
		// NOTE: cproxy should only ever be 0 in the very first cycle
		if (cproxy) command = popPreviousCommand(cproxy->id);
		if (request->synchronous) command_requests.append(request);
		tryNextCommand();
		if (cproxy) {
			RK_ASSERT(command);
			command->mergeAndDeleteProxy(cproxy);
			handleCommandOut(command);
		}
		tryNextCommand();
	} else if (request->type == RBackendRequest::RCallRequest) {
		const auto params = request->params;

		auto subcommandrequest = request->subcommandrequest;
		RCommandChain *in_chain = RCommandStack::regular_stack;
		if (subcommandrequest) {
			int id = params.value(QStringLiteral("cid")).toInt();
			RCommand *parent = nullptr;
			for (int i = all_current_commands.size() - 1; i >= 0; --i) {
				if (all_current_commands[i]->id() == id) {
					parent = all_current_commands[i];
					break;
				}
			}
			if (!parent) {
				// This can happen for Tcl events. Create a dummy command on the stack to keep things looping.
				parent = new RCommand(QString(), RCommand::App | RCommand::EmptyCommand | RCommand::Sync);
				RCommandStack::issueCommand(parent, nullptr);
				all_current_commands.append(parent);
				dummy_command_on_stack = parent; // so we can get rid of it again, after it's sub-commands have finished
			}
			in_chain = openSubcommandChain(parent);
			RK_DEBUG(RBACKEND, DL_DEBUG, "started sub-command chain (%p) for command %s", in_chain, qPrintable(parent->command()));

			command_requests.append(request->subcommandrequest);
			request->subcommandrequest = nullptr; // it is now a separate request. Make sure we won't try to send it back as part of this one.
		}

		request->setResult(processRCallRequest(params.value(QStringLiteral("call")).toString(), params.value(QStringLiteral("args")), in_chain));

		if (subcommandrequest) closeChain(in_chain);

		RKRBackendProtocolFrontend::setRequestCompleted(request);
	} else if (request->type == RBackendRequest::Started) {
		// The backend thread has finished basic initialization, but we still have more to do...
		backend_error.message.append(request->params[QStringLiteral("message")].toString());

		command_requests.append(request);
		RCommandChain *chain = openSubcommandChain(runningCommand());

		runStartupCommand(new RCommand(QStringLiteral("paste (R.version[c (\"major\", \"minor\")], collapse=\".\")\n"), RCommand::GetStringVector | RCommand::App | RCommand::Sync), chain,
		                  [](RCommand *command) {
			                  RK_ASSERT(command->getDataType() == RData::StringVector);
			                  RK_ASSERT(command->getDataLength() == 1);
			                  RKSessionVars::setRVersion(command->stringVector().value(0));
		                  });

		if (qEnvironmentVariableIsSet("APPDIR")) {
			// Running inside an AppImage. As soon as R has started, it should behave as if running in the main (system) environment (esp. when calling helper binaries such as wget or gcc).
			// Unset any paths starting with APPDIR, _except_ those inside R_HOME.
			runStartupCommand(new RCommand(QStringLiteral("local({\n"
			                                              "	appdir <- Sys.getenv(\"APPDIR\")\n"
			                                              "	fix <- function(key) {\n"
			                                              "		paths <- strsplit(Sys.getenv(key), \":\", fixed=TRUE)[[1]]\n"
			                                              "		paths <- paths[!(startsWith(paths, appdir) & !startsWith(paths, R.home()))]\n"
			                                              "		patharg <- list(paste(paths, collapse=\":\"))\n"
			                                              "		names(patharg) <- key\n"
			                                              "		do.call(Sys.setenv, patharg)\n"
			                                              "	}\n"
			                                              "	fix(\"LD_LIBRARY_PATH\")\n"
			                                              "	fix(\"PATH\")\n"
			                                              "})\n"),
			                               RCommand::App | RCommand::Sync),
			                  chain, [](RCommand *) {});
		}

		// find out about standard library locations
		runStartupCommand(new RCommand(QStringLiteral("c(path.expand(Sys.getenv(\"R_LIBS_USER\")), .libPaths())\n"), RCommand::GetStringVector | RCommand::App | RCommand::Sync), chain,
		                  [this](RCommand *command) {
			                  RK_ASSERT(command->getDataType() == RData::StringVector);
			                  RKSettingsModuleRPackages::r_libs_user = command->stringVector().value(0);
			                  RKSettingsModuleRPackages::defaultliblocs = command->stringVector().mid(1);

			                  RCommandChain *chain = command->parent;
			                  RK_ASSERT(chain);
			                  RK_ASSERT(!chain->isClosed());

			                  // apply user configurable run time options
			                  auto runtimeopt_callback = [](RCommand *) {}; // No special handling. Any failure will be recorded with runStartupCommand().
			                  QStringList commands = RKSettingsModuleR::makeRRunTimeOptionCommands() + RKSettingsModuleRPackages::makeRRunTimeOptionCommands() + RKSettingsModuleOutput::makeRRunTimeOptionCommands() + RKSettingsModuleGraphics::makeRRunTimeOptionCommands();
			                  for (QStringList::const_iterator it = commands.cbegin(); it != commands.cend(); ++it) {
				                  runStartupCommand(new RCommand(*it, RCommand::App | RCommand::Sync), chain, runtimeopt_callback);
			                  }
			                  // initialize output file
			                  RKOutputDirectory::getCurrentOutput(chain);

			                  // Workaround for https://bugs.kde.org/show_bug.cgi?id=421958
			                  if (RKSessionVars::compareRVersion(QStringLiteral("4.0.0")) < 1 && RKSessionVars::compareRVersion(QStringLiteral("4.0.1")) > 0) {
				                  runStartupCommand(new RCommand(QStringLiteral("if(compiler::enableJIT(-1) > 2) compiler::enableJIT(2)\n"), RCommand::App | RCommand::Sync), chain, runtimeopt_callback);
			                  }

			                  closeChain(chain);
		                  });

		// start help server / determined help base url
		runStartupCommand(new RCommand(QStringLiteral(".rk.getHelpBaseUrl ()\n"), RCommand::GetStringVector | RCommand::App | RCommand::Sync), chain,
		                  [](RCommand *command) {
			                  RK_ASSERT(command->getDataType() == RData::StringVector);
			                  RK_ASSERT(command->getDataLength() == 1);
			                  RKSettingsModuleR::help_base_url = command->stringVector().value(0);
		                  });

		QString cd_to = RKSettingsModuleGeneral::initialWorkingDirectory();
		if (cd_to.isEmpty()) cd_to = QDir::currentPath();
		if (cd_to.isEmpty()) cd_to = QStringLiteral("."); // we must be in a non-existent dir. cd'ing to "." will cause us to sync with the backend
		RInterface::issueCommand(new RCommand(u"setwd("_s + RObject::rQuote(cd_to) + u")\n"_s, RCommand::App | RCommand::Sync), chain);
	} else {
		processRBackendRequest(request);
	}
}

void RInterface::flushOutput(bool forced) {
	// do not trace. called periodically
	//	RK_TRACE (RBACKEND);
	const ROutputList list = backendprotocol->flushOutput(forced);

	for (ROutput *output : list) {
		if (all_current_commands.isEmpty()) {
			RK_DEBUG(RBACKEND, DL_DEBUG, "output without receiver'%s'", qPrintable(output->output));
			if (RKConsole::mainConsole()) RKConsole::mainConsole()->insertSpontaneousROutput(output); // the "if" is to prevent crash, should output arrive during exit
			delete output;
			continue; // to delete the other output pointers, too
		} else {
			RK_DEBUG(RBACKEND, DL_DEBUG, "output '%s'", qPrintable(output->output));
		}

		bool first = true;
		for (RCommand *command : std::as_const(all_current_commands)) {
			ROutput *coutput = output;
			if (!first) { // this output belongs to several commands at once. So we need to copy it.
				coutput = new ROutput;
				coutput->type = output->type;
				coutput->output = output->output;
			}
			first = false;

			if (coutput->type == ROutput::Output) {
				command->status |= RCommand::HasOutput;
				command->output_list.append(coutput);
			} else if (coutput->type == ROutput::Warning) {
				command->status |= RCommand::HasWarnings;
				command->output_list.append(coutput);
			} else if (coutput->type == ROutput::Error) {
				command->status |= RCommand::HasError;
				// An error output is typically just the copy of the previous output, so merge if possible
				if (command->output_list.isEmpty()) {
					command->output_list.append(coutput);
				}
				if (command->output_list.last()->output == coutput->output) {
					command->output_list.last()->type = ROutput::Error;
					continue; // don't call command->newOutput(), again!
				}
			}
			command->newOutput(coutput);
		}
	}
}

void RInterface::issueCommand(RCommand *command, RCommandChain *chain) {
	instance()->_issueCommand(command, chain);
}

void RInterface::_issueCommand(RCommand *command, RCommandChain *chain) {
	RK_TRACE(RBACKEND);

	if (backend_dead) {
		command->status |= RCommand::Failed;
		handleCommandOut(command);
		return;
	}

	if (command->command().isEmpty()) command->_type |= RCommand::EmptyCommand;
	if (RKCarbonCopySettings::shouldCarbonCopyCommand(command)) {
		command->_type |= RCommand::CCCommand;
		if (RKCarbonCopySettings::includeOutputInCarbonCopy()) command->_type |= RCommand::CCOutput;
	}
	RCommandStack::issueCommand(command, chain);
	tryNextCommand();
}

RCommandChain *RInterface::startChain(RCommandChain *parent) {
	RK_TRACE(RBACKEND);

	return RCommandStack::startChain(parent);
};

void RInterface::closeChain(RCommandChain *chain) {
	RK_TRACE(RBACKEND);

	RCommandStack::closeChain(chain);
	instance()->tryNextCommand();
};

void RInterface::cancelAll() {
	RK_TRACE(RBACKEND);

	const QList<RCommand *> all_commands = RCommandStack::regular_stack->allCommands();
	for (RCommand *command : all_commands) {
		if (!(command->type() & RCommand::Sync)) cancelCommand(command);
	}
}

bool RInterface::softCancelCommand(RCommand *command) {
	RK_TRACE(RBACKEND);

	if (!(command->status & RCommand::Running)) {
		cancelCommand(command);
	}
	return command->status & RCommand::Canceled;
}

void RInterface::cancelCommand(RCommand *command) {
	RK_TRACE(RBACKEND);

	if (!(command->type() & RCommand::Sync)) {
		command->status |= RCommand::Canceled;
		if (command->status & RCommand::Running) {
			if ((RKDebugHandler::instance()->state() == RKDebugHandler::InDebugPrompt) && (command == RKDebugHandler::instance()->command())) {
				RK_DEBUG(RBACKEND, DL_DEBUG, "Interrupting debug prompt");
				RKDebugHandler::instance()->sendCancel();
			} else {
				RK_DEBUG(RBACKEND, DL_DEBUG, "Interrupting running command %d", command->id());
				backendprotocol->interruptCommand(command->id());
			}
		}
		RCommandStackModel::getModel()->itemChange(command);
	} else {
		RK_ASSERT(false);
	}
	tryNextCommand();
}

void RInterface::pauseProcessing(bool pause) {
	RK_TRACE(RBACKEND);

	if (pause) locked |= User;
	else locked -= locked & User;
}

GenericRRequestResult RInterface::processRCallRequest(const QString &call, const QVariant &args, RCommandChain *in_chain) {
	RK_TRACE(RBACKEND);

	// TODO: All these calls should be mapped to an enum directly in the backend, for efficiency, and to avoid typo-bugs
	if (call == QLatin1String("sync")) {
		QVariantList al = args.toList(); // added, removed, changed
		RObjectList::getGlobalEnv()->updateFromR(in_chain, al.value(0).toStringList(), al.value(1).toStringList());
		QStringList changed = al.value(2).toStringList();
		RK_DEBUG(RBACKEND, DL_DEBUG, "symbols added %s, removed %s, changed %s", qPrintable(al.value(0).toStringList().join(u","_s)), qPrintable(al.value(1).toStringList().join(u","_s)), qPrintable(al.value(2).toStringList().join(u","_s)));
		for (int i = 0; i < changed.count(); ++i) {
			QString object_name = changed[i];
			RObject *obj = RObjectList::getObjectList()->findObject(object_name);
			if (obj) {
				RK_DEBUG(RBACKEND, DL_DEBUG, "triggering update for symbol %s", qPrintable(object_name));
				obj->markDataDirty();
				obj->updateFromR(in_chain);
			} else {
				RK_DEBUG(RBACKEND, DL_WARNING, "lookup failed for changed symbol %s", qPrintable(object_name));
			}
		}
		return GenericRRequestResult();
	}
	if (call == QLatin1String("syncenvs")) {
		QVariantList al = args.toList(); // envs, namespaces
		RK_DEBUG(RBACKEND, DL_DEBUG, "triggering update of object list");
		RObjectList::getObjectList()->updateFromR(in_chain, al.value(0).toStringList(), al.value(1).toStringList());
		return GenericRRequestResult();
	}

	// NOTE: all requests below pass their arguments as single stringlist (although that may be worth changing)
	QStringList arglist = args.toStringList();
	if (false) { // syntax dummy
#ifndef DISABLE_RKWINDOWCATCHER
		// NOTE: WARNING: When converting these to PlainGenericRequests, the occasional "error, figure margins too large" starts coming up, again. Not sure, why.
	} else if (call == QLatin1String("startOpenX11")) {
		RK_ASSERT(arglist.count() == 1);
		RKWindowCatcher::instance()->start(arglist.value(0).toInt());
	} else if (call == QLatin1String("endOpenX11")) {
		RK_ASSERT(arglist.count() == 1);
		RKWindowCatcher::instance()->stop(arglist.value(0).toInt());
	} else if (call == QLatin1String("updateDeviceHistory")) {
		if (!arglist.isEmpty()) {
			RKWindowCatcher::instance()->updateHistory(arglist);
		}
	} else if (call == QLatin1String("killDevice")) {
		RK_ASSERT(arglist.count() == 1);
		RKWindowCatcher::instance()->killDevice(arglist.value(0).toInt());
#endif // DISABLE_RKWINDOWCATCHER
	} else if (call == QLatin1String("edit")) {
		RK_ASSERT(!arglist.isEmpty());
		new RKEditObjectAgent(arglist, in_chain);
	} else if (call == QLatin1String("require")) {
		RK_ASSERT(!arglist.isEmpty());
		if (!RKWardMainWindow::suppressModalDialogsForTesting()) {
			QString lib_name = arglist.value(0);
			KMessageBox::information(nullptr, i18n("The R-backend has indicated that in order to carry out the current task it needs the package '%1', which is not currently installed. We will open the package-management tool, and there you can try to locate and install the needed package.", lib_name), i18n("Require package '%1'", lib_name));
			RKLoadLibsDialog::showInstallPackagesModal(nullptr, in_chain, QStringList(lib_name));
		}
	} else if (call == QLatin1String("doPlugin")) {
		if (arglist.count() >= 2) {
			QString message;
			bool ok;
			RKComponentMap::ComponentInvocationMode mode = RKComponentMap::ManualSubmit;
			if (arglist[1] == QLatin1String("auto")) mode = RKComponentMap::AutoSubmit;
			else if (arglist[1] == QLatin1String("submit")) mode = RKComponentMap::AutoSubmitOrFail;
			ok = RKComponentMap::invokeComponent(arglist[0], arglist.mid(2), mode, &message, in_chain);

			if (!message.isEmpty()) {
				return (GenericRRequestResult(QVariant(), ok ? message : QString(), !ok ? message : QString()));
			}
		} else {
			RK_ASSERT(false);
		}
	} else if (call == QStringLiteral("output")) {
		return (RKOutputDirectory::handleRCall(arglist, in_chain));
	} else if (call == QLatin1String("set.output.file")) {
		RK_ASSERT(arglist.count() == 1);
		RKOutputWindowManager::self()->setCurrentOutputPath(arglist.value(0));
	} else if (call == QLatin1String("wdChange")) {
		// apply new working directory in frontend, too.
		QDir::setCurrent(arglist.value(0));
		Q_EMIT backendWorkdirChanged();
	} else if (call == QLatin1String("highlightRCode")) {
		return GenericRRequestResult(RKCommandHighlighter::commandToHTML(arglist.join(u'\n')));
	} else if (call == QLatin1String("quit")) {
		RKWardMainWindow::getMain()->close();
		// if we're still alive, quitting was canceled
		return GenericRRequestResult::makeError(i18n("Quitting was canceled"));
	} else if (call == QLatin1String("preLocaleChange")) {
		int res = KMessageBox::warningContinueCancel(nullptr, i18n("A command in the R backend is trying to change the character encoding. While RKWard offers support for this, and will try to adjust to the new locale, this operation may cause subtle bugs, if data windows are currently open. Also the feature is not well tested, yet, and it may be advisable to save your workspace before proceeding.\nIf you have any data editor opened, or in any doubt, it is recommended to close those first (this will probably be auto-detected in later versions of RKWard). In this case, please choose 'Cancel' now, then close the data windows, save, and retry."), i18n("Locale change"));
		if (res != KMessageBox::Continue) return GenericRRequestResult::makeError(i18n("Changing the locale was canceled by user"));
	} else if (call == QLatin1String("listPlugins")) {
		RK_ASSERT(arglist.isEmpty());
		return GenericRRequestResult(RKComponentMap::getMap()->listPlugins());
	} else if (call == QLatin1String("setPluginStatus")) {
		// TODO: Pass args in a saner way
		RK_ASSERT((arglist.size() % 3) == 0);
		const int rows = arglist.size() / 3;
		QStringList ids = arglist.mid(0, rows);
		QStringList contexts = arglist.mid(rows, rows);
		QStringList visible = arglist.mid(rows * 2, rows);
		RKComponentMap::getMap()->setPluginStatus(ids, contexts, visible);
	} else if (call == QLatin1String("loadPluginMaps")) {
		bool force = (arglist.value(0) == QLatin1String("force"));
		bool reload = (arglist.value(1) == QLatin1String("reload"));
		RKSettingsModulePlugins::registerPluginMaps(arglist.mid(2), force ? RKSettingsModulePlugins::ForceActivate : RKSettingsModulePlugins::AutoActivateIfNew, reload);
	} else if (call == QLatin1String("updateInstalledPackagesList")) {
		RKSessionVars::instance()->setInstalledPackages(arglist);
	} else if (call == QLatin1String("showHTML")) {
		if (arglist.size() == 1) RKWorkplace::mainWorkplace()->openHelpWindow(QUrl::fromUserInput(arglist.value(0), QDir::currentPath(), QUrl::AssumeLocalFile));
		else {
			auto win = qobject_cast<RKHTMLWindow *>(RKWorkplace::mainWorkplace()->openHelpWindow(QUrl()));
			RK_ASSERT(win);
			win->setContent(arglist.value(1));
		}
	} else if (call == QLatin1String("showPDF")) {
		RK_ASSERT(arglist.size() == 1);
		RKWorkplace::mainWorkplace()->openPDFWindow(QUrl::fromUserInput(arglist.value(0), QDir::currentPath(), QUrl::AssumeLocalFile));
	} else if (call == QLatin1String("select.list")) {
		QString title = arglist.value(0);
		bool multiple = (arglist.value(1) == QLatin1String("multi"));
		int num_preselects = arglist.value(2).toInt();
		QStringList preselects = arglist.mid(3, num_preselects);
		QStringList choices = arglist.mid(3 + num_preselects);

		QStringList results = RKSelectListDialog::doSelect(QApplication::activeWindow(), title, choices, preselects, multiple);
		if (results.isEmpty()) results.append(QLatin1String("")); // R wants to have it that way
		return GenericRRequestResult(results);
	} else if (call == QLatin1String("choosefile")) {
		QFileDialog d(nullptr, arglist.value(0)); // caption
		QString initial = arglist.value(1);
		QString cat;
		if (initial.startsWith(u'#')) {
			cat = initial.mid(1);
			initial = QFileInfo(RKRecentUrls::mostRecentUrl(cat).toLocalFile()).absolutePath();
		}

		d.setDirectory(initial);
		QString filter = arglist.value(2);
		if (!filter.isEmpty()) {
			if (!filter.contains(u'(')) filter += u'(' + filter + u')';
			d.setNameFilter(filter);
		}
		QString mode = arglist.value(3);
		if (mode == QLatin1String("file")) d.setFileMode(QFileDialog::ExistingFile);
		else if (mode == QLatin1String("files")) d.setFileMode(QFileDialog::ExistingFiles);
		else if (mode == QLatin1String("dir")) d.setFileMode(QFileDialog::Directory);
		else if (mode == QLatin1String("newfile")) {
			d.setFileMode(QFileDialog::AnyFile);
			d.setAcceptMode(QFileDialog::AcceptSave);
		} else RK_ASSERT(false);

		d.exec();
		auto res = d.selectedFiles();
		if (!res.isEmpty() && !cat.isEmpty()) {
			RKRecentUrls::addRecentUrl(cat, QUrl::fromLocalFile(res.value(0)));
		}

		return GenericRRequestResult(res);
	} else if (call == QLatin1String("commandHistory")) {
		if (arglist.value(0) == QLatin1String("get")) {
			return GenericRRequestResult(RKConsole::mainConsole()->commandHistory());
		} else {
			RKConsole::mainConsole()->setCommandHistory(arglist.mid(1), arglist.value(0) == QLatin1String("append"));
		}
	} else if (call == QLatin1String("getWorkspaceUrl")) {
		QUrl url = RKWorkplace::mainWorkplace()->workspaceURL();
		if (!url.isEmpty()) return GenericRRequestResult(url.url());
	} else if (call == QLatin1String("workplace.layout")) {
		if (arglist.value(0) == QLatin1String("set")) {
			if (arglist.value(1) == QLatin1String("close")) RKWorkplace::mainWorkplace()->closeAll();
			QStringList list = arglist.mid(2);
			RKWorkplace::mainWorkplace()->restoreWorkplace(list);
		} else {
			RK_ASSERT(arglist.value(0) == "get"_L1);
			return GenericRRequestResult(RKWorkplace::mainWorkplace()->makeWorkplaceDescription());
		}
	} else if (call == QLatin1String("set.window.placement.hint")) {
		RKWorkplace::mainWorkplace()->setWindowPlacementOverrides(arglist.value(0), arglist.value(1), arglist.value(2));
	} else if (call == QLatin1String("frontendSessionInfo")) {
		return GenericRRequestResult(RKSessionVars::frontendSessionInfo());
	} else if (call == QLatin1String("recordCommands")) {
		RK_ASSERT(arglist.count() == 2);
		QString filename = arglist.value(0);
		bool unfiltered = (arglist.value(1) == QLatin1String("include.all"));

		if (filename.isEmpty()) {
			command_logfile_mode = NotRecordingCommands;
			command_logfile.close();
		} else {
			if (command_logfile_mode != NotRecordingCommands) {
				return GenericRRequestResult(QVariant(), i18n("Attempt to start recording, while already recording commands. Ignoring."));
			} else {
				command_logfile.setFileName(filename);
				bool ok = command_logfile.open(QIODevice::WriteOnly | QIODevice::Truncate);
				if (ok) {
					if (unfiltered) command_logfile_mode = RecordingCommandsUnfiltered;
					else command_logfile_mode = RecordingCommands;
				} else {
					return GenericRRequestResult::makeError(i18n("Could not open file for writing. Not recording commands"));
				}
			}
		}
	} else if (call == QLatin1String("printPreview")) {
		RKPrintAgent::printPostscript(arglist.value(0), true);
	} else if (call == QLatin1String("endBrowserContext")) {
		RKDebugHandler::instance()->endDebug();
	} else if (call == QLatin1String("switchLanguage")) {
		RKMessageCatalog::switchLanguage(arglist.value(0));
	} else if (call == QLatin1String("menuupdate")) {
		RKWardMainWindow::getMain()->rApiMenu()->updateFromR(args.toList());
	} else if (call == QLatin1String("menuenable")) {
		auto path = args.toList().value(0).toStringList();
		auto enable = args.toList().value(1).toList().value(0).toBool(); // NOTE: bool value is a vector of ints, in R, therefore the toList(), first
		auto show = args.toList().value(2).toList().value(0).toBool();
		RKWardMainWindow::getMain()->rApiMenu()->enableAction(path, enable, show);
	} else if (call == QLatin1String("with.progress")) {
		auto dialog = new RKProgressControl(this, arglist.value(0), QString(), RKProgressControl::CancellableProgress | RKProgressControl::OutputShownByDefault);
		auto command = new RCommand(QStringLiteral("rkward:::.rk.with.progress.eval()"), RCommand::App);
		dialog->addRCommand(command, true);
		issueCommand(command, in_chain);
		dialog->doNonModal(true);
	} else {
		return GenericRRequestResult::makeError(i18n("Error: unrecognized request '%1'", call));
	}

	// for those calls which were recognized, but do not return anything
	return GenericRRequestResult();
}

int addButtonToBox(QDialog *dialog, QDialogButtonBox *box, QDialogButtonBox::StandardButton which, const QString &text, const QString &def_text, bool is_default) {
	if (text.isEmpty()) return 0;
	QPushButton *button = box->addButton(which);
	if (text != def_text) button->setText(text);
	if (is_default) button->setDefault(true);
	QObject::connect(button, &QPushButton::clicked, dialog, [dialog, which]() { dialog->done(which); });
	return 1;
}

void RInterface::processRBackendRequest(RBackendRequest *request) {
	RK_TRACE(RBACKEND);

	// first, copy out the type. Allows for easier typing below
	RBackendRequest::RCallbackType type = request->type;

	if (type == RBackendRequest::CommandLineIn) {
		int id = request->params[QStringLiteral("commandid")].toInt();
		RCommand *command = all_current_commands.value(0, nullptr); // User command will always be the first.
		if ((command == nullptr) || (command->id() != id)) {
			RK_ASSERT(false);
		} else {
			command->commandLineIn();
		}
	} else if (type == RBackendRequest::ShowMessage) {
		QString caption = request->params[QStringLiteral("caption")].toString();
		QString message = request->params[QStringLiteral("message")].toString();
		QString button_yes = request->params[QStringLiteral("button_yes")].toString();
		QString button_no = request->params[QStringLiteral("button_no")].toString();
		QString button_cancel = request->params[QStringLiteral("button_cancel")].toString();
		QString def_button = request->params[QStringLiteral("default")].toString();

		// NOTE: In order to support non-modal (information) dialogs, we cannot use KMessageBox or QMessgaeBox, below.
		QDialog *dialog = new QDialog();
		dialog->setResult(-1); // We use this to stand for cancelled
		QDialogButtonBox *button_box = new QDialogButtonBox(dialog);
		int button_count = 0;
		button_count += addButtonToBox(dialog, button_box, QDialogButtonBox::Yes, button_yes, QStringLiteral("yes"), def_button == button_yes);
		button_count += addButtonToBox(dialog, button_box, QDialogButtonBox::No, button_no, QStringLiteral("no"), def_button == button_no);
		button_count += addButtonToBox(dialog, button_box, QDialogButtonBox::Cancel, button_cancel, QStringLiteral("cancel"), def_button == button_cancel);
		if (!button_count) { // cannot have no button defined at all
			button_count += addButtonToBox(dialog, button_box, QDialogButtonBox::Ok, QStringLiteral("ok"), QStringLiteral("ok"), true);
		}

		bool synchronous = request->synchronous || (button_count > 1);
		KMessageBox::createKMessageBox(dialog, button_box, button_count < 2 ? QMessageBox::Information : QMessageBox::Question, message, QStringList(), QString(), nullptr, KMessageBox::Notify | KMessageBox::NoExec | KMessageBox::AllowLink);
		dialog->setWindowTitle(caption);

		if (!synchronous) {
			dialog->setAttribute(Qt::WA_DeleteOnClose);
			dialog->show();

			RKRBackendProtocolFrontend::setRequestCompleted(request);
			return;
		} else {
			int result = RKWardMainWindow::suppressModalDialogsForTesting() ? QDialogButtonBox::Cancel : dialog->exec();
			QString result_string;
			if (result == QDialogButtonBox::Yes || result == QDialogButtonBox::Ok) result_string = QLatin1String("yes");
			else if (result == QDialogButtonBox::No) result_string = QLatin1String("no");
			else result_string = QLatin1String("cancel");
			request->params[QStringLiteral("result")] = result_string;
			delete dialog;
		}
	} else if (type == RBackendRequest::ReadLine) {
		QString result;

		// yes, readline *can* be called outside of a current command (e.g. from tcl/tk)
		bool dummy_command = false;
		RCommand *command = runningCommand();
		if (!command) {
			command = new RCommand(QLatin1String(""), RCommand::EmptyCommand);
			dummy_command = true;
		}

		bool ok = RKReadLineDialog::readLine(nullptr, i18n("R backend requests information"), request->params[QStringLiteral("prompt")].toString(), command, &result);
		request->params[QStringLiteral("result")] = QVariant(result);

		if (dummy_command) delete command;
		if (!ok) request->params[QStringLiteral("cancelled")] = QVariant(true);
	} else if (type == RBackendRequest::Debugger) {
		RKDebugHandler::instance()->debugCall(request, runningCommand());
		return; // request will be closed by the debug handler
	} else if ((type == RBackendRequest::ShowFiles) || (type == RBackendRequest::EditFiles)) {
		ShowEditTextFileAgent::showEditFiles(request);
		return; // we are not done, yet!
	} else if (type == RBackendRequest::SetParamsFromBackend) {
		na_real = request->params[QStringLiteral("na_real")].toDouble();
		na_int = request->params[QStringLiteral("na_int")].toInt();
	} else if (type == RBackendRequest::BackendExit) {
		if (!backend_dead) {
			bool report = false;
			backend_dead = true;
			if (!request->params.value(QStringLiteral("regular"), QVariant(false)).toBool()) { // irregular exit
				backend_error.message.append(request->params[QStringLiteral("message")].toString());
				RK_DEBUG(RBACKEND, DL_ERROR, "Backend exit: %s", qPrintable(backend_error.message));
				if (backend_started) {
					backend_error.message += i18n("\nThe R backend will be shut down immediately. This means, you can not use any more functions that rely on it. I.e. you can do hardly anything at all, not even save the workspace (but if you're lucky, R already did that). What you can do, however, is save any open command-files, the output, or copy data out of open data editors. Quit RKWard after that. Sorry!");
					backend_error.title = i18n("R engine has died");
					backend_error.id = QLatin1String("r_engine_has_died");
					report = true;
				} else if (all_current_commands.isEmpty()) {
					// the fake startup command may not technically have started running, if exit occurred early on
					RCommand *fake_startup_command = RCommandStack::currentCommand();
					if (fake_startup_command) handleCommandOut(fake_startup_command);
					// in any case, reporting the error will happen from there
				}
			}
			while (!all_current_commands.isEmpty()) {
				auto c = all_current_commands.takeLast();
				c->status |= RCommand::Failed;
				handleCommandOut(c);
			}
			Q_EMIT backendStatusChanged(Dead);
			if (report) reportFatalError(); // TODO: Reporting should probably be moved to RKWardMinaWindow, entirely
		}
	} else {
		RK_ASSERT(false);
	}

	RKRBackendProtocolFrontend::setRequestCompleted(request);
}
