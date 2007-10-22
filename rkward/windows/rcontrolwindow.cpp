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

#include <qpushbutton.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>

#include <klocale.h>
#include <kmessagebox.h>

#include "../settings/rksettings.h"
#include "../rbackend/rinterface.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rcommandstack.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../debug.h"

//static
RControlWindow *RControlWindow::control_window = 0;

RControlWindow::RControlWindow (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, PendingJobsWindow, tool_window, name) {
	RK_TRACE (APP);
	setPart (new RControlWindowPart (this));
	initializeActivationSignals ();
	setFocusPolicy (Qt::ClickFocus);

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

	commands_view = new QTreeView (this);

	commands_view->setSortingEnabled (false);
	commands_view->header ()->setMovable (false);
	commands_view->header ()->setStretchLastSection (false);

	commands_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
	main_vbox->addWidget (commands_view);

	paused = false;
}

RControlWindow::~RControlWindow () {
	RK_TRACE (APP);

	commands_view->setModel (0);
	RCommandStackModel::getModel ()->removeListener ();
}

void RControlWindow::showEvent (QShowEvent *e) {
	RK_TRACE (APP);

	if (!commands_view->model ()) {
		RCommandStackModel::getModel ()->addListener ();
		commands_view->setModel (RCommandStackModel::getModel ());
		commands_view->header ()->setResizeMode (0, QHeaderView::Stretch);
		commands_view->expandAll ();
	}

	RKMDIWindow::showEvent (e);
}

void RControlWindow::hideEvent (QHideEvent *e) {
	RK_TRACE (APP);

	if (commands_view->model ()) {
		commands_view->setModel (0);
		RCommandStackModel::getModel ()->removeListener ();
	}

	RKMDIWindow::hideEvent (e);
}

void RControlWindow::commandSelectionChanged () {
	RK_TRACE (APP);
/*
	// we will make some modifications to the selection in here, so disconnect the SIGNAL first.
	disconnect (commands_view, SIGNAL (selectionChanged ()), this, SLOT (commandSelectionChanged ()));

	bool have_selection = false;
	// if a chain is selected, select all of its child items
	Q3ListViewItemIterator itemit (commands_view, Q3ListViewItemIterator::Selected);
	while (itemit.current ()) {
		Q3ListViewItem *item = itemit.current ()->firstChild ();
		while (item) {
			item->setSelected (true);
			item = item->nextSibling ();
		}
		have_selection = true;
		++itemit;
	}

	cancel_button->setEnabled (have_selection);

	connect (commands_view, SIGNAL (selectionChanged ()), this, SLOT (commandSelectionChanged ())); */
}

void RControlWindow::cancelButtonClicked () {
	RK_TRACE (APP);
/*
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
	} */
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

	RKSettings::configureSettings (RKSettings::PageR, this);
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
/*
// static
unsigned int RControlWindowListViewItem::lid = 0;

RControlWindowListViewItem::RControlWindowListViewItem (Q3ListViewItem *parent) : Q3ListViewItem (parent) {
	chain = 0;
	chain_closed = false;

	id = ++lid;
}

RControlWindowListViewItem::RControlWindowListViewItem (Q3ListView *parent) : Q3ListViewItem (parent) {
	chain = 0;
	chain_closed = false;

	id = ++lid;
}

RControlWindowListViewItem::~RControlWindowListViewItem () {
}

int RControlWindowListViewItem::compare (Q3ListViewItem *i, int, bool ascending) const {
	unsigned int comp_id = static_cast<RControlWindowListViewItem *> (i)->id;
	if (ascending) {
		if (comp_id > id) {
			return -1;
		}
		return 1;
	} else {
		if (comp_id > id) {
			return 1;
		}
		return -1;
	}
}

void RControlWindowListViewItem::update (RCommand *command) {
	RK_TRACE (APP);
	RK_ASSERT (this);

	QString dummy = command->command ().left (40).trimmed ();
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
*/
#include "rcontrolwindow.moc"
