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

#include "rcontrolwindow.h"

#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <klocale.h>

#include "../settings/rksettings.h"
#include "../rbackend/rinterface.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rcommandstack.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../debug.h"

RControlWindow::RControlWindow (QWidget *parent) : KMdiChildView (parent) {
	RK_TRACE (APP);

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
	commands_view->setSorting (-1);
	main_vbox->addWidget (commands_view);

	paused = false;
}

RControlWindow::~RControlWindow () {
	RK_TRACE (APP);
}

void RControlWindow::show () {
	RK_TRACE (APP);

//	refreshCommands ();		// TODO: can't do this yet. Take care of the mutext first.
	KMdiChildView::show ();
}

void RControlWindow::addChain (RCommandChain *chain) {
	RK_TRACE (APP);

	RChainOrCommand *dummy = new RChainOrCommand;
	dummy->command = 0;
	dummy->chain = chain;
	addCommands (dummy, chain_map[chain]);
	delete dummy;
}

void RControlWindow::addCommand (RCommand *command, RCommandChain *chain) {
	RK_TRACE (APP);

	addCommand (command, chain_map[chain]);
}

void RControlWindow::updateChain (RCommandChain *chain) {
	RK_TRACE (APP);
}

void RControlWindow::updateCommand (RCommand *command) {
	RK_TRACE (APP);
}

void RControlWindow::removeChain (RCommandChain *chain) {
	RK_TRACE (APP);
}

void RControlWindow::removeCommand (RCommand *command) {
	RK_TRACE (APP);
}

void RControlWindow::refreshCommands () {
//	if (!isShown ()) return;	// do expensive GUI stuff only when visible
	RK_TRACE (APP);

	commands_view->clear ();
	command_map.clear ();
	chain_map.clear ();

	RChainOrCommand *dummy = new RChainOrCommand;
	dummy->command = 0;
	dummy->chain = RCommandStack::regular_stack;

	addCommands (dummy, 0);

	delete dummy;
}

void RControlWindow::addCommands (RChainOrCommand *coc, QListViewItem *parent) {
//	if (!isShown ()) return;	// do expensive GUI stuff only when visible
	RK_TRACE (APP);

	if (coc->chain) {
		QListViewItem *item;
		RCommandChain *chain = coc->chain;
		if (!parent) {
			item = new QListViewItem (commands_view, i18n ("Command Stack"));
		} else {
			item = new QListViewItem (parent);
		}
		item->setText (1, i18n ("Chain"));
		if (chain->closed) {
			item->setText (2, i18n ("Closed"));
		} else {
			item->setText (2, i18n ("Open"));
		}
		item->setOpen (true);
		chain_map.insert (chain, item);
		// new QListViewItems are always added at the top, so we need to walk the list backwards
		for (RChainOrCommand *nc = chain->commands.last (); nc; nc = chain->commands.prev ()) {
			addCommands (nc, item);
		}
	} else {	// coc->command
		RK_ASSERT (parent);
		addCommand (coc->command, parent);
	}
}

void RControlWindow::addCommand (RCommand *command, QListViewItem *parent) {
	RK_TRACE (APP);

	QString text = command->command ().left (40);
	if (text.length () > 37) {
		text = text.left (37) + "...";
	}

	QString type;
	if (command->type () & RCommand::User) type += "U";
	if (command->type () & RCommand::Plugin) type += "P";
	if (command->type () & RCommand::PluginCom) type += "C";
	if (command->type () & RCommand::App) type += "A";
	if (command->type () & RCommand::Sync) type += "S";
	if (command->type () & RCommand::EmptyCommand) type += "E";
	if (command->type () & (RCommand::GetIntVector | RCommand::GetRealVector | RCommand::GetStringVector)) type += "D";
	if (command->type () & RCommand::DirectToOutput) type += "O";

	QString flags;
	if (command->type () & RCommand::Canceled) flags = i18n ("Cancelled");

	QListViewItem *item = new QListViewItem (parent, text, type, flags, command->rkEquivalent ());
	item->setMultiLinesEnabled (true);
	command_map.insert (command, item);
}

void RControlWindow::commandSelectionChanged () {
	RK_TRACE (APP);
}

void RControlWindow::cancelButtonClicked () {
	RK_TRACE (APP);
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

RControlWindowPart::RControlWindowPart () : KParts::Part () {
	RK_TRACE (APP);
	setWidget (new RControlWindow (0));
}

RControlWindowPart::~RControlWindowPart () {
	RK_TRACE (APP);
}

#include "rcontrolwindow.moc"
