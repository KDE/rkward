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
#include <qcheckbox.h>
#include <qdir.h>

#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettings.h"
#include "../misc/rkerrordialog.h"
#include "../misc/rkcanceldialog.h"

#include "../debug.h"

#define DOWNLOAD_PACKAGES_COMMAND 1

RKLoadLibsDialog::RKLoadLibsDialog (QWidget *parent, RCommandChain *chain, bool modal) : KDialogBase (KDialogBase::Tabbed, Qt::WStyle_DialogBorder, parent, 0, modal, i18n ("Configure Packages"), KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel | KDialogBase::User1) {
	RK_TRACE (DIALOGS);
	RKLoadLibsDialog::chain = chain;
	
	QFrame *page = addPage (i18n ("Local packages"));
	QVBoxLayout *layout = new QVBoxLayout (page, 0, KDialog::spacingHint ());
	layout->addWidget (new LoadUnloadWidget (this, page));
	
	page = addPage (i18n ("Update"));
	layout = new QVBoxLayout (page, 0, KDialog::spacingHint ());
	layout->addWidget (new UpdatePackagesWidget (this, page));

	page = addPage (i18n ("Install"));
	layout = new QVBoxLayout (page, 0, KDialog::spacingHint ());
	layout->addWidget (new InstallPackagesWidget (this, page));

	error_dialog = new RKErrorDialog (i18n ("The R-backend has reported errors handling packages.\nA transcript of the errors is shown below."), i18n ("Error handling packages"), false);
	installation_error_dialog = new RKErrorDialog (i18n ("There was an error or warning while installing the packages.\nPlease check the output below for more information."), i18n ("Error installing packages"), false);

	setButtonText (KDialogBase::User1, i18n ("Configure Repositories"));

	num_child_widgets = 3;
	accepted = false;
}

RKLoadLibsDialog::~RKLoadLibsDialog () {
	RK_TRACE (DIALOGS);
	delete error_dialog;
	delete installation_error_dialog;
}

//static
void RKLoadLibsDialog::showInstallPackagesModal (QWidget *parent, RCommandChain *chain) {
	RK_TRACE (DIALOGS);

	RKLoadLibsDialog *dialog = new RKLoadLibsDialog (parent, chain, true);
	dialog->showPage (2);
	dialog->exec ();
}

void RKLoadLibsDialog::tryDestruct () {
	RK_TRACE (DIALOGS);
	if (accepted) accept ();
	else reject ();
	
	if (num_child_widgets <= 0) {
		deleteThis ();
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

void RKLoadLibsDialog::slotUser1 () {
	RK_TRACE (DIALOGS);

	RKSettings::configureSettings (RKSettings::R, this, chain);
}

void RKLoadLibsDialog::closeEvent (QCloseEvent *e) {
	RK_TRACE (DIALOGS);
	QDialog::closeEvent (e);
	slotCancel ();
}

void RKLoadLibsDialog::rCommandDone (RCommand *command) {
	RK_TRACE (DIALOGS);
	if (command->getFlags () == DOWNLOAD_PACKAGES_COMMAND) {
		if (command->failed ()) error_dialog->newError (command->error ());
		emit (downloadComplete ());
	} else {
		RK_ASSERT (false);
	}
}

bool RKLoadLibsDialog::downloadPackages (const QStringList &packages) {
	RK_TRACE (DIALOGS);

	QString to_dir = QDir (RKSettingsModuleGeneral::filesPath ()).filePath (".packagetemp");

	if (packages.isEmpty ()) return false;
	
	QString package_string = "c (\"" + packages.join ("\", \"") + "\")";
	RCommand *command = new RCommand ("download.packages (pkgs=" + package_string + ", destdir=\"" + to_dir + "\")", RCommand::App, QString::null, this, DOWNLOAD_PACKAGES_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, chain);
	
	if (RKCancelDialog::showCancelDialog (i18n ("Fetch list"), i18n ("Please, stand by while downloading the list of packages."), this, this, SIGNAL (downloadComplete ()), command) == QDialog::Rejected) return false;
	return true;
}

void RKLoadLibsDialog::installDownloadedPackages (bool become_root) {
	RK_TRACE (DIALOGS);
	QDir tempdir = QDir (RKSettingsModuleGeneral::filesPath ()).filePath (".packagetemp");

	tempdir.setFilter (QDir::Files);

	KProcess *proc = new KProcess;
	if (become_root) {
		*proc << "kdesu" << "-t";
	};
	*proc << "R" << "CMD" << "INSTALL";
	for (unsigned int i=0; i < tempdir.count (); ++i) {
		*proc << tempdir.filePath (tempdir[i]).latin1 ();
	}

	connect (proc, SIGNAL (processExited (KProcess *)), this, SLOT (processExited (KProcess *)));
	connect (proc, SIGNAL (receivedStdout (KProcess *, char *, int)), this, SLOT (installationProcessOutput (KProcess *, char *, int)));
	connect (proc, SIGNAL (receivedStderr (KProcess *, char *, int)), this, SLOT (installationProcessOutput (KProcess *, char *, int)));
	proc->start (KProcess::NotifyOnExit, KProcess::AllOutput);
	if (RKCancelDialog::showCancelDialog (i18n ("Installing packages..."), i18n ("Please, stand by while installing packages."), this, this, SIGNAL (installationComplete ())) == QDialog::Rejected) {
		proc->kill ();
	};
	if ((!proc->normalExit ()) || (proc->exitStatus ())) {
		installation_error_dialog->newError (QString::null);		// to make sure it is shown
	}
	installation_error_dialog->resetOutput ();
	delete proc;

	// archive / delete packages
	bool ok = true;
	if (RKSettingsModuleR::archivePackages ()) {
		// step 1: create archive-dir, if neccessary
		QDir archivedir = QDir (RKSettingsModuleGeneral::filesPath ()).filePath ("package_archive");
		if (!archivedir.exists ()) {
			// does not compile on some systems (October 2005).
			//QDir (RKSettingsModuleLogfiles::filesPath ()).mkdir ("package_archive");
			// Workaround (use this instead for a couple of months):
			QString dummy = RKSettingsModuleGeneral::filesPath ();
			QDir dir;
			dir.setPath (dummy);
			dir.mkdir ("package_archive");
		} if (!archivedir.isReadable ()) {
			RK_DO (qDebug ("Directory '%s' could not be created or is not readable", archivedir.absPath ().latin1 ()), DIALOGS, DL_ERROR);
			return;
		}

		for (unsigned int i=0; i < tempdir.count (); ++i) {
			if (!tempdir.rename (tempdir[i].latin1 (), archivedir.absFilePath (tempdir[i].latin1 ()))) ok = false;
		}
	} else {
		for (unsigned int i=0; i < tempdir.count (); ++i) {
			if (!QFile (tempdir.filePath (tempdir[i]).latin1 ()).remove ()) ok = false;
		}
	}

	if (!ok) {
		RK_DO (qDebug ("One or more package files could not be moved/delted"), DIALOGS, DL_ERROR);
	}
}

void RKLoadLibsDialog::installationProcessOutput (KProcess *, char *buffer, int buflen) {
	RK_TRACE (DIALOGS);
	installation_error_dialog->newOutput (QCString (buffer, buflen));
}

void RKLoadLibsDialog::processExited (KProcess *) {
	RK_TRACE (DIALOGS);
	emit (installationComplete ());
}

////////////////////// LoadUnloadWidget ////////////////////////////

#define GET_INSTALLED_PACKAGES 1
#define GET_LOADED_PACKAGES 2
#define LOAD_PACKAGE_COMMAND 3

LoadUnloadWidget::LoadUnloadWidget (RKLoadLibsDialog *dialog, QWidget *p_widget) : QWidget (p_widget) {
	RK_TRACE (DIALOGS);
	LoadUnloadWidget::parent = dialog;
	
	QVBoxLayout *mvbox = new QVBoxLayout (this, 0, KDialog::spacingHint ());
	QLabel *label = new QLabel (i18n ("There are no safeguards against removing essential packages. For example, unloading \"rkward\" will prevent this application from running properly. Please, be careful about the packages you unload."), this);
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
	
	RKGlobals::rInterface ()->issueCommand (".rk.get.installed.packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_INSTALLED_PACKAGES, dialog->chain);
	RKGlobals::rInterface ()->issueCommand (".packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_LOADED_PACKAGES, dialog->chain);
	
	connect (dialog, SIGNAL (okClicked ()), this, SLOT (ok ()));
	connect (dialog, SIGNAL (apply ()), this, SLOT (apply ()));
	connect (dialog, SIGNAL (cancelClicked ()), this, SLOT (cancel ()));
	connect (this, SIGNAL (destroyed ()), dialog, SLOT (childDeleted ()));
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
	} else {
		RK_ASSERT (false);
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
	// load packages previously not loaded
	QListViewItem *loaded = loaded_view->firstChild ();
	while (loaded) {
		if (!prev_packages.contains (loaded->text (0))) {
			RKGlobals::rInterface ()->issueCommand ("library (\"" + loaded->text (0) + "\")", RCommand::App, QString::null, this, LOAD_PACKAGE_COMMAND, parent->chain);
		}
		loaded = loaded->nextSibling ();
	}
	
	// detach packages previously attached
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
			RKGlobals::rInterface ()->issueCommand ("detach (package:" + (*it) + ")", RCommand::App, QString::null, this, LOAD_PACKAGE_COMMAND, parent->chain);
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

/////////////////////// UpdatePackagesWidget //////////////////////////

#define FIND_OLD_PACKAGES_COMMAND 1

UpdatePackagesWidget::UpdatePackagesWidget (RKLoadLibsDialog *dialog, QWidget *p_widget) : QWidget (p_widget) {
	RK_TRACE (DIALOGS);
	UpdatePackagesWidget::parent = dialog;
	
	QVBoxLayout *mvbox = new QVBoxLayout (this, 0, KDialog::spacingHint ());
	QLabel *label = new QLabel (i18n ("In order to find out, which of your installed packaged have an update available, click \"Fetch List\". This feature requires a working internet connection."), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	mvbox->addWidget (label);
	
	QHBoxLayout *hbox = new QHBoxLayout (mvbox, KDialog::spacingHint ());
	
	updateable_view = new QListView (this);
	updateable_view->addColumn (i18n ("Name"));
	updateable_view->addColumn (i18n ("Location"));
	updateable_view->addColumn (i18n ("Local"));
	updateable_view->addColumn (i18n ("Online"));
	updateable_view->setSelectionMode (QListView::Extended);
	hbox->addWidget (updateable_view);
	
	QVBoxLayout *buttonvbox = new QVBoxLayout (hbox, KDialog::spacingHint ());
	get_list_button = new QPushButton (i18n ("Fetch list"), this);
	connect (get_list_button, SIGNAL (clicked ()), this, SLOT (getListButtonClicked ()));
	update_selected_button = new QPushButton (i18n ("Update Selected"), this);
	connect (update_selected_button, SIGNAL (clicked ()), this, SLOT (updateSelectedButtonClicked ()));
	update_all_button = new QPushButton (i18n ("Update All"), this);
	connect (update_selected_button, SIGNAL (clicked ()), this, SLOT (updateAllButtonClicked ()));
	become_root_box = new QCheckBox (i18n ("As \"root\" user"), this);
	become_root_box->setChecked (true);
	buttonvbox->addWidget (get_list_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (update_selected_button);
	buttonvbox->addWidget (update_all_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (become_root_box);
	buttonvbox->addStretch (1);
	
	update_selected_button->setEnabled (false);
	update_all_button->setEnabled (false);
	updateable_view->setEnabled (false);
	//placeholder = new QListViewItem (updateable_view, i18n ("[Click \"Fetch list\" for updates]"));
	placeholder = new QListViewItem (updateable_view, "...");
	
	connect (dialog, SIGNAL (okClicked ()), this, SLOT (ok ()));
	connect (dialog, SIGNAL (cancelClicked ()), this, SLOT (cancel ()));
	connect (this, SIGNAL (destroyed ()), dialog, SLOT (childDeleted ()));
}

UpdatePackagesWidget::~UpdatePackagesWidget () {
	RK_TRACE (DIALOGS);
	delete placeholder;
}

void UpdatePackagesWidget::rCommandDone (RCommand *command) {
	RK_TRACE (DIALOGS);
	if (command->getFlags () == FIND_OLD_PACKAGES_COMMAND) {
		emit (actionDone ());
		if (!command->failed ()) {
			delete placeholder;
			placeholder = 0;
			RK_ASSERT ((command->stringVectorLength () % 4) == 0);
			int count = (command->stringVectorLength () / 4);
			for (int i=0; i < count; ++i) {
				new QListViewItem (updateable_view, command->getStringVector ()[i], command->getStringVector ()[count + i], command->getStringVector ()[2*count + i], command->getStringVector ()[3* count + i]);
			}
			updateable_view->setEnabled (true);

			if (updateable_view->firstChild ()) {
				update_selected_button->setEnabled (true);
				update_all_button->setEnabled (true);
			} else {
				placeholder = new QListViewItem (updateable_view, i18n ("[No updates available]"));
			}
		} else {
			get_list_button->setEnabled (true);
			parent->error_dialog->newError (command->error ());
		}
	} else {
		RK_ASSERT (false);
	}
}

void UpdatePackagesWidget::updatePackages (const QStringList &list) {
	if (!list.isEmpty ()) {
		if (parent->downloadPackages (list)) parent->installDownloadedPackages (become_root_box->isChecked ());
	}
}

void UpdatePackagesWidget::updateSelectedButtonClicked () {
	RK_TRACE (DIALOGS);
	QStringList list;
	for (QListViewItem *item = updateable_view->firstChild (); item; item = item->nextSibling ()) {
		if (item->isSelected ()) list.append (item->text (0));
	}
	updatePackages (list);
}

void UpdatePackagesWidget::updateAllButtonClicked () {
	RK_TRACE (DIALOGS);
	QStringList list;
	for (QListViewItem *item = updateable_view->firstChild (); item; item = item->nextSibling ()) {
		list.append (item->text (0));
	}
	updatePackages (list);
}

void UpdatePackagesWidget::getListButtonClicked () {
	RK_TRACE (DIALOGS);
	RCommand *command = new RCommand ("as.vector (old.packages ())", RCommand::App | RCommand::GetStringVector, QString::null, this, FIND_OLD_PACKAGES_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, parent->chain);
	
	get_list_button->setEnabled (false);
	RKCancelDialog::showCancelDialog (i18n ("Fetch list"), i18n ("Please, stand by while downloading the list of packages."), this, this, SIGNAL (actionDone ()), command);
}

void UpdatePackagesWidget::ok () {
	RK_TRACE (DIALOGS);
	deleteThis ();
}

void UpdatePackagesWidget::cancel () {
	RK_TRACE (DIALOGS);
	deleteThis ();
}


/////////////////////// InstallPackagesWidget //////////////////////////

#define FIND_AVAILABLE_PACKAGES_COMMAND 1

InstallPackagesWidget::InstallPackagesWidget (RKLoadLibsDialog *dialog, QWidget *p_widget) : QWidget (p_widget) {
	RK_TRACE (DIALOGS);
	InstallPackagesWidget::parent = dialog;
	
	QVBoxLayout *mvbox = new QVBoxLayout (this, 0, KDialog::spacingHint ());
	QLabel *label = new QLabel (i18n ("Many packages are available on CRAN (Comprehensive R Archive Network), and other repositories (click \"Configure Repositories\" to add more sources). Click \"Fetch List\" to find out, which packages are available. This feature requires a working internet connection."), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	mvbox->addWidget (label);
	QHBoxLayout *hbox = new QHBoxLayout (mvbox, KDialog::spacingHint ());
	
	installable_view = new QListView (this);
	installable_view->addColumn (i18n ("Name"));
	installable_view->addColumn (i18n ("Version"));
	installable_view->setSelectionMode (QListView::Extended);
	hbox->addWidget (installable_view);
	
	QVBoxLayout *buttonvbox = new QVBoxLayout (hbox, KDialog::spacingHint ());
	get_list_button = new QPushButton (i18n ("Fetch list"), this);
	connect (get_list_button, SIGNAL (clicked ()), this, SLOT (getListButtonClicked ()));
	install_selected_button = new QPushButton (i18n ("Install Selected"), this);
	connect (install_selected_button, SIGNAL (clicked ()), this, SLOT (installSelectedButtonClicked ()));
	become_root_box = new QCheckBox (i18n ("As \"root\" user"), this);
	become_root_box->setChecked (true);
	buttonvbox->addWidget (get_list_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (install_selected_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (become_root_box);
	buttonvbox->addStretch (1);
	
	install_selected_button->setEnabled (false);
	installable_view->setEnabled (false);
	//placeholder = new QListViewItem (installable_view, i18n ("[Click \"Fetch list\" to see available packages]"));
	placeholder = new QListViewItem (installable_view, "...");
	
	connect (dialog, SIGNAL (okClicked ()), this, SLOT (ok ()));
	connect (dialog, SIGNAL (cancelClicked ()), this, SLOT (cancel ()));
	connect (this, SIGNAL (destroyed ()), dialog, SLOT (childDeleted ()));
}

InstallPackagesWidget::~InstallPackagesWidget () {
	RK_TRACE (DIALOGS);
	delete placeholder;
}

void InstallPackagesWidget::rCommandDone (RCommand *command) {
	RK_TRACE (DIALOGS);
	if (command->getFlags () == FIND_AVAILABLE_PACKAGES_COMMAND) {
		emit (actionDone ());
		if (!command->failed ()) {
			delete placeholder;
			placeholder = 0;
			RK_ASSERT ((command->stringVectorLength () % 2) == 0);
			int count = (command->stringVectorLength () / 2);
			for (int i=0; i < count; ++i) {
				new QListViewItem (installable_view, command->getStringVector ()[i], command->getStringVector ()[count + i]);
			}
			installable_view->setEnabled (true);

			if (installable_view->firstChild ()) {
				install_selected_button->setEnabled (true);
			} else {
				placeholder = new QListViewItem (installable_view, i18n ("[No packages available]"));
			}
		} else {
			get_list_button->setEnabled (true);
			parent->error_dialog->newError (command->error ());
		}
	} else {
		RK_ASSERT (false);
	}
}

void InstallPackagesWidget::installPackages (const QStringList &list) {
	if (!list.isEmpty ()) {
		if (parent->downloadPackages (list)) parent->installDownloadedPackages (become_root_box->isChecked ());
	}
}

void InstallPackagesWidget::installSelectedButtonClicked () {
	RK_TRACE (DIALOGS);
	QStringList list;
	for (QListViewItem *item = installable_view->firstChild (); item; item = item->nextSibling ()) {
		if (item->isSelected ()) list.append (item->text (0));
	}
	installPackages (list);
}

void InstallPackagesWidget::getListButtonClicked () {
	RK_TRACE (DIALOGS);
	RCommand *command = new RCommand (".rk.get.available.packages ()", RCommand::App | RCommand::GetStringVector, QString::null, this, FIND_AVAILABLE_PACKAGES_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, parent->chain);
	
	get_list_button->setEnabled (false);
	RKCancelDialog::showCancelDialog (i18n ("Fetch list"), i18n ("Please, stand by while downloading packages from CRAN."), this, this, SIGNAL (actionDone ()), command);
}

void InstallPackagesWidget::ok () {
	RK_TRACE (DIALOGS);
	deleteThis ();
}

void InstallPackagesWidget::cancel () {
	RK_TRACE (DIALOGS);
	deleteThis ();
}

#include "rkloadlibsdialog.moc"
