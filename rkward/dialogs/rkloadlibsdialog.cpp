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

RKLoadLibsDialog::RKLoadLibsDialog (QWidget *parent, RCommandChain *chain) : KDialogBase (parent, 0, false, i18n ("Load / unload packages")) {
	RK_TRACE (DIALOGS);
	RKLoadLibsDialog::chain = chain;
	setMainWidget (new LoadUnloadWidget (this));
	
	error_dialog = new RKErrorDialog (i18n ("The R-backend has reported errors loading/removing or installing one or more packages.\nA transcript of the error message(s) is shown below."), i18n ("Error loading/unloading packages"), false);

	num_child_widgets = 1;
	accepted = false;
}

RKLoadLibsDialog::~RKLoadLibsDialog () {
	RK_TRACE (DIALOGS);
	delete error_dialog;
}

void RKLoadLibsDialog::tryDestruct () {
	RK_TRACE (DIALOGS);
	if (accepted) accept ();
	else reject ();
	
	if (num_child_widgets <= 0) {
		delete this;
	}
}

void RKLoadLibsDialog::childDeleted () {
	RK_TRACE (DIALOGS);
	--num_child_widgets;
	if (should_destruct) tryDestruct ();
}

void RKLoadLibsDialog::slotOk () {
	RK_TRACE (DIALOGS);

	emit (okClicked ());
	accepted = true;
	hide ();
	tryDestruct ();
}

void RKLoadLibsDialog::slotApply () {
	RK_TRACE (DIALOGS);

	emit (apply ());
}

void RKLoadLibsDialog::slotCancel () {
	RK_TRACE (DIALOGS);
	
	emit (cancelClicked ());
	accepted = false;
	hide ();
	tryDestruct ();
}

void RKLoadLibsDialog::closeEvent (QCloseEvent *e) {
	QDialog::closeEvent (e);
	slotCancel ();
}


//////////////////// LoadUnLoadWidget ////////////////////////////

#define GET_INSTALLED_PACKAGES 1
#define GET_LOADED_PACKAGES 2
#define LOAD_PACKAGE_COMMAND 3

LoadUnloadWidget::LoadUnloadWidget (RKLoadLibsDialog *parent) : QWidget (parent) {
	RK_TRACE (DIALOGS);
	LoadUnloadWidget::parent = parent;
	
	QVBoxLayout *mvbox = new QVBoxLayout (this, 0, KDialog::spacingHint ());
	QLabel *label = new QLabel (i18n ("Warning! There are no safeguards against removing essential packages so far. If you remove e.g. package rkward, RKWard will no longer function properly (including this very dialog). So please be careful about the packages you remove."), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	mvbox->addWidget (label);
	
	QHBoxLayout *hbox = new QHBoxLayout (mvbox, KDialog::spacingHint ());
	QVBoxLayout *instvbox = new QVBoxLayout (hbox, KDialog::spacingHint ());
	QVBoxLayout *buttonvbox = new QVBoxLayout (hbox, KDialog::spacingHint ());
	QVBoxLayout *loadedvbox = new QVBoxLayout (hbox, KDialog::spacingHint ());
	
	label = new QLabel (i18n ("Installed packages"), this);
	installed_view = new QListView (this);
	installed_view->addColumn (i18n ("Name"));
	installed_view->addColumn (i18n ("Title"));
	installed_view->addColumn (i18n ("Version"));
	installed_view->addColumn (i18n ("Location"));
	installed_view->setSelectionMode (QListView::Extended);
	instvbox->addWidget (label);
	instvbox->addWidget (installed_view);
	
	load_button = new QPushButton (i18n ("Load"), this);
	connect (load_button, SIGNAL (clicked ()), this, SLOT (loadButtonClicked ()));
	detach_button = new QPushButton (i18n ("Unload"), this);
	connect (detach_button, SIGNAL (clicked ()), this, SLOT (detachButtonClicked ()));
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (load_button);
	buttonvbox->addWidget (detach_button);
	buttonvbox->addStretch (2);
	
	label = new QLabel (i18n ("Loaded packages"), this);
	loaded_view = new QListView (this);
	loaded_view->addColumn (i18n ("Name"));
	loaded_view->setSelectionMode (QListView::Extended);
	loadedvbox->addWidget (label);
	loadedvbox->addWidget (loaded_view);
	
	setEnabled (false);
	
	RKGlobals::rInterface ()->issueCommand (".rk.get.installed.packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_INSTALLED_PACKAGES, parent->chain);
	RKGlobals::rInterface ()->issueCommand (".packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_LOADED_PACKAGES, parent->chain);
	
	connect (parent, SIGNAL (okClicked ()), this, SLOT (ok ()));
	connect (parent, SIGNAL (apply ()), this, SLOT (apply ()));
	connect (parent, SIGNAL (cancelClicked ()), this, SLOT (cancel ()));
	connect (this, SIGNAL (destroyed ()), parent, SLOT (childDeleted ()));
}

LoadUnloadWidget::~LoadUnloadWidget () {
	RK_TRACE (DIALOGS);
}

void LoadUnloadWidget::rCommandDone (RCommand *command) {
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
		setEnabled (true);
		updateCurrentList ();
	} else if (command->getFlags () == LOAD_PACKAGE_COMMAND) {
		if (command->failed ()) {
			parent->error_dialog->newError (command->error ());
		}
	}
}

void LoadUnloadWidget::loadButtonClicked () {
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

void LoadUnloadWidget::detachButtonClicked () {
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

void LoadUnloadWidget::ok () {
	RK_TRACE (DIALOGS);
	doLoadUnload ();
	deleteThis ();
}

void LoadUnloadWidget::updateCurrentList () {
	RK_TRACE (DIALOGS);
	
	prev_packages.clear ();
	QListViewItem *loaded = loaded_view->firstChild ();
	while (loaded) {
		prev_packages.append (loaded->text (0));
		loaded = loaded->nextSibling ();
	}
}

void LoadUnloadWidget::doLoadUnload () {
	RK_TRACE (DIALOGS);
	// load libs previously not loaded
	QListViewItem *loaded = loaded_view->firstChild ();
	while (loaded) {
		if (!prev_packages.contains (loaded->text (0))) {
			RKGlobals::rInterface ()->issueCommand ("library (\"" + loaded->text (0) + "\")", RCommand::App, "", this, LOAD_PACKAGE_COMMAND, parent->chain);
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
			RKGlobals::rInterface ()->issueCommand ("detach (package:" + (*it) + ")", RCommand::App, "", this, LOAD_PACKAGE_COMMAND, parent->chain);
		}
	}
}

void LoadUnloadWidget::apply () {
	RK_TRACE (DIALOGS);

	doLoadUnload ();
	updateCurrentList ();
}

void LoadUnloadWidget::cancel () {
	RK_TRACE (DIALOGS);
	deleteThis ();
}

#include "rkloadlibsdialog.moc"
