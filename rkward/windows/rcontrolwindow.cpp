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

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	QHBoxLayout *button_hbox = new QHBoxLayout ();
	button_hbox->setContentsMargins (0, 0, 0, 0);
	main_vbox->addLayout (button_hbox);

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
		commands_view->header ()->setResizeMode (0, QHeaderView::Stretch);	// can't do this in the ctor, as column 0 does not yet exist
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

void RControlWindow::cancelButtonClicked () {
	RK_TRACE (APP);
	RCommandStackModel::getModel ()->index (0, 0, QModelIndex ());		// side-effect of locking the mutex

	QModelIndexList list = commands_view->selectionModel ()->selectedIndexes ();
	bool some_not_cancelable = false;

	// find out all the RCommands selected (not the chains)
	for (QModelIndexList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
		if ((*it).column ()) continue;		// only react once per row
		RCommandBase* coc = static_cast<RCommandBase*> ((*it).internalPointer ());
		RK_ASSERT (coc);
		RCommand* command = coc->commandPointer ();
		if (command) {
			if (!(command->type () & RCommand::Sync)) {
				RKGlobals::rInterface ()->cancelCommand (command);
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

	RKSettings::configureSettings (RKSettings::PageR, this);
}

//############# END RContolWindow #######################
//############# BEGIN RContolWindowPart ###################

RControlWindowPart::RControlWindowPart (RControlWindow *widget) : KParts::Part () {
	RK_TRACE (APP);

	setComponentData (KGlobal::mainComponent ());

	setWidget (widget);
}

RControlWindowPart::~RControlWindowPart () {
	RK_TRACE (APP);
}

#include "rcontrolwindow.moc"
