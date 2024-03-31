/*
rcontrolwindow - This file is part of RKWard (https://rkward.kde.org). Created: Wed Oct 12 2005
SPDX-FileCopyrightText: 2005-2009 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rcontrolwindow.h"

#include <qpushbutton.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>

#include <KLocalizedString>
#include <kmessagebox.h>

#include "../settings/rksettings.h"
#include "../misc/rkdummypart.h"
#include "../rbackend/rkrinterface.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rcommandstack.h"
#include "../rkward.h"
#include "../debug.h"

//static
RControlWindow *RControlWindow::control_window = nullptr;

RControlWindow::RControlWindow (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, PendingJobsWindow, tool_window, name) {
	RK_TRACE (APP);
	setPart (new RKDummyPart(nullptr, this));
	initializeActivationSignals ();
	setFocusPolicy (Qt::ClickFocus);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	QHBoxLayout *button_hbox = new QHBoxLayout ();
	button_hbox->setContentsMargins (0, 0, 0, 0);
	main_vbox->addLayout (button_hbox);

	QPushButton *configure_r_button = new QPushButton (i18n ("Configure R backend"), this);
	connect (configure_r_button, &QPushButton::clicked, this, &RControlWindow::configureButtonClicked);
	button_hbox->addWidget (configure_r_button);
	button_hbox->addStretch ();

	pause_button = new QPushButton (i18n ("Pause execution"), this);
	connect (pause_button, &QPushButton::clicked, this, &RControlWindow::pauseButtonClicked);
	button_hbox->addWidget (pause_button);
	button_hbox->addStretch ();

	cancel_button = new QPushButton (i18n ("Cancel selected commands"), this);
	connect (cancel_button, &QPushButton::clicked, this, &RControlWindow::cancelButtonClicked);
	button_hbox->addWidget (cancel_button);

	commands_view = new QTreeView (this);

	commands_view->setSortingEnabled (false);
	commands_view->header ()->setSectionsMovable (false);
	commands_view->header ()->setStretchLastSection (false);

	commands_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
	main_vbox->addWidget (commands_view);

	paused = false;
}

RControlWindow::~RControlWindow () {
	RK_TRACE (APP);

	if (commands_view->model ()) {
		commands_view->setModel(nullptr);
		RCommandStackModel::getModel ()->removeListener ();
	}
}

void RControlWindow::showEvent (QShowEvent *e) {
	RK_TRACE (APP);

	if (!commands_view->model ()) {
		RCommandStackModel::getModel ()->addListener ();
		commands_view->setModel (RCommandStackModel::getModel ());
		commands_view->header ()->setSectionResizeMode (0, QHeaderView::Stretch);	// can't do this in the ctor, as column 0 does not yet exist
		commands_view->expandAll ();
	}

	RKMDIWindow::showEvent (e);
}

void RControlWindow::hideEvent (QHideEvent *e) {
	RK_TRACE (APP);

	if (commands_view->model ()) {
		commands_view->setModel(nullptr);
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
		RCommandChain* coc = static_cast<RCommandChain*> ((*it).internalPointer ());
		RK_ASSERT (coc);
		RCommand* command = coc->toCommand ();
		if (command) {
			if (!(command->type () & RCommand::Sync)) {
				RInterface::instance()->cancelCommand(command);
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
		RInterface::instance()->pauseProcessing(false);
		pause_button->setText (i18n ("Pause execution"));
		paused = false;
	} else {
		RInterface::instance()->pauseProcessing(true);
		pause_button->setText (i18n ("Resume execution"));
		paused = true;
	}
}

void RControlWindow::configureButtonClicked () {
	RK_TRACE (APP);

	RKSettings::configureSettings (RKSettings::PageR, this);
}

