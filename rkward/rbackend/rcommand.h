/*
rcommand.h - This file is part of the RKWard project. Created: Mon Nov 11 2002
SPDX-FileCopyrightText: 2002-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RCOMMAND_H
#define RCOMMAND_H

#include <QString>
#include <QObject>
#include <QList>

#include "rdata.h"

class RCommand;
class RCommandProxy;
class RObject;

/** R Commands can be arranged in a simple chain to make sure they are not interrupted by other commands.
 *  Also, command may need to run sub-commands.
@see \ref UsingTheInterfaceToR
@see RInterface::startChain
@see RInterface::closeChain */
class RCommandChain {
public:
	bool isClosed () const { return closed; };
/** @returns true, if there are no sub-commands or sub-chains waiting in this chain */
	bool isEmpty () const { return sub_commands.isEmpty (); };
	bool isCommand () const { return is_command; };
	RCommandChain* parentChain () const { return parent; };
	RCommand *toCommand ();
protected:
friend class RCommandStack;
friend class RCommandStackModel;
	RCommandChain (bool is_chain=true) : closed (!is_chain), is_command (!is_chain) {};
	QList<RCommandChain*> sub_commands;
	bool closed;
	bool is_command;
	RCommandChain *parent;
};

/** this struct is used to store the R output to an RCommand. The RCommand basically keeps a list of ROutputString (s). The difference to a normal
QString is, that additionally we store information on whether the output was "normal", "warning", or an "error". */
struct ROutput {
	enum ROutputType {
		NoOutput,		/**< No output. Rarely used. */
		Output,			/**< normal output */
		Warning,		/**< R warning */
		Error			/**< R error */
	};
	ROutputType type;
	QString output;
};

typedef QList<ROutput*> ROutputList;

/** Supplies signals for RCommands.
 * Obtain an instance of this using RCommand::notifier ();
 * Provides signals for new output, command completion (with or without error), and, as a special case for the R console, a signal, when a new line of a command has started runnning.
 */
class RCommandNotifier : public QObject {
	Q_OBJECT
Q_SIGNALS:
/** given command has finished (not necessarily successfully) */
	void commandFinished(RCommand *command);
/** new output for the given command */
	void commandOutput(RCommand *command, const ROutput* output);
/** a new line of the command has started being evaluate. Only emitted to RCommand::User-type commands */
	void commandLineIn(RCommand *command);
private:
friend class RCommand;
	RCommandNotifier();
	~RCommandNotifier();
	void emitFinished(RCommand *command) { Q_EMIT commandFinished(command); };
	void emitOutput(RCommand *command, const ROutput* output) { Q_EMIT commandOutput(command, output); };
	void emitLineIn(RCommand *command) { Q_EMIT commandLineIn(command); };
};

/** For introductory information on using RCommand, see \ref UsingTheInterfaceToR 

	This class is used to encapsulate an R-command, so it can be easily identified
	in a chain of commands. It is needed, since communication with R is asynchronous
	and it is therefore not possible to get the result of an R-call right away.

	To track the status of a command, and work with the results, connect to the notifier() signals.
	Usually, RCommand::whenFinished() is the most convenient way to connect a callback (typically a lambda function).

	I case several commands a connected to a single function, the command id() can be stored to identify, which command
	to deal with. Keeping a pointer is another options, but beware: don't use that pointer except to compare it with the pointer of an incoming command.
	Commands get deleted when they are finished, and maybe (in the future) if they become obsolete etc. Hence the pointers you keep may be invalid!

	Note that an RCommand carries a whole lot of information around. However, RCommands generally don't get
	kept around very long, so they should not be a memory issue.
  *@author Thomas Friedrichsmeier
  */
class RCommand : public RData, public RCommandChain {
public:
/** constructs an RCommand.
@param command The command (string) to be run in the backend. This may include newlines and ";". The command should be a complete statement. If it is an incomplete statement, the backend will not wait for the rest of the command to come in, but rather the command will fail with RCommand::errorIncomplete.
@param type An integer being the result of a bit-wise OR combination of the values in RCommand::CommandTypes. The type-parameter is used to indicate the type of command, and also how the command should retrieve information (as a usual string, or as a data vector). See \ref RCommand::CommandTypes
@param rk_equiv Not yet used: a short descriptive string attached to the RCommand, that allows the user to make some sense of what this command is all about.
*/
	explicit RCommand (const QString &command, int type, const QString &rk_equiv = QString ());
/** destructor. Note: you should not delete RCommands manually. This is done in RInterface. TODO: make protected */
	~RCommand();
/** @returns the type as specified in RCommand::RCommand */
	int type () const { return _type; };
/** @returns the raw command status. @see CommandStatus */
	int getStatus () const { return status; };
/** @returns the rk_equiv as specified in RCommand::RCommand */
	QString rkEquivalent () const { return _rk_equiv; };
/** @returns the command string (i.e. the input) as specified in RCommand::RCommand */
	QString command () const { return _command; };
/** @returns like command(), but for user commands, which have been run, partially, returns only the remaining portion of the command. */
	QString remainingCommand () const;
/** Each RCommand is assigned a unique integer id (incrementing from 0 to integer overflow) upon creation. This returns this id. 
	@returns the unique id of this command */
	int id () const { return _id; };
/* TODO: Adjust these two functions to allow re-getting of output and error-messages from logs */
/** @returns the full output of the command, i.e. all "regular" output, warning messages, and errors, in the order they were encountered. @see RCommand::output @see RCommand::error @see RCommand::warnings */
	QString fullOutput () const;
/** @returns the "regular" (ROutput::Output) output of the command, if any (e.g. "[1] 1" for "print (1)"). @see RCommand::succeeded @see RCommand::hasOutput */
	QString output () const;
/** @returns the warning message(s) given by R, if any. @see RCommand::output @see RCommand::error */
	QString warnings () const;
/** @returns the error message given by R, if any. @see RCommand::failed @see RCommand::hasError */
	QString error () const;
/** Types of commands (potentially more to come), bitwise or-able,
	although partially exclusive. See \ref UsingTheInterfaceToR for a overview of what these are used for. TODO: find out, why Canceled is in here, and document that fact. */
	enum CommandTypes {
		User=1,               /**< Command was created directly by the user (e.g. in the console or in a command editor window) */
		Plugin=1 << 1,        /**< Command comes from a plugin */
		App=1 << 2,           /**< Command comes from the application (e.g. loading / saving the workspace */
		Sync=1 << 3,          /**< Command is used to sync data to or from R-space. Typically used in the editor classes */
		EmptyCommand=1 << 4,  /**< Command is empty and will not be processed (an empty command may be used as a "marker") */
		Console=1 << 5,       /**< Command originated in the console. These commands will get some extra treatment in RKwatch */
		Internal=1 << 6,      /**< Command is meant to be used in the backend, only. Do not use outside rbackend classes! */
		Silent=1 << 7,        /**< Command can be interrupted, but is otherwise an internal command. In particular it should not be carbon copied to the output. */
		GetIntVector=1 << 8,  /**< Try to fetch result as an array of integers */
		GetStringVector=1 << 9, /**< Try to fetch result as an array of chars */
		GetRealVector=1 << 10, /**< Try to fetch result as an array of doubles */
		GetStructuredData=1 << 11, /**< Try to fetch result as an RData structure */
		CCOutput=1 << 12,     /**< Append command output to the HTML-output file */
		CCCommand=1 << 13,    /**< Append the command itself to the HTML-output file */
		ObjectListUpdate=1 << 14, /**< The command may change the list of objects available. Do an update */
		QuitCommand=1 << 15,  /**< The R backend should be killed */
		PriorityCommand=1 << 16 /**< The command has high priority, should be run during R's event loop processing. In general, PriorityCommands *must* not have side-effects
		                             Use only, when absolutely necessary. */
	};
	enum CommandStatus {
		Running=1,						/**< command is currently running */
		WasTried=2,						/**< the command has been passed to the backend. */
		Failed=4,							/**< the command failed */
		HasOutput=8,					/**< command has a string output retrievable via RCommand::output () */
		HasError=16,						/**< command has an error-message retrievable via RCommand::error () */
		HasWarnings=32,			/**< command has warning-message(s) retrievable via RCommand::warnings () */
		ErrorIncomplete=512,		/**< backend rejected command as being incomplete */
		ErrorSyntax=1024,			/**< backend rejected command as having a syntax error */
		ErrorOther=2048,			/**< another error (not incomplete, not syntax error) has occurred while trying to execute the command */
		Canceled=8192				/**< Command was cancelled. */
	};
/** the command has been passed to the backend. */
	bool wasTried () const { return (status & WasTried); };
/** the command failed */
	bool failed () const { return (status & Failed); };
/** the command was cancelled before it was executed */
	bool wasCanceled () const { return (wasTried () && failed () && (status & Canceled)); }
/** the command succeeded (wasTried () && (!failed ()) */
	bool succeeded () const { return ((status & WasTried) && !(status & Failed)); };
/** command has a string output retrievable via RCommand::output () */
	bool hasOutput () const { return (status & HasOutput); };
/** command has a string output retrievable via RCommand::warnings () */
	bool hasWarnings () const { return (status & HasWarnings); };
/** command has an error-message retrievable via RCommand::error () */
	bool hasError () const { return (status & HasError); };
/** backend rejected command as being incomplete */
	bool errorIncomplete () const { return (status & ErrorIncomplete); };
/** backend rejected command as having a syntax error */
	bool errorSyntax () const { return (status & ErrorSyntax); };
	void addTypeFlag (int flag) { _type |= flag; };
	ROutputList &getOutput () { return output_list; };
/** modify the command string. DO NOT CALL THIS after the command has been submitted! */
	void setCommand (const QString &command) { _command = command; };
	void setUpdatesObject(const RObject *object);

/** creates a proxy for this RCommand */
	RCommandProxy* makeProxy () const;
	void mergeAndDeleteProxy (RCommandProxy *proxy);

/** returns a notifier for this command (creating it, if needed). You can connect to the notifiers signals. */
	RCommandNotifier* notifier ();
/** same as RObject::rQuote */
	static QString rQuote (const QString &quoted);

	template<typename T> void whenFinished(const QObject* receiver, const T func) {
		QObject::connect(notifier(), &RCommandNotifier::commandFinished, receiver, func);
	}
private:
friend class RInterface;
friend class RCommandStack;
friend class RCommandStackModel;
/** internal function will be called by the backend, as the command gets passed through. Takes care of sending this command (back) to its receiver(s) */
	void finished ();
/** new output was generated. Pass on to receiver(s) */
	void newOutput (ROutput *output);
/** next line of command has been transmitted. Pass on to receiver(s). Only called for RCommand::User type commands */
	void commandLineIn ();
	ROutputList output_list;
	QString _command;
	int _type;
	int status;
	int has_been_run_up_to;
	QString _rk_equiv;
	QString _updated_object;
	int _id;
	static int next_id;

	RCommandNotifier *_notifier;
};

#include <QVariant>
/** Standard wrapper for the result of a "generic" API call, such that we need less special-casing in the backend. The conventions are simple:
*  - If @param error is non-null, stop() will be called in the backend, with the given message.
*  - If @param warning is non-null, the message will be shown (as a warning), but no error will be raised.
*  - Unless an error was thrown, @param ret will be returned as a basic data type (possibly NULL). */
struct GenericRRequestResult {
	GenericRRequestResult(const QVariant& ret=QVariant(), const QString& warning=QString(), const QString& error=QString()) :
		error(error), warning(warning), ret(ret) {};
	static GenericRRequestResult makeError(const QString& error) {
		return GenericRRequestResult(QVariant(), QString(), error);
	}
	GenericRRequestResult& addMessages(const GenericRRequestResult &other) {
		if (!other.error.isEmpty()) {
			if (!error.isEmpty()) error.append(QLatin1Char('\n'));
			error.append(other.error);
		}
		if (!other.warning.isEmpty()) {
			if (!warning.isEmpty()) warning.append(QLatin1Char('\n'));
			warning.append(other.warning);
		}
		return *this;
	}
	QString error;
	QString warning;
	QVariant ret;
	bool failed() { return !error.isEmpty(); }
	bool toBool() { return ret.toBool(); }
};

#endif
