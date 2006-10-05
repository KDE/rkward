/***************************************************************************
                          rcommand.h  -  description
                             -------------------
    begin                : Mon Nov 11 2002
    copyright            : (C) 2002, 2006 by Thomas Friedrichsmeier
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

#include <qfile.h>
#include <qstring.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qvaluelist.h>

#include "rdata.h"

class RCommandReceiver;
class RCommand;
class RChainOrCommand;

/** this simple struct is used to ensure a sequence of RCommand s does not get interrupted by unrelated RCommands.
@see \ref UsingTheInterfaceToR
@see RInterface::startChain
@see RInterface::closeChain */
class RCommandChain {
protected:
friend class RControlWindow;
friend class RControlWindowListViewItem;
friend class RCommandStack;
	QPtrList<RChainOrCommand> commands;
	bool closed;
	RCommandChain *parent;
};

/** this struct is needed by the RCommandStack. It is only a wrapper, which stores a pointer to _either_ a command _or_ a chain. Its sole purpose is to
be able to insert either a command or a chain using the same mechanism, easily. You don't want to use this class outside of RCommandStack (TODO: move it to rcommandstack.h, then!) */
class RChainOrCommand {
private:
friend class RControlWindow;
friend class RCommandStack;
	RCommand *command;
	RCommandChain *chain;
};

/** this struct is used to pass on eval-requests (i.e. request for RKWard to do something, which may involve issuing further commands) from the
backend-thread to the main thread. Do not use outside the backend-classes. */
struct REvalRequest {
private:
friend class RInterface;
friend class RThread;
	QString *call;
	int call_length;
	RCommandChain *in_chain;
};

/** this struct is used to store the R output to an RCommand. The RCommand basically keeps a list of ROutputString (s). The difference to a normal
QString is, that additionally we store information on whether the output was "normal", "warning", or an "error". */
struct ROutput {
	enum ROutputType {
		Output,			/**< normal output */
		Warning,		/**< R warning */
		Error				/**< R error */
	};
	ROutputType type;
	QString output;
};

/*
struct RGetValueRequest {
private:
friend class RInterface;
friend class RThread;
	char **call;
	int call_length;
};

struct RGetValueReply {
private:
friend class RInterface;
friend class RThread;
	char **reply;
	int reply_length;
};
*/

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
  
class RCommand : public RData {
public:
/** constructs an RCommand.
@param command The command (string) to be run in the backend. This may include newlines and ";". The command should be a complete statement. If it is an incomplete statement, the backend will not wait for the rest of the command to come in, but rather the command will fail with RCommand::errorIncomplete.
@param type An integer being the result of a bit-wise OR combination of the values in RCommand::CommandTypes. The type-parameter is used to indicate the type of command, and also how the command should retrieve information (as a usual string, or as a data vector). See \ref RCommand::CommandTypes
@param rk_equiv Not yet used: a short descriptive string attached to the RCommand, that allows the user to make some sense of what this command is all about.
@param receiver The RCommandReceiver this command should be passed on to, when finished.
@param flags A freely assignable integer, that you can use to identify what the command was all about. Only the RCommandReceiver handling the results will have to know what exactly the flags mean.
*/
	RCommand (const QString &command, int type = 0, const QString &rk_equiv = QString::null, RCommandReceiver *receiver=0, int flags=0);
/** destructor. Note: you should not delete RCommands manually. This is done in RInterface. TODO: make protected */
	~RCommand();
/** @returns the type as specified in RCommand::RCommand */
	int type () { return _type; };
/** @returns the raw command status. @see CommandStatus */
	int getStatus () { return status; };
/** @returns the rk_equiv as specified in RCommand::RCommand */
	QString rkEquivalent () { return _rk_equiv; };
/** @returns the command string (i.e. the input) as specified in RCommand::RCommand */
	QString command () { return _command; };
/** Each RCommand is assigned a unique integer id (incrementing from 0 to integer overflow) upon creation. This returns this id. 
	@returns the unique id of this command */
	int id () { return _id; };
/* TODO: Adjust these two functions to allow re-getting of output and error-messages from logs */
/** @returns the full output of the command, i.e. all "regular" output, warning messages, and errors, in the order they were encountered. @see RCommand::output @see RCommand::error @see RCommand::warnings */
	QString fullOutput ();
/** @returns the "regular" (ROutput::Output) output of the command, if any (e.g. "[1] 1" for "print (1)"). @see RCommand::succeeded @see RCommand::hasOutput */
	QString output ();
/** @returns the warning message(s) given by R, if any. @see RCommand::output @see RCommand::error */
	QString warnings ();
/** @returns the error message given by R, if any. @see RCommand::failed @see RCommand::hasError */
	QString error ();
/** Types of commands (potentially more to come), bitwise or-able,
	although partially exclusive. See \ref UsingTheInterfaceToR for a overview of what these are used for. TODO: find out, why Canceled is in here, and document that fact. */
	enum CommandTypes {
		User=1,		/**< Command was created directly by the user (e.g. in the console or in a command editor window) */
		Plugin=2,		/**< Command comes from a plugin */
		PluginCom=4,	/**< Command comes from a plugin, and is used by the plugin to communicate directly with R (no real use so far) */
		App=8,			/**< Command comes from the application (e.g. loading / saving the workspace */
		Sync=16,		/**< Command is used to sync data to or from R-space. Typically used in the editor classes */
		EmptyCommand=32,		/**< Command is empty and will not be processed (an empty command may be used as a "marker") */
		Console=64,	/**< Command originated in the console. These commands will get some extra treatment in RKwatch */
		GetIntVector=512,			/**< Try to fetch result as an array of integers */
		GetStringVector=1024,	/**< Try to fetch result as an array of chars */
		GetRealVector=2048,		/**< Try to fetch result as an array of doubles */
		GetStructuredData=4096,		/**< Try to fetch result as an RData structure */
		DirectToOutput=8192,		/**< Append command output to the HTML-output file */
		ObjectListUpdate=16384		/**< The command may change the list of objects available. Do an update */
	};
	enum CommandStatus {
		WasTried=1,						/**< the command has been passed to the backend. */
		Failed=2,							/**< the command failed */
		HasOutput=4,					/**< command has a string output retrievable via RCommand::output () */
		HasError=8,						/**< command has an error-message retrievable via RCommand::error () */
		HasWarnings=16,			/**< command has warning-message(s) retrievable via RCommand::warnings () */
		ErrorIncomplete=512,		/**< backend rejected command as being incomplete */
		ErrorSyntax=1024,			/**< backend rejected command as having a syntax error */
		ErrorOther=2048,			/**< another error (not incomplete, not syntax error) has occured while trying to execute the command */
		Canceled=8192				/**< Command was cancelled. */
	};
/** the command has been passed to the backend. */
	bool wasTried () { return (status & WasTried); };
/** the command failed */
	bool failed () { return (status & Failed); };
/** the command succeeded (wasTried () && (!failed ()) */
	bool succeeded () { return ((status & WasTried) && !(status & Failed)); };
/** command has a string output retrievable via RCommand::output () */
	bool hasOutput () { return (status & HasOutput); };
/** command has an error-message retrievable via RCommand::error () */
	bool hasError () { return (status & HasError); };
/** backend rejected command as being incomplete */
	bool errorIncomplete () { return (status & ErrorIncomplete); };
/** backend rejected command as having a syntax error */
	bool errorSyntax () { return (status & ErrorSyntax); };
/** return the flags associated with the command. Those are the same that you specified in the constructor, RKWard does not touch them. @see RCommand::RCommand */
	int getFlags () { return (_flags); };
/** Add an additional listener to the command */
	void addReceiver (RCommandReceiver *receiver);
/** Remove a receiver from the list. This may be needed when a listener wants to self-destruct, to make sure we don't try to send any further info there */
	void removeReceiver (RCommandReceiver *receiver);
private:
friend class RThread;
friend class RInterface;
/** internal function will be called by the backend, as the command gets passed through. Takes care of sending this command (back) to its receiver(s) */
	void finished ();
/** new output was generated. Pass on to receiver(s) */
	void newOutput (ROutput *output);
	QValueList<ROutput*> output_list;
	QString _command;
	int _type;
	int _flags;
	int status;
	QString _rk_equiv;
	int _id;
	static int next_id;
	int num_receivers;
	RCommandReceiver **receivers;
};

#endif
