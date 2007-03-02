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

#include "rcontrolwindow.h"

#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "../settings/rksettings.h"
#include "../rbackend/rinterface.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rcommandstack.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../debug.h"

RControlWindow::RControlWindow (QWidget *parent, bool tool_window, char *name) : RKMDIWindow (parent, PendingJobsWindow, tool_window, name) {
	RK_TRACE (APP);
	setPart (new RControlWindowPart (this));
	initializeActivationSignals ();
	setFocusPolicy (QWidget::ClickFocus);

	QVBoxLayout *main_vbox = new QVBoxLayout (this, RKGlobals::marginHint ());
	QHBoxLayout *button_hbox = new QHBoxLayout (main_vbox, RKGlobals::spacingHint ());

	QPushButton *configure_r_button = new QPushButton (i18n ("Configure R backend"), this);
	connect (configure_r_button, SIGNAL (clicked ()), this, SLOT (configureButtonClicked ()));
	button_hbox->addWidget (configure_r_button);
	button_hbox->addStretch ();

	pause_button = new QPushButton (i18n ("Pause execution"), this);
	connect (pause_button, SIGNAL (clicked ()), this, SLOT (pauseButtonClicked ()));
	button_hbox->addWidget (pause_button);
	button_hbox->addStretch ();

	cancel_button = new QPushButton (i18n ("Cancel selected commands"), this);
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (cancelButtonClicked ()));
	button_hbox->addWidget (cancel_button);

	commands_view = new QListView (this);
	commands_view->addColumn (i18n ("Command"));
	commands_view->addColumn (i18n ("Type"));
	commands_view->addColumn (i18n ("Flags"));
	commands_view->addColumn (i18n ("Description"));
	commands_view->setSorting (0);		// actually, we ignore the column, and do our own sorting
	commands_view->setSelectionMode (QListView::Extended);
	connect (commands_view, SIGNAL (selectionChanged ()), this, SLOT (commandSelectionChanged ()));
	main_vbox->addWidget (commands_view);

	paused = false;
	initialized = false;
}

RControlWindow::~RControlWindow () {
	RK_TRACE (APP);
}

bool RControlWindow::isActive () {
	// don't trace!
	return (initialized && isShown ());
}

void RControlWindow::initialize () {
	RK_TRACE (APP);
	initialized = true;

	if (isShown ()) show ();	// refreshes the commands
}

void RControlWindow::show () {
	RK_TRACE (APP);

	RKMDIWindow::show ();
	if (!initialized) return;
	MUTEX_LOCK;
	refreshCommands ();
	MUTEX_UNLOCK;
}

void RControlWindow::addChain (RCommandChain *chain) {
	if (!isActive ()) return;	// do expensive GUI stuff only when visible
	RK_TRACE (APP);

	RChainOrCommand *dummy = new RChainOrCommand;
	dummy->command = 0;
	dummy->chain = chain;
	addCommands (dummy, chain_map[chain->parent]);
	delete dummy;
}

void RControlWindow::addCommand (RCommand *command, RCommandChain *parent) {
	if (!isActive ()) return;	// do expensive GUI stuff only when visible
	RK_TRACE (APP);

	if (!parent) parent = RCommandStack::regular_stack;
	addCommand (command, chain_map[parent]);
}

void RControlWindow::updateChain (RCommandChain *chain) {
	if (!isActive ()) return;	// do expensive GUI stuff only when visible
	RK_TRACE (APP);

	RControlWindowListViewItem *chainitem = chain_map[chain];
	chainitem->update (chain);
	checkCleanChain (chainitem);
}

void RControlWindow::updateCommand (RCommand *command) {
	if (!isActive ()) return;	// do expensive GUI stuff only when visible
	RK_TRACE (APP);

	RControlWindowListViewItem *item = command_map[command];
	if (!item) {
		RK_ASSERT (false);
		// unfortunately, yes, this can happen! Namely when the command is in the reply stack. We do not find commands in (the) reply stack(s), in refreshCommands.
		// TODO: find a way to include reply stacks in refreshCommands!
		return;
	}
	item->update (command);
}

void RControlWindow::removeCommand (RCommand *command) {
	if (!isActive ()) return;	// do expensive GUI stuff only when visible
	RK_TRACE (APP);

	RControlWindowListViewItem *item = command_map[command];
	if (!item) {
		RK_ASSERT (false);
		// TODO: see updateCommand ()
		return;
	}
	RControlWindowListViewItem *chain = static_cast<RControlWindowListViewItem *> (item->parent ());

	delete item;
	command_map.remove (command);

	checkCleanChain (chain);
}

void RControlWindow::checkCleanChain (RControlWindowListViewItem *chain) {
	if (!isActive ()) return;	// do expensive GUI stuff only when visible
	RK_TRACE (APP);

	while (chain && chain->chain_closed && chain->parent () && (!chain->firstChild ())) {
		RControlWindowListViewItem *del = chain;
		chain = static_cast<RControlWindowListViewItem *> (chain->parent ());
		chain_map.remove (del->chain);
		delete del;
	}
}

void RControlWindow::setCommandRunning (RCommand *command) {
	if (!isActive ()) return;	// do expensive GUI stuff only when visible
	RK_TRACE (APP);

	RControlWindowListViewItem *item = command_map[command];
	if (!item) {
		RK_ASSERT (false);
		// TODO: see updateCommand ()
		return;
	}
	item->setText (2, "Running");
}

void RControlWindow::refreshCommands () {
	RK_TRACE (APP);

	commands_view->clear ();
	command_map.clear ();
	chain_map.clear ();

	RChainOrCommand *dummy = new RChainOrCommand;
	dummy->command = 0;
	dummy->chain = RCommandStack::regular_stack;

	addCommands (dummy, 0);

	delete dummy;

/* add the currently running command (if needed). It is not in the stack. */
	RCommand *running = RKGlobals::rInterface ()->runningCommand ();
	if (running && (!command_map.contains (running))) {
		RControlWindowListViewItem *item = static_cast <RControlWindowListViewItem *> (commands_view->firstChild ());
		while (item->chain && item->firstChild ()) {
			item = static_cast <RControlWindowListViewItem *> (item->firstChild ());
		}
		addCommand (running, item);
	}
	if (running) setCommandRunning (running);

	cancel_button->setEnabled (false);
}

void RControlWindow::addCommands (RChainOrCommand *coc, RControlWindowListViewItem *parent) {
	RK_TRACE (APP);

	if (coc->chain) {
		RControlWindowListViewItem *item;
		RCommandChain *chain = coc->chain;
		if (!parent) {
			item = new RControlWindowListViewItem (commands_view);
			item->setText (0, i18n ("Command Stack"));
		} else {
			item = new RControlWindowListViewItem (parent);
		}
		item->setOpen (true);
		chain_map.insert (chain, item);
		item->update (chain);
		for (RChainOrCommand *nc = chain->commands.first (); nc; nc = chain->commands.next ()) {
			addCommands (nc, item);
		}
	} else {	// coc->command
		RK_ASSERT (parent);
		addCommand (coc->command, parent);
	}
}

void RControlWindow::addCommand (RCommand *command, RControlWindowListViewItem *parent) {
	RK_TRACE (APP);

	RControlWindowListViewItem *item = new RControlWindowListViewItem (parent);
	item->setMultiLinesEnabled (true);
	command_map.insert (command, item);

	item->update (command);
}

void RControlWindow::commandSelectionChanged () {
	RK_TRACE (APP);

	// we will make some modifications to the selection in here, so disconnect the SIGNAL first.
	disconnect (commands_view, SIGNAL (selectionChanged ()), this, SLOT (commandSelectionChanged ()));

	bool have_selection = false;
	// if a chain is selected, select all of its child items
	QListViewItemIterator itemit (commands_view, QListViewItemIterator::Selected);
	while (itemit.current ()) {
		QListViewItem *item = itemit.current ()->firstChild ();
		while (item) {
			item->setSelected (true);
			item = item->nextSibling ();
		}
		have_selection = true;
		++itemit;
	}

	cancel_button->setEnabled (have_selection);

	connect (commands_view, SIGNAL (selectionChanged ()), this, SLOT (commandSelectionChanged ()));
}

void RControlWindow::cancelButtonClicked () {
	RK_TRACE (APP);

	bool some_not_cancelable = false;
	// find out all the RCommands selected (not the chains)
	for (QMap <RCommand *, RControlWindowListViewItem *>::const_iterator it = command_map.begin (); it != command_map.end (); ++it) {
		if (it.data ()->isSelected ()) {
			if (!(it.key ()->type () & RCommand::Sync)) {
				RKGlobals::rInterface ()->cancelCommand (it.key ());
			} else {
				some_not_cancelable = true;
			}
		}
	}

	if (some_not_cancelable) {
		KMessageBox::information (this, i18n ("Some of the commands you were trying to cancel are marked as \"sync\" (letter 'S' in the type column). Cancelling such commands could lead to loss of data. These commands have _not_ been cancelled."), i18n ("Some commands not cancelled"), "cancel_sync");
	}
}

void RControlWindow::pauseButtonClicked () {
	RK_TRACE (APP);

	if (paused) {
		RKGlobals::rInterface ()->pauseProcessing (false);
		pause_button->setText (i18n ("Pause execution"));
		paused = false;
	} else {
		RKGlobals::rInterface ()->pauseProcessing (true);
		pause_button->setText (i18n ("Resume execution"));
		paused = true;
	}
}

void RControlWindow::configureButtonClicked () {
	RK_TRACE (APP);

	RKSettings::configureSettings (RKSettings::R, this);
}

//############# END RContolWindow #######################
//############# BEGIN RContolWindowPart ###################

RControlWindowPart::RControlWindowPart (RControlWindow *widget) : KParts::Part () {
	RK_TRACE (APP);
	setWidget (widget);
}

RControlWindowPart::~RControlWindowPart () {
	RK_TRACE (APP);
}

//############# END RContolWindowPart #######################
//############# BEGIN RContolWindowListViewItem #################

// static
unsigned int RControlWindowListViewItem::lid = 0;

RControlWindowListViewItem::RControlWindowListViewItem (QListViewItem *parent) : QListViewItem (parent) {
	chain = 0;
	chain_closed = false;

	id = ++lid;
}

RControlWindowListViewItem::RControlWindowListViewItem (QListView *parent) : QListViewItem (parent) {
	chain = 0;
	chain_closed = false;

	id = ++lid;
}

RControlWindowListViewItem::~RControlWindowListViewItem () {
}

int RControlWindowListViewItem::compare (QListViewItem *i, int, bool ascending) const {
	unsigned int comp_id = static_cast<RControlWindowListViewItem *> (i)->id;
	if (ascending) {
		if (comp_id > id) {
			return -1;
		} /* else if (comp_id == id) {		// all items have a unique id!
			return 0;
		}*/
		return 1;
	} else {
		if (comp_id > id) {
			return 1;
		} /* else if (comp_id == id) {		// all items have a unique id!
			return 0;
		}*/
		return -1;
	}
}

void RControlWindowListViewItem::update (RCommand *command) {
	RK_TRACE (APP);
	RK_ASSERT (this);

	QString dummy = command->command ().left (40).stripWhiteSpace ();
	if (dummy.length () > 37) {
		dummy = dummy.left (37) + "...";
	}
	setText (0, dummy);

	dummy = "";
	if (command->type () & RCommand::User) dummy += 'U';
	if (command->type () & RCommand::Plugin) dummy += 'P';
	if (command->type () & RCommand::PluginCom) dummy += 'C';
	if (command->type () & RCommand::App) dummy += 'A';
	if (command->type () & RCommand::Sync) dummy += 'S';
	if (command->type () & RCommand::EmptyCommand) dummy += 'E';
	if (command->type () & (RCommand::GetIntVector | RCommand::GetRealVector | RCommand::GetStringVector)) dummy += 'D';
	if (command->type () & RCommand::DirectToOutput) dummy += 'O';
	setText (1, dummy);

	if (command->getStatus () & RCommand::Canceled) setText (2, i18n ("Cancelled"));

	setText (3, command->rkEquivalent ());
}

void RControlWindowListViewItem::update (RCommandChain *cchain) {
	RK_TRACE (APP);
	RK_ASSERT (this);
	
	chain = cchain;
	chain_closed = cchain->closed;

	setText (1, i18n ("Chain"));
	if (chain_closed) {
		setText (2, i18n ("Closed"));
	} else {
		setText (2, i18n ("Open (Waiting)"));
	}
}

#include "rcontrolwindow.moc"
