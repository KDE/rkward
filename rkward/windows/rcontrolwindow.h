/***************************************************************************
                          rcontrolwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
    copyright            : (C) 2005, 2007 by Thomas Friedrichsmeier
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

#ifndef RCONTROLWINDOW_H
#define RCONTROLWINDOW_H

#include <kparts/part.h>

#include <qmap.h>
#include <qlabel.h>
#include <qlistview.h>

#include "rkmdiwindow.h"

class QPushButton;
class RCommand;
class RCommandChain;
class RChainOrCommand;
class RControlWindowListViewItem;
class RControlWindowPart;

/**
	\brief Interface to control R command execution

This class provides a GUI interface to inspect, and manipulate the current RCommandStack, and to Pause/Resume the R engine.
Do create an instance of this class directly. Create a RControlWindowPart instead. Also, probably RInterface should be the only class ever calling
functions of RControlWindow.

// TODO: probably the QListView of RCommands (and associated functionality) should be separted to an own class
to allow reuse

// TODO: check, if everything is implemented

@author Thomas Friedrichsmeier
*/
class RControlWindow : public RKMDIWindow {
	Q_OBJECT
public:
/** constructor.
@param parent parent QWidget, usually RKGlobals::rkApp () or similar */
	RControlWindow (QWidget *parent, bool tool_window, const char *name=0);
/** destructor */
	~RControlWindow ();
/** Add new chain to the RControlWindow. Has no effect unless RControlWindow::isShown () */
	void addChain (RCommandChain *chain);
/** Add new command to the RControlWindow. The command is added to the given parent chain. Has no effect unless RControlWindow::isShown () */
	void addCommand (RCommand *command, RCommandChain *parent);
/** Update information on the given chain. Use esp, if the chain was set to closed. Has no effect unless RControlWindow::isShown () */
	void updateChain (RCommandChain *chain);
/** Update information on the given command. Use esp, if the command was cancelled. Has no effect unless RControlWindow::isShown () */
	void updateCommand (RCommand *command);
/** Remove given command from the RControlWindow. This will also check, whether the parent chain can be torn down, automatically. Has no effect unless RControlWindow::isShown ()*/
	void removeCommand (RCommand *command);
/** Set given command as running. Has no effect unless RControlWindow::isShown ()*/
	void setCommandRunning (RCommand *command);

/** reimplemented to refresh list of commands when showing. This is needed, as the RControlWindow is only kept up to date as long as it is shown. Hence, if it was hidden, and then gets shown, it will have to update the entire list. */
	void show ();
/** Call this once, when the RInterface is ready, and it is ok to try showing commands */
	void initialize ();
/** Static reference to the control window */
	static RControlWindow* getControl () { return control_window; };
public slots:
/** command selection was changed. Automatically select sub-items of selected chains. Enable/disable "Cancel" button */
	void commandSelectionChanged ();
/** cancel button was clicked. Cancel selected commands (unless they are RCommand::Sync). */
	void cancelButtonClicked ();
/** pause button was clicked. Pause/Resume processing of the stack */
	void pauseButtonClicked ();
/** configure button was clicked. Invoke settings dialog */
	void configureButtonClicked ();
private:
	QListView *commands_view;
	QPushButton *cancel_button;
	QPushButton *pause_button;

	RControlWindowListViewItem *itemForCommand (RCommand *command);
	RControlWindowListViewItem *itemForChain (RCommandChain *chain);

/** internal: recursively add commands/chains */
	void addCommands (RChainOrCommand *coc, RControlWindowListViewItem *parent);
/** internal: add single command */
	void addCommand (RCommand *command, RControlWindowListViewItem *parent);
/** internal: delete chain(s) if applicable. This basically mimics the behavior in RCommandStack::pop () */
	void checkCleanChain (RControlWindowListViewItem *chain);

/** internal: causes the RControlWindow (if shown) to refresh it's entire list of commands. Warning! Does not lock the mutex. Lock the mutex before calling this! */
	void refreshCommands ();

	QMap <RCommand *, RControlWindowListViewItem *> command_map;
	QMap <RCommandChain *, RControlWindowListViewItem *> chain_map;

	bool paused;
	bool isActive ();
	bool initialized;

	void lockMutex ();
	void unlockMutex ();
	int mutex_lockcount;

friend class RKWardMainWindow;
	static RControlWindow *control_window;
};

/**
	\brief Part interface to RControlWindow

Part interface to RControlWindow

@author Thomas Friedrichsmeier
*/
class RControlWindowPart : public KParts::Part {
	Q_OBJECT
friend class RControlWindow;
protected:
/** constructor. */
	RControlWindowPart (RControlWindow *widget);
/** destructor */
	~RControlWindowPart ();
};

/**
	\brief ListViewItem used in RControlWindow

A listview-item with a convenience constructor, and storing some additional information. For use in RControlWindow only.

*/
class RControlWindowListViewItem : public QListViewItem {
public:
/** constructor. */
	explicit RControlWindowListViewItem (QListViewItem *parent);
/** constructor. */
	explicit RControlWindowListViewItem (QListView *parent);
/** destructor */
	~RControlWindowListViewItem ();

/** initialize/update item according to command flags, status, etc. */
	void update (RCommand *command);
/** initialize/update item according to chain flags, status, etc. */
	void update (RCommandChain *chain);

/** warning! do not try to access members of this pointer! RCommandChains get deleted in the stack when done, without notice. Use this pointer only to check, whether this is a chain, and to remove it from the RControlWindow::chain_map! */
	RCommandChain *chain;
	bool chain_closed;

	unsigned int id;
	static unsigned int lid;

/** reimplemented to always have the top of the stack at the top */
	int compare (QListViewItem *i, int col, bool ascending) const;
};

#endif
