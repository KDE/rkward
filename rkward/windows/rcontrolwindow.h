/***************************************************************************
                          rcontrolwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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
#include <kmdichildview.h>

#include <qmap.h>
#include <qlabel.h>

class QListView;
class QPushButton;
class QListViewItem;
class RCommand;
class RCommandChain;
class RChainOrCommand;

/**
	\brief Interface to control R command execution

This class provides a GUI interface to inspect, and manipulate the current RCommandStack, and to Pause/Resume the R engine.
Do create an instance of this class directly. Create a RControlWindowPart instead.

// TODO: probably the QListView of RCommands (and associated functionality) should be separted to an own class
to allow reuse

// TODO: check, if everything is implemented

@author Thomas Friedrichsmeier
*/
class RControlWindow : public KMdiChildView {
	Q_OBJECT
friend class RControlWindowPart;
protected:
/** constructor. Protected. Do not create an instance of this class directly. Rather, create a RControlWindowPart.
@param parent parent QWidget, usually RKGlobals::rkApp () or similar */
	RControlWindow (QWidget *parent = 0);
/** destructor */
	~RControlWindow ();
public:
	void addChain (RCommandChain *chain);
	void addCommand (RCommand *command, RCommandChain *chain);
	void updateChain (RCommandChain *chain);
	void updateCommand (RCommand *command);
	void removeChain (RCommandChain *chain);
	void removeCommand (RCommand *command);

// TODO: move to private. The above functions should be used instead.
/** causes the RControlWindow (if shown) to refresh it's entire list of commands. Warning! Does not lock the mutex. Lock the mutex before calling this! */
	void refreshCommands ();

/** reimplemented to refresh list of commands when showing. */
	void show ();
public slots:
	void commandSelectionChanged ();
	void cancelButtonClicked ();
	void pauseButtonClicked ();
	void configureButtonClicked ();
private:
	QListView *commands_view;
	QPushButton *cancel_button;
	QPushButton *pause_button;
	void addCommands (RChainOrCommand *coc, QListViewItem *parent);
	void addCommand (RCommand *command, QListViewItem *parent);

	QMap <RCommand *, QListViewItem *> command_map;
	QMap <RCommandChain *, QListViewItem *> chain_map;

	bool paused;
};

/**
	\brief Part interface to RControlWindow

Part interface to RControlWindow

@author Thomas Friedrichsmeier
*/
class RControlWindowPart : public KParts::Part {
	Q_OBJECT
public:
/** constructor. */
	RControlWindowPart ();
/** destructor */
	~RControlWindowPart ();
};

#endif
