/***************************************************************************
                          rcommand.h  -  description
                             -------------------
    begin                : Mon Nov 11 2002
    copyright            : (C) 2002, 2006, 2007, 2009, 2010 by Thomas Friedrichsmeier
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

#ifndef RCOMMAND_H
#define RCOMMAND_H

#include <QString>
#include <QObject>
#include <QList>

#include "rdata.h"

#define MAX_RECEIVERS_PER_RCOMMAND 3

class RCommandReceiver;
class RCommand;
class RCommandProxy;
class RCommandChain;

/** Base class for RCommand and RCommandChain, to make it possible to store both in the same list */
class RCommandBase {
public:
/** Returns the casted pointer, if this is a command, else 0 */
	RCommand* commandPointer ();
/** Returns the casted pointer, if this is a chain, else 0 */
	RCommandChain* chainPointer ();
protected:
friend class RCommandStack;
friend class RCommandStackModel;
	RCommandBase (bool is_chain);
	RCommandChain *parent;
private:
	bool is_command_chain;
};

/** this simple struct is used to ensure a sequence of RCommand s does not get interrupted by unrelated RCommands.
@see \ref UsingTheInterfaceToR
@see RInterface::startChain
@see RInterface::closeChain */
class RCommandChain : public RCommandBase {
public:
	bool isClosed () const { return closed; };
protected:
friend class RCommandStack;
friend class RCommandStackModel;
	RCommandChain () : RCommandBase (true) {};
	QList<RCommandBase*> commands;
	bool closed;
	bool isStack () { return (parent == 0); }
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
 * Currently, only a single signal is available: When the command has finished. Further signals may be added, in the future.
 *
 * @Note You can also use this in connection with RCommandReceiver-based classes, if interested in RCommandReceiver::cancelOutstandingCommands().
 */
class RCommandNotifier : public QObject {
	Q_OBJECT
signals:
	void commandFinished (RCommand *command);
private:
friend class RCommand;
	RCommandNotifier ();
	~RCommandNotifier ();
	void emitFinished (RCommand *command) { emit commandFinished (command); };
};

/** For introductory information on using RCommand, see \ref UsingTheInterfaceToR 

	This class is used to encapsulate an R-command, so it can be easily identified
	in a chain of commands. It is needed, since communication with R is asynchronous
	and it is therefore not possible to get the result of an R-call right away.
	Instead, create an object of this class, specifying the RCommandReceiver that should
	be called when the command has finished. You can then retrieve all information
	on the command (including the reply) from the object that is passed to your handler.
	
	There are several ways to identify a command when it's finished (needed, if a single RCommandReceiver needs to handle the results of
	several different commands):
		- storing the id () (each command is automatically assigned a unique id, TODO: do we need this functionality? Maybe remove it for redundancy)
		- passing appropriate flags to know how to handle the command
		- keeping the pointer (CAUTION: don't use that pointer except to compare it with the pointer of an incoming command. Commands get deleted when they are finished, and maybe (in the future) if they become obsolete etc. Hence the pointers you keep may be invalid!)
		(- checking the command-string)
	Note that an RCommand carries a whole lot of information around. However, RCommands generally don't get
	kept around very long, so they should not be a memory issue.
  *@author Thomas Friedrichsmeier
  */
class RCommand : public RData, public RCommandBase {
public:
/** constructs an RCommand.
@param command The command (string) to be run in the backend. This may include newlines and ";". The command should be a complete statement. If it is an incomplete statement, the backend will not wait for the rest of the command to come in, but rather the command will fail with RCommand::errorIncomplete.
@param type An integer being the result of a bit-wise OR combination of the values in RCommand::CommandTypes. The type-parameter is used to indicate the type of command, and also how the command should retrieve information (as a usual string, or as a data vector). See \ref RCommand::CommandTypes
@param rk_equiv Not yet used: a short descriptive string attached to the RCommand, that allows the user to make some sense of what this command is all about.
@param receiver The RCommandReceiver this command should be passed on to, when finished. @Note: consider connecting to the notifier(), instead!
@param flags A freely assignable integer, that you can use to identify what the command was all about. Only the RCommandReceiver handling the results will have to know what exactly the flags mean.
*/
	explicit RCommand (const QString &command, int type, const QString &rk_equiv = QString::null, RCommandReceiver *receiver=0, int flags=0);
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
		User=1,		/**< Command was created directly by the user (e.g. in the console or in a command editor window) */
		Plugin=1 << 1,		/**< Command comes from a plugin */
		App=1 << 2,			/**< Command comes from the application (e.g. loading / saving the workspace */
		Sync=1 << 3,		/**< Command is used to sync data to or from R-space. Typically used in the editor classes */
		EmptyCommand=1 << 4,		/**< Command is empty and will not be processed (an empty command may be used as a "marker") */
		Console=1 << 5,	/**< Command originated in the console. These commands will get some extra treatment in RKwatch */
		Internal=1 << 6,	/**< Command is meant to be used in the backend, only. Do not use outside rbackend classes! */
		Silent=1 << 7,	/**< Command can be interrupted, but is otherwise an internal command. In particular it should not be carbon copied to the output. */
		GetIntVector=1 << 8,			/**< Try to fetch result as an array of integers */
		GetStringVector=1 << 9,	/**< Try to fetch result as an array of chars */
		GetRealVector=1 << 10,		/**< Try to fetch result as an array of doubles */
		GetStructuredData=1 << 11,		/**< Try to fetch result as an RData structure */
		CCOutput=1 << 12,		/**< Append command output to the HTML-output file */
		CCCommand=1 << 13,		/**< Append the command itself to the HTML-output file */
		ObjectListUpdate=1 << 14,		/**< The command may change the list of objects available. Do an update */
		QuitCommand=1 << 15		/**< The R backend should be killed */
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
/** return the flags associated with the command. Those are the same that you specified in the constructor, RKWard does not touch them. @see RCommand::RCommand */
	int getFlags () const { return (_flags); };
/** Add an additional listener to the command */
	void addReceiver (RCommandReceiver *receiver);
/** Remove a receiver from the list. This may be needed when a listener wants to self-destruct, to make sure we don't try to send any further info there */
	void removeReceiver (RCommandReceiver *receiver);
	void addTypeFlag (int flag) { _type |= flag; };
	ROutputList &getOutput () { return output_list; };
/** modify the command string. DO NOT CALL THIS after the command has been submitted! */
	void setCommand (const QString &command) { _command = command; };

/** creates a proxy for this RCommand */
	RCommandProxy* makeProxy () const;
	void mergeAndDeleteProxy (RCommandProxy *proxy);

/** returns a notifier for this command (creating it, if needed). You can connect to the notifiers signals. */
	RCommandNotifier* notifier ();
/** same as RObject::rQuote */
	static QString rQuote (const QString &quoted);
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
	int _flags;
	int status;
	int has_been_run_up_to;
	QString _rk_equiv;
	int _id;
	static int next_id;
	RCommandReceiver *receivers[MAX_RECEIVERS_PER_RCOMMAND];

	RCommandNotifier *_notifier;
};

#endif
