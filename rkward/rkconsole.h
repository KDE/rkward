/***************************************************************************
                          rkconsole  -  description
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

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include "rbackend/rcommandreceiver.h"
#include "windows/rkcommandeditorwindow.h"
#include "windows/rkmdiwindow.h"

class QEvent;
class QKeyEvent;
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

class RKConsole : public RKMDIWindow, public RCommandReceiver, public RKScriptContextProvider {
Q_OBJECT
public:
/** Constructor. */
	RKConsole (QWidget *parent, bool tool_window, const char *name=0);
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
/** interrupt the current incomplete command (if any) */
	void resetIncompleteCommand ();
	void doTabCompletion ();
	bool provideContext (unsigned int line_rev, QString *context, int *cursor_position);

	static RKConsole *mainConsole () { return main_console; };
	static void setMainConsole (RKConsole *console) { main_console = console; };

	bool isBusy () { return (current_command || command_incomplete); };
/** Run a user command (through console, if applicable */
	static void pipeUserCommand (RCommand *command);
/** Overload for the above function: Use this, if you just need to run a string with no specials */
	static void pipeUserCommand (const QString &command);
protected:
/** Handle keystrokes before they reach the kate-part. Return TRUE if we want the kate-part to ignore it
\param e the QKeyEvent */
	bool handleKeyPress (QKeyEvent * e);
	void rCommandDone (RCommand *command);
/** reimplemented from RCommandReceiver::newOutput () to handle output of console commands */
	void newOutput (RCommand *command, ROutput *output);
/** reimplemented from QWidget to show the context menu */
	void contextMenuEvent (QContextMenuEvent * event);
private:
friend class RKConsolePart;
	bool eventFilter (QObject *o, QEvent *e);
	bool doTabCompletionHelper (int line_num, const QString &line, int word_start, int word_end, const QStringList &entries);
/** a helper function to doTabCompletionHelper */
	void insertCompletion (int line_num, int word_start, int word_end, const QString &completion);
	QString incomplete_command;
	bool command_incomplete;
/** A list to store previous commands */
	QStringList commands_history;
/** current position in the commands history */
	QStringList::const_iterator commands_history_position;
/** A flag to indicate whether the command was edited while scrolling in the history */ 
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
	KTextEditor::Document *doc;
	KTextEditor::View *view;
	RKFunctionArgHinter *hinter;

	static RKConsole *main_console;

	bool tab_key_pressed_before;

	QAction* context_help_action;
	QAction* run_selection_action;
	QAction* interrupt_command_action;
	QAction* copy_action;
	QAction* copy_literal_action;
	QAction* paste_action;

	KActionCollection *kate_edit_actions;
	void triggerEditAction (QString name);
	void setCursorClear (int line, int col);

	void initializeActions (KActionCollection *ac);
	void pipeCommandThroughConsoleLocal (RCommand *command);
	bool command_was_piped;

	RKConsolePart *console_part;
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
