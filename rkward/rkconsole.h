/***************************************************************************
                          rkconsole  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006, 2007, 2010, 2011 by Thomas Friedrichsmeier
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

/** Returns the command currently being edited (not executed yet) */
	QString currentEditingLine () const;
/** Returns the current cursor position, within the current command (without taking into account the prefix). Returns -1 if the cursor is not on the line containing the command. */
	int currentCursorPositionInCommand ();
	void doTabCompletion ();
	QString provideContext (int line_rev);

	static RKConsole *mainConsole () { return main_console; };
	static void setMainConsole (RKConsole *console) { main_console = console; };

	bool isBusy () const;
/** Run a command through the console (unless user has configured such commands to be run outside the console: then it will run in separately). */
	static void pipeUserCommand (const QString &command);

/** reimplemnented from RKMDIWindow to clear selection when gaining focus */
	void activate (bool with_focus=true);
	void setCommandHistory (const QStringList &new_history, bool append);
	QStringList commandHistory () const { return commands_history; };
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
	void setCurrentEditingLine (const QString &line);
/** Try to submit the next chunk of the input buffer. */
	void tryNextInBuffer ();
	void showPrompt ();
/** Add given command to command history. Also checks, wether the history is longer than max length, and chops it if so. */
	void addCommandToHistory (const QString &command);

	QString prefix;
/** This string stores the regular prefix printed at the beginning of each line. */
	QString nprefix;
/** This string stores the continuation prefix. */
	QString iprefix;

/** Create a proxy for the katepart action of the same name. The action is added to the actioncollection, automatically. Also any icon and label (but not shorcut) is copied.
@param actionName Identifier of the action in katepartui.rc and rkconsolepart.rc
@param label Label for the proxy action. If empty (default) copy the label from the katepartui
@returns a pointer to the proxy action */
	QAction* addProxyAction (const QString& actionName, const QString& label=QString ());

	QString cleanSelection (const QString &origin);

	RCommand *current_command;
	KTextEditor::Document *doc;
	KTextEditor::View *view;
	RKFunctionArgHinter *hinter;

	static RKConsole *main_console;

	bool tab_key_pressed_before;

	KAction* copy_lines_to_output_action;
	KAction* context_help_action;
	KAction* run_selection_action;
	KAction* interrupt_command_action;
	KAction* copy_commands_action;
	KAction* copy_literal_action;
	KAction* paste_action;

	KActionCollection *kate_edit_actions;
	void triggerEditAction (QString name);
	void setCursorClear (int line, int col);

	void initializeActions (KActionCollection *ac);
	void pipeCommandThroughConsoleLocal (const QString &command);

	RKConsolePart *console_part;
public slots:
/** We intercept paste commands and get them executed through submitBatch.
@sa submitBatch */
	void paste ();
	void copyCommands ();
	void literalCopy ();
/** Clear the view, and add a prompt at the top. */
	void clear ();
/** show context help on the current word */
	void showContextHelp ();
/** Cancels the current command, if any, and clears the command buffer(s) */
	void resetConsole ();
	void runSelection ();
	void copyLinesToOutput ();

/** Adds a chunk of commands to the input buffer
\param batch a QString containing the batch of commands to be executed */
	void submitBatch (const QString &batch);

	void userLoadHistory (const KUrl &url=KUrl ());
	void userSaveHistory (const KUrl &url=KUrl ());
private:
/** Commands can be queued in the console in four different places:
1) The not-yet-executed remainder of a previous incomplete command.
2) A command which has already been issued. Note that this command, too, be incomplete, in which case it's remainder will be stored to 1.
3) One or more lines of commands which have been pasted or piped to the Console, but have not yet been submitted.
4) One line of a command without a trailing newline. This may be part of a previously pasted command, but most typically it is the line the user is currently editing.

1 and 2 are mutually exclusive, and stored in incomplete_command, or current_command->command ().
3 is stored in input_buffer. If there is no current command, the input buffer will be emptied as soon as tryNextInBatch() is called.
4 and 2 are mutually exclusive. 4 can be retrieved as currentEditingLine () */
	QString input_buffer;
	int current_command_displayed_up_to;
	int skip_command_display_lines;
	bool previous_chunk_was_piped;
	
/** Reimplemented from RCommandReceiver to display the next line of the command */
	void userCommandLineIn (RCommand* command);
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
