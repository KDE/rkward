/***************************************************************************
                          rkloadlibsdialog  -  description
                             -------------------
    begin                : Mon Sep 6 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rkloadlibsdialog.h"

#include <qwidget.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <klocale.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../misc/rkerrordialog.h"

#include "../debug.h"

#define GET_INSTALLED_PACKAGES 1
#define GET_LOADED_PACKAGES 2
#define LOAD_PACKAGE_COMMAND 3

RKLoadLibsDialog::RKLoadLibsDialog (QWidget *parent) : KDialogBase (parent, 0, false, i18n ("Load / unload packages")) {
	RK_TRACE (DIALOGS);
	widget = new QWidget (this);
	setMainWidget (widget);
	
	QVBoxLayout *mvbox = new QVBoxLayout (widget, 0, spacingHint ());
	QLabel *label = new QLabel (i18n ("Warning! There are no safeguards against removing essential packages so far. If you remove e.g. package rkward, RKWard will no longer function properly (including this very dialog). So please be careful about the packages you remove."), widget);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	mvbox->addWidget (label);
	
	QHBoxLayout *hbox = new QHBoxLayout (mvbox, spacingHint ());
	QVBoxLayout *instvbox = new QVBoxLayout (hbox, spacingHint ());
	QVBoxLayout *buttonvbox = new QVBoxLayout (hbox, spacingHint ());
	QVBoxLayout *loadedvbox = new QVBoxLayout (hbox, spacingHint ());
	
	label = new QLabel (i18n ("Installed packages"), widget);
	installed_view = new QListView (widget);
	installed_view->addColumn (i18n ("Name"));
	installed_view->addColumn (i18n ("Title"));
	installed_view->addColumn (i18n ("Version"));
	installed_view->addColumn (i18n ("Location"));
	installed_view->setSelectionMode (QListView::Extended);
	instvbox->addWidget (label);
	instvbox->addWidget (installed_view);
	
	load_button = new QPushButton (i18n ("Load"), widget);
	connect (load_button, SIGNAL (clicked ()), this, SLOT (loadButtonClicked ()));
	detach_button = new QPushButton (i18n ("Unload"), widget);
	connect (detach_button, SIGNAL (clicked ()), this, SLOT (detachButtonClicked ()));
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (load_button);
	buttonvbox->addWidget (detach_button);
	buttonvbox->addStretch (2);
	
	label = new QLabel (i18n ("Loaded packages"), widget);
	loaded_view = new QListView (widget);
	loaded_view->addColumn (i18n ("Name"));
	loaded_view->setSelectionMode (QListView::Extended);
	loadedvbox->addWidget (label);
	loadedvbox->addWidget (loaded_view);
	
	widget->setEnabled (false);
	
	RKGlobals::rInterface ()->issueCommand (".rk.get.installed.packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_INSTALLED_PACKAGES);
	RKGlobals::rInterface ()->issueCommand (".packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_LOADED_PACKAGES);
	
	error_dialog = new RKErrorDialog (i18n ("The R-backend has reported errors loading/removing one or more packages.\nA transcript of the error message(s) is shown below."), i18n ("Error loading/unloading packages"), false);
}

RKLoadLibsDialog::~RKLoadLibsDialog () {
	RK_TRACE (DIALOGS);
	delete error_dialog;
}

void RKLoadLibsDialog::rCommandDone (RCommand *command) {
	RK_TRACE (DIALOGS);
	if (command->getFlags () == GET_INSTALLED_PACKAGES) {
		RK_ASSERT ((command->stringVectorLength () % 4) == 0);
		int count = (command->stringVectorLength () / 4);
		for (int i=0; i < count; ++i) {
			new QListViewItem (installed_view, command->getStringVector ()[i], command->getStringVector ()[count + i], command->getStringVector ()[2*count + i], command->getStringVector ()[3* count + i]);
		}
	} else if (command->getFlags () == GET_LOADED_PACKAGES) {
		for (int i=0; i < command->stringVectorLength (); ++i) {
			new QListViewItem (loaded_view, command->getStringVector ()[i]);
		}
		widget->setEnabled (true);
		updateCurrentList ();
	} else if (command->getFlags () == LOAD_PACKAGE_COMMAND) {
		if (command->failed ()) {
			error_dialog->newError (command->error ());
		}
	}
}

void RKLoadLibsDialog::loadButtonClicked () {
	RK_TRACE (DIALOGS);
	
	QListViewItem *installed = installed_view->firstChild ();
	while (installed) {
		if (installed->isSelected ()) {
			QListViewItem *loaded = loaded_view->firstChild ();
			// find out, whether package is already loaded
			bool dup = false;
			while (loaded) {
				if (loaded->text (0) == installed->text (0)) {
					dup = true;
					loaded = 0;
				} else {
					loaded = loaded->nextSibling ();
				}
			}
			if (!dup) {
				new QListViewItem (loaded_view, installed->text (0));
			}
		}
		installed = installed->nextSibling ();
	}
}

void RKLoadLibsDialog::detachButtonClicked () {
	RK_TRACE (DIALOGS);
	
	QListViewItem *loaded = loaded_view->firstChild ();
	while (loaded) {
		QListViewItem *next = loaded->nextSibling ();
		if (loaded->isSelected ()) {
			delete loaded;
		}
		loaded = next;
	}
}

void RKLoadLibsDialog::slotOk () {
	RK_TRACE (DIALOGS);
	doLoadUnload ();
	if (isModal ()) KDialogBase::slotOk ();
	hide ();
	deleteThis ();
}

void RKLoadLibsDialog::updateCurrentList () {
	RK_TRACE (DIALOGS);
	
	prev_packages.clear ();
	QListViewItem *loaded = loaded_view->firstChild ();
	while (loaded) {
		prev_packages.append (loaded->text (0));
		loaded = loaded->nextSibling ();
	}
}

void RKLoadLibsDialog::doLoadUnload () {
	RK_TRACE (DIALOGS);
	// load libs previously not loaded
	QListViewItem *loaded = loaded_view->firstChild ();
	while (loaded) {
		if (!prev_packages.contains (loaded->text (0))) {
			RKGlobals::rInterface ()->issueCommand ("library (\"" + loaded->text (0) + "\")", RCommand::App, "", this, LOAD_PACKAGE_COMMAND);
		}
		loaded = loaded->nextSibling ();
	}
	
	// detach libs previously attached
	for (QStringList::Iterator it = prev_packages.begin (); it != prev_packages.end (); ++it) {
		bool found = false;
		loaded = loaded_view->firstChild ();
		while (loaded) {
			QListViewItem *next = loaded->nextSibling ();
			if (loaded->text (0) == (*it)) {
				found = true;
				loaded = 0;
			}
			loaded = next;
		}
		if (!found) {
			RKGlobals::rInterface ()->issueCommand ("detach (package:" + (*it) + ")", RCommand::App, "", this, LOAD_PACKAGE_COMMAND);
		}
	}
}

void RKLoadLibsDialog::slotApply () {
	RK_TRACE (DIALOGS);

	doLoadUnload ();
	updateCurrentList ();
}

void RKLoadLibsDialog::slotCancel () {
	RK_TRACE (DIALOGS);
	if (isModal ()) KDialogBase::slotCancel ();
	hide ();
	deleteThis ();
}

void RKLoadLibsDialog::closeEvent (QCloseEvent *e) {
	QDialog::closeEvent (e);
	slotCancel ();
}

#include "rkloadlibsdialog.moc"
