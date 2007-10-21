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

#include <qlabel.h>
#include <QTreeView>

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

// KDE4 TODO: check, if everything is implemented

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

/** reimplemented to refresh list of commands when showing. This is needed, as the RControlWindow is only kept up to date as long as it is shown. Hence, if it was hidden, and then gets shown, it will have to update the entire list. */
	void showEvent (QShowEvent *e);
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
	QTreeView *commands_view;
	QPushButton *cancel_button;
	QPushButton *pause_button;

	bool paused;
	bool isActive ();
	bool initialized;
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
#if 0
/**
	\brief ListViewItem used in RControlWindow

A listview-item with a convenience constructor, and storing some additional information. For use in RControlWindow only.

*/
class RControlWindowListViewItem : public Q3ListViewItem {
public:
/** constructor. */
	explicit RControlWindowListViewItem (Q3ListViewItem *parent);
/** constructor. */
	explicit RControlWindowListViewItem (Q3ListView *parent);
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
	int compare (Q3ListViewItem *i, int col, bool ascending) const;
};
#endif

#endif
