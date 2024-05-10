/*
rkconsole - This file is part of the RKWard project. Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKCONSOLE_H
#define RKCONSOLE_H


#include <kparts/part.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include "rbackend/rcommand.h"
#include "windows/rkcommandeditorwindow.h"
#include "windows/rkmdiwindow.h"
#include "misc/rkcommandhistory.h"

class QEvent;
class QKeyEvent;
class QAction;
class KActionCollection;
class RKConsolePart;

/**
\brief Provides an R-like console.

This class provides a console, which is very similar to the classic R console. It is based on a heavily modified KatePart.

Do not construct directly. Construct an RKConsolePart instead.

\sa RKCommandLog

@author Pierre Ecochard
**/

class RKConsole : public RKMDIWindow, public RKScriptContextProvider {
Q_OBJECT
public:
/** Constructor. */
	RKConsole (QWidget *parent, bool tool_window, const char *name=nullptr);
/** Destructor */
	~RKConsole ();

/** Returns the command currently being edited (not executed yet) */
	QString currentEditingLine () const;
/** Returns the current cursor position, within the current command (without taking into account the prefix). Returns -1 if the cursor is not on the line containing the command. */
	int currentCursorPositionInCommand ();
	QString provideContext (int line_rev) override;
	void currentHelpContext (QString *symbol, QString *package) override;

	static RKConsole *mainConsole () { return main_console; };
	static void setMainConsole (RKConsole *console) { main_console = console; };

	bool isBusy () const;
/** Run a command through the console (unless user has configured such commands to be run outside the console: then it will run in separately). */
	static void pipeUserCommand (const QString &command);

/** reimplemnented from RKMDIWindow to clear selection when gaining focus */
	void activate (bool with_focus=true) override;
	void setCommandHistory (const QStringList &new_history, bool append);
	QStringList commandHistory () const { return commands_history.getHistory (); };
	void addCommandToHistory (const QString& text) { commands_history.append (text); };
	void insertSpontaneousROutput(ROutput *output);
protected:
/** Handle keystrokes before they reach the kate-part. Return TRUE if we want the kate-part to ignore it
\param e the QKeyEvent */
	bool handleKeyPress (QKeyEvent * e);
/** handle output of console commands */
	void newOutput(RCommand *command, const ROutput *output);
/** display the next line of the command for multi-line commands */
	void userCommandLineIn(RCommand* command);
/** reimplemented from QWidget to show the context menu */
	void contextMenuEvent (QContextMenuEvent * event) override;
private:
friend class RKConsolePart;
	bool eventFilter (QObject *o, QEvent *e) override;
	QString incomplete_command;
/** A list to store previous commands */
	RKCommandHistory commands_history;
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

	QString prefix;
/** This string stores the regular prefix printed at the beginning of each line. */
	QString nprefix;
/** This string stores the continuation prefix. */
	QString iprefix;

/** Create a proxy for the katepart action of the same name. The action is added to the actioncollection, automatically. Also any icon and label (but not shortcut) is copied.
@param actionName Identifier of the action in katepartui.rc and rkconsolepart.rc
@param label Label for the proxy action. If empty (default) copy the label from the katepartui
@returns a pointer to the proxy action */
	QAction* addProxyAction (const QString& actionName, const QString& label=QString ());

	QString cleanSelection (const QString &origin);

	RCommand *current_command;
	KTextEditor::Document *doc;
	KTextEditor::View *view;

	static RKConsole *main_console;

	bool tab_key_pressed_before;

	QAction* run_selection_action;
	QAction* interrupt_command_action;
	QAction* copy_commands_action;
	QAction* copy_literal_action;
	QAction* paste_action;

	KActionCollection *kate_edit_actions;
	void triggerEditAction (const QString &name);
	void setCursorClear (int line, int col);

	void initializeActions (KActionCollection *ac);
	void pipeCommandThroughConsoleLocal (const QString &command);

	RKConsolePart *console_part;
public Q_SLOTS:
/** We intercept paste commands and get them executed through submitBatch.
@sa submitBatch */
	void paste ();
	void copyCommands ();
	void literalCopy ();
/** Clear the view, and add a prompt at the top. */
	void clear ();
/** Cancels the current command, if any, and clears the command buffer(s) */
	void resetConsole ();
	void runSelection ();
	void copyLinesToOutput ();

/** Adds a chunk of commands to the input buffer
\param batch a QString containing the batch of commands to be executed */
	void submitBatch (const QString &batch);

	void userLoadHistory (const QUrl &url=QUrl ());
	void userSaveHistory (const QUrl &url=QUrl ());
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
	KTextEditor::Cursor output_cursor;
	void rawWriteLine(const QString &line, QChar line_end);
};

/** A part interface to RKConsole. Provides the context-help functionality

@author Thomas Friedrichsmeier */

class RKConsolePart : public KParts::Part {
friend class RKConsole;
protected:
/** constructor. Protected. Meant to the created by the RKConsole itself
@param console The console for this part */
	explicit RKConsolePart(RKConsole *console);
/** destructor */
	~RKConsolePart ();

	void showPopupMenu (const QPoint &pos);
};

#endif
