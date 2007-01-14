/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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
#ifndef RKCONSOLE_H
#define RKCONSOLE_H


#include <kparts/part.h>

#include <kate/document.h>
#include <kate/view.h>

#include <qptrlist.h>

#include "rbackend/rcommandreceiver.h"
#include "windows/rkcommandeditorwindow.h"

class QStringList;
class KAction;
class RCommand;
class KActionCollection;
class RKConsolePart;

/**
\brief Provides an R-like console.

This class provides a console, which is very similar to the classic R console. It is based on a heavily modified KatePart.

Do not construct directly. Construct an RKConsolePart instead.

\sa RKCommandLog

@author Pierre Ecochard
**/

class RKConsole : public QWidget, public RCommandReceiver, public RKScriptContextProvider {
Q_OBJECT
public:
/** Constructor. */
	RKConsole ();
/** Destructor */
	~RKConsole ();

/** Submits a batch of commands, line by line.
\param batch a QString containing the batch of commands to be executed */
	void submitBatch (const QString &batch);
/** Returns the command currently being edited (not executed yet) */
	QString currentCommand ();
/** Returns the current cursor position. Returns the column on which is the cursor.  */
	int currentCursorPosition ();
/** Returns the current cursor position, within the current command (without taking into account the prefix).*/
	int currentCursorPositionInCommand ();
/** Returns TRUE if some text is selected; otherwise returns FALSE.  */
	bool hasSelectedText ();
/** interrupt the current incomplete command (if any) */
	void resetIncompleteCommand ();
	void doTabCompletion ();
	bool provideContext (unsigned int line_rev, QString *context, int *cursor_position);

	static RKConsole *mainConsole () { return main_console; };
	static void setMainConsole (RKConsole *console) { main_console = console; };

	bool isBusy () { return (current_command || command_incomplete); };
/** Run a user command (through console, if applicable */
	static void pipeUserCommand (RCommand *command);

	RKConsolePart *getPart () { return part; };
signals:
	void raiseWindow ();
protected:
/** Handle keystrokes before they reach the kate-part. Return TRUE if we want the kate-part to ignore it
\param e the QKeyEvent */
	bool handleKeyPress (QKeyEvent * e);
	void rCommandDone (RCommand *command);
/** reimplemented from RCommandReceiver::newOutput () to handle output of console commands */
	void newOutput (RCommand *command, ROutput *output);
private:
friend class RKConsolePart;
	bool eventFilter (QObject *o, QEvent *e);
/** set syntax-highlighting for R */
	void setRHighlighting ();
	QString incomplete_command;
	bool command_incomplete;
/** A list to store previous commands */
	QStringList commands_history;
/** current position in the commands history */
	QStringList::const_iterator commands_history_position;
/** A flag to indicate wheter the command was edited while scrolling in the history */ 
	bool command_edited;
/** The last line in the history is special, in that it is stored before it is submitted, but not permanently so */
	QString history_editing_line;
/** The context to look out for, if doing a context search in the command history */
	QString command_history_context;
/** A list to store a commands batch that will be executed one line at a time */
	QStringList commands_batch;
/** Sets the cursor position to the end of the last line. */
	void cursorAtTheEnd ();
/** Submits the current command */
	void submitCommand ();
/** Set the current command to the previous command in the command list
@param context_sensitive if set to true, history lines that do not start with command_history_context are ignored (leading to context sensitive navigation of the command history) */
	void commandsListUp (bool context_sensitive=false);
/** Set the current command to the next command in the command list
@param context_sensitive if set to true, history lines that do not start with command_history_context are ignored (leading to context sensitive navigation of the command history) */
	void commandsListDown (bool context_sensitive=false);
/** Sets the cursor position to the beginning of the last line. */
	void cursorAtTheBeginning ();
/** Sets the current command. This is used from commandsListUp (), and commandsListDown ();
\param command the new command */
	void setCurrentCommand (const QString &command);
/** Add a new line, and try to submit the next item in a batch of (pasted) commands. If there is no batch, only add the new line. */
	void tryNextInBatch (bool add_new_line = true);
/** Add given command to command history. Also checks, wether the history is longer than max length, and chops it if so. */
	void addCommandToHistory (const QString &command);

	QString prefix;
/** This string stores the regular prefix printed at the beginning of each line. */
	const char *nprefix;
/** This string stores the continuation prefix. */
	const char *iprefix;
/** This function unplugs a KAction
\param action the KAction to be unplugged
\param ac the action collection from which to retrieve the KAction*/
	void unplugAction (const QString &action, KActionCollection* ac);

	QString cleanedSelection ();

	bool output_continuation;

	RCommand *current_command;
	Kate::Document *doc;
	Kate::View *view;
	RKFunctionArgHinter *hinter;

	static RKConsole *main_console;

	bool tab_key_pressed_before;

	KAction* context_help_action;
	KAction* run_selection_action;
	KAction* interrupt_command_action;
	KAction* copy_action;
	KAction* copy_literal_action;
	KAction* paste_action;

	void initializeActions (KActionCollection *ac);
	void pipeCommandThroughConsoleLocal (RCommand *command);

	RKConsolePart *part;

	void doPopupMenu (const QPoint &pos);
public slots:
/** We intercept paste commands and get them executed through submitBatch.
@sa submitBatch */
	void paste ();
	void copy ();
	void literalCopy ();
/** Clear the view, and add a prompt at the top. */
	void clear ();
	void configure ();
/** show context help on the current word */
	void showContextHelp ();
/** interrupt current command. */
	void slotInterruptCommand ();
	void runSelection ();
};

/** A part interface to RKConsole. Provides the context-help functionality

@author Thomas Friedrichsmeier */

class RKConsolePart : public KParts::Part {
friend class RKConsole;
protected:
/** constructor. Protected. Meant to the created by the RKConsole itself
@param console The console for this part */
	RKConsolePart (RKConsole *console);
/** destructor */
	~RKConsolePart ();

	void showPopupMenu (const QPoint &pos);
};

#endif
