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
#include <qregexp.h>
#include <qtimer.h>

#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettings.h"
#include "../misc/rkprogresscontrol.h"

#include "../debug.h"

#define GET_CURRENT_LIBLOCS_COMMAND 1

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
	layout->addWidget (install_packages_widget = new InstallPackagesWidget (this, page));

	setButtonText (KDialogBase::User1, i18n ("Configure Repositories"));

	num_child_widgets = 3;
	accepted = false;

	RKGlobals::rInterface ()->issueCommand (".libPaths ()", RCommand::App | RCommand::GetStringVector, QString::null, this, GET_CURRENT_LIBLOCS_COMMAND, chain);
}

RKLoadLibsDialog::~RKLoadLibsDialog () {
	RK_TRACE (DIALOGS);

	if (accepted) accept ();
	else reject ();
}

//static
void RKLoadLibsDialog::showInstallPackagesModal (QWidget *parent, RCommandChain *chain, const QString &package_name) {
	RK_TRACE (DIALOGS);

	RKLoadLibsDialog *dialog = new RKLoadLibsDialog (parent, chain, true);
	dialog->auto_install_package = package_name;
	QTimer::singleShot (0, dialog, SLOT (automatedInstall ()));		// to get past the dialog->exec, below
	dialog->showPage (2);
	dialog->exec ();
	RK_TRACE (DIALOGS);
}

void RKLoadLibsDialog::automatedInstall () {
	RK_TRACE (DIALOGS);

	install_packages_widget->getListButtonClicked ();
	install_packages_widget->trySelectPackage (auto_install_package);
}

void RKLoadLibsDialog::tryDestruct () {
	RK_TRACE (DIALOGS);

	if (num_child_widgets <= 0) {
		deleteLater ();
	}
}

void RKLoadLibsDialog::childDeleted () {
	RK_TRACE (DIALOGS);
	--num_child_widgets;
	tryDestruct ();
}

void RKLoadLibsDialog::slotOk () {
	RK_TRACE (DIALOGS);

	accepted = true;
	hide ();
	emit (okClicked ());
}

void RKLoadLibsDialog::slotApply () {
	RK_TRACE (DIALOGS);

	emit (apply ());
}

void RKLoadLibsDialog::slotCancel () {
	RK_TRACE (DIALOGS);
	
	accepted = false;
	hide ();
	emit (cancelClicked ()); // will self-destruct via childDeleted ()
}

void RKLoadLibsDialog::slotUser1 () {
	RK_TRACE (DIALOGS);

	RKSettings::configureSettings (RKSettings::RPackages, this, chain);
}

void RKLoadLibsDialog::closeEvent (QCloseEvent *e) {
	RK_TRACE (DIALOGS);
	e->accept ();
	slotCancel ();
}

void RKLoadLibsDialog::rCommandDone (RCommand *command) {
	RK_TRACE (DIALOGS);
	if (command->getFlags () == GET_CURRENT_LIBLOCS_COMMAND) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		RK_ASSERT (command->getDataLength () > 0);
		QStringList current_library_locations;
		for (unsigned int i=0; i < command->getDataLength (); ++i) {
			current_library_locations.append (command->getStringVector ()[i]);
		}
		emit (libraryLocationsChanged (current_library_locations));
	} else {
		RK_ASSERT (false);
	}
}

bool RKLoadLibsDialog::installPackages (const QStringList &packages, const QString &to_libloc, bool install_dependencies, bool as_root) {
	RK_TRACE (DIALOGS);

	if (packages.isEmpty ()) return false;
	QString command_string = "install.packages (pkgs=c (\"" + packages.join ("\", \"") + "\")" + ", lib=\"" + to_libloc + "\""; 
	if (RKSettingsModuleRPackages::archivePackages ()) command_string += ", destdir=\"" + QDir (RKSettingsModuleGeneral::filesPath ()).filePath ("package_archive") + "\"";
	if (install_dependencies) command_string += ", dependencies=TRUE";
	command_string += ")\n";

	QFile file (QDir (RKSettingsModuleGeneral::filesPath ()).filePath ("install_script.R"));
	if (file.open (IO_WriteOnly)) {
		QTextStream stream (&file);
		stream << "options (repos=" + repos_string + ")\n" + command_string;
		if (as_root) {
			stream << QString ("system (\"chown ") + cuserid (0) + " " + QDir (RKSettingsModuleGeneral::filesPath ()).filePath ("package_archive") + "/*\")\n";
		}
		stream << "q ()\n";
		file.close();
	}

	KProcess *proc = new KProcess;
	if (as_root) *proc << "kdesu" << "-t";
	else *proc << "sh" << "-c";
	*proc << "$R_HOME/bin/R --no-save < " + file.name ();

	connect (proc, SIGNAL (processExited (KProcess *)), this, SLOT (processExited (KProcess *)));
	connect (proc, SIGNAL (receivedStdout (KProcess *, char *, int)), this, SLOT (installationProcessOutput (KProcess *, char *, int)));
	connect (proc, SIGNAL (receivedStderr (KProcess *, char *, int)), this, SLOT (installationProcessError (KProcess *, char *, int)));

	RKProgressControl *installation_progress = new RKProgressControl (this, i18n ("Please stand by while installing selected packages"), i18n ("Installing packages"), RKProgressControl::CancellableProgress);
	connect (this, SIGNAL (installationComplete ()), installation_progress, SLOT (done ()));
	connect (this, SIGNAL (installationOutput (const QString &)), installation_progress, SLOT (newOutput (const QString &)));
	connect (this, SIGNAL (installationError (const QString &)), installation_progress, SLOT (newError (const QString &)));
	proc->start (KProcess::NotifyOnExit, KProcess::AllOutput);

	if (!installation_progress->doModal (true)) proc->kill ();

	file.remove ();
	delete proc;

	return true;
}

void RKLoadLibsDialog::installationProcessOutput (KProcess *, char *buffer, int buflen) {
	RK_TRACE (DIALOGS);
	emit (installationOutput (QCString (buffer, buflen)));
}

void RKLoadLibsDialog::installationProcessError (KProcess *, char *buffer, int buflen) {
	RK_TRACE (DIALOGS);
	emit (installationError (QCString (buffer, buflen)));
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
	QLabel *label = new QLabel (i18n ("There are no safeguards against removing essential packages. For example, unloading \"rkward\" will prevent this application from running properly. Please be careful about the packages you unload."), this);
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
		RK_ASSERT (command->getDataType () == RData::StringVector);
		RK_ASSERT ((command->getDataLength () % 4) == 0);
		int count = (command->getDataLength () / 4);
		for (int i=0; i < count; ++i) {
			new QListViewItem (installed_view, command->getStringVector ()[i], command->getStringVector ()[count + i], command->getStringVector ()[2*count + i], command->getStringVector ()[3* count + i]);
		}
	} else if (command->getFlags () == GET_LOADED_PACKAGES) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		for (unsigned int i=0; i < command->getDataLength (); ++i) {
			new QListViewItem (loaded_view, command->getStringVector ()[i]);
		}
		setEnabled (true);
		updateCurrentList ();
	} else if (command->getFlags () == LOAD_PACKAGE_COMMAND) {
		emit (loadUnloadDone ());
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
	deleteLater ();
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

	RKProgressControl *control = new RKProgressControl (this, i18n ("There has been an error while trying to load / unload packages. See transcript below for details"), i18n ("Error while handling packages"), RKProgressControl::DetailedError);
	connect (this, SIGNAL (loadUnloadDone ()), control, SLOT (done ()));

	// load packages previously not loaded
	QListViewItem *loaded = loaded_view->firstChild ();
	while (loaded) {
		if (!prev_packages.contains (loaded->text (0))) {
			RCommand *command = new RCommand ("library (\"" + loaded->text (0) + "\")", RCommand::App, QString::null, this, LOAD_PACKAGE_COMMAND);
			control->addRCommand (command);
			RKGlobals::rInterface ()->issueCommand (command, parent->chain);
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
			RCommand *command = new RCommand ("detach (package:" + (*it) + ")", RCommand::App, QString::null, this, LOAD_PACKAGE_COMMAND);
			control->addRCommand (command);
			RKGlobals::rInterface ()->issueCommand (command, parent->chain);
		}
	}

	control->doNonModal (true);
}

void LoadUnloadWidget::apply () {
	RK_TRACE (DIALOGS);

	doLoadUnload ();
	updateCurrentList ();
}

void LoadUnloadWidget::cancel () {
	RK_TRACE (DIALOGS);
	deleteLater ();
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
	updateable_view->addColumn (i18n ("Local Version"));
	updateable_view->addColumn (i18n ("Online Version"));
	updateable_view->setSelectionMode (QListView::Extended);
	hbox->addWidget (updateable_view);
	
	QVBoxLayout *buttonvbox = new QVBoxLayout (hbox, KDialog::spacingHint ());
	get_list_button = new QPushButton (i18n ("Fetch list"), this);
	connect (get_list_button, SIGNAL (clicked ()), this, SLOT (getListButtonClicked ()));
	update_selected_button = new QPushButton (i18n ("Update Selected"), this);
	connect (update_selected_button, SIGNAL (clicked ()), this, SLOT (updateSelectedButtonClicked ()));
	update_all_button = new QPushButton (i18n ("Update All"), this);
	connect (update_all_button, SIGNAL (clicked ()), this, SLOT (updateAllButtonClicked ()));
	install_params = new PackageInstallParamsWidget (this, false);
	connect (parent, SIGNAL (libraryLocationsChanged (const QStringList &)), install_params, SLOT (liblocsChanged (const QStringList &)));

	buttonvbox->addWidget (get_list_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (update_selected_button);
	buttonvbox->addWidget (update_all_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (install_params);
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
		if (!command->failed ()) {
			delete placeholder;
			placeholder = 0;
			RK_ASSERT (command->getDataType () == RData::StringVector);
			RK_ASSERT ((command->getDataLength () % 4) == 1);
			unsigned int count = (command->getDataLength () / 4);
			for (unsigned int i=0; i < count; ++i) {
				new QListViewItem (updateable_view, command->getStringVector ()[i], command->getStringVector ()[count + i], command->getStringVector ()[2*count + i], command->getStringVector ()[3*count + i]);
			}

			updateable_view->setEnabled (true);
	
			if (updateable_view->firstChild ()) {
				update_selected_button->setEnabled (true);
				update_all_button->setEnabled (true);
			} else {
				placeholder = new QListViewItem (updateable_view, i18n ("[No updates available]"));
			}

			// this is after the repository was chosen. Update the repository string.
			parent->repos_string = command->getStringVector ()[4 * count];
		} else {
			get_list_button->setEnabled (true);
		}
	} else {
		RK_ASSERT (false);
	}
}

void UpdatePackagesWidget::updatePackages (const QStringList &list) {
	RK_TRACE (DIALOGS);
	bool as_root = false;

	if (list.isEmpty ()) return;
	if (!install_params->checkWritable (&as_root)) return;

	parent->installPackages (list, install_params->libraryLocation (), install_params->installDependencies (), as_root);
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

	get_list_button->setEnabled (false);

	RCommand *command = new RCommand ("c (.rk.get.old.packages (), rk.make.repos.string ())", RCommand::App | RCommand::GetStringVector, QString::null, this, FIND_OLD_PACKAGES_COMMAND);

	RKProgressControl *control = new RKProgressControl (this, i18n ("Please stand by while determining, which package have an update available online."), i18n ("Fetching list"), RKProgressControl::CancellableProgress | RKProgressControl::AutoCancelCommands);
	control->addRCommand (command, true);
	RKGlobals::rInterface ()->issueCommand (command, parent->chain);
	control->doModal (true);
	RKGlobals::rInterface ()->issueCommand ("rm (rk.temp.old)", RCommand::App, QString::null, 0, 0, parent->chain);
}

void UpdatePackagesWidget::ok () {
	RK_TRACE (DIALOGS);
	deleteLater ();
}

void UpdatePackagesWidget::cancel () {
	RK_TRACE (DIALOGS);
	deleteLater ();
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
	install_params = new PackageInstallParamsWidget (this, true);
	connect (parent, SIGNAL (libraryLocationsChanged (const QStringList &)), install_params, SLOT (liblocsChanged (const QStringList &)));

	buttonvbox->addWidget (get_list_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (install_selected_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (install_params);
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
}

void InstallPackagesWidget::rCommandDone (RCommand *command) {
	RK_TRACE (DIALOGS);
	if (command->getFlags () == FIND_AVAILABLE_PACKAGES_COMMAND) {
		if (!command->failed ()) {
			delete placeholder;
			placeholder = 0;
			RK_ASSERT (command->getDataType () == RData::StringVector);
			RK_ASSERT ((command->getDataLength () % 2) == 1);
			unsigned int count = (command->getDataLength () / 2);
			for (unsigned int i=0; i < count; ++i) {
				new QListViewItem (installable_view, command->getStringVector ()[i], command->getStringVector ()[count + i]);
			}
			installable_view->setEnabled (true);

			if (installable_view->firstChild ()) {
				install_selected_button->setEnabled (true);
			} else {
				placeholder = new QListViewItem (installable_view, i18n ("[No packages available]"));
			}

			// this is after the repository was chosen. Update the repository string.
			parent->repos_string = command->getStringVector ()[2 * count];
		} else {
			get_list_button->setEnabled (true);
		}
	} else {
		RK_ASSERT (false);
	}
}

void InstallPackagesWidget::installPackages (const QStringList &list) {
	RK_TRACE (DIALOGS);
	bool as_root = false;

	if (list.isEmpty ()) return;
	if (!install_params->checkWritable (&as_root)) return;

	parent->installPackages (list, install_params->libraryLocation (), install_params->installDependencies (), as_root);
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

	get_list_button->setEnabled (false);

	RCommand *command = new RCommand ("c (.rk.get.available.packages (), rk.make.repos.string ())", RCommand::App | RCommand::GetStringVector, QString::null, this, FIND_AVAILABLE_PACKAGES_COMMAND);
	RKProgressControl *control = new RKProgressControl (this, i18n ("Please stand by while downloading the list of available packages."), i18n ("Fetching list"), RKProgressControl::CancellableProgress | RKProgressControl::AutoCancelCommands);
	control->addRCommand (command, true);
	RKGlobals::rInterface ()->issueCommand (command, parent->chain);
	control->doModal (true);
}

void InstallPackagesWidget::trySelectPackage (const QString &package_name) {
	RK_TRACE (DIALOGS);

	bool found = false;
	for (QListViewItem *item = installable_view->firstChild (); item; item = item->nextSibling ()) {
		if (item->text (0) == package_name) {
			found = true;
			item->setSelected (true);
			installable_view->ensureItemVisible (item);
			break;
		}
	}

	if (!found) {
		KMessageBox::sorry (0, i18n ("The package requested by the backend (\"%1\") was not found in the package repositories. Maybe the package name was mis-spelled. Or maybe you need to add additional repositories via the \"Configure Repositories\"-button.").arg (package_name), i18n ("Package not available"));
	}
}

void InstallPackagesWidget::ok () {
	RK_TRACE (DIALOGS);
	deleteLater ();
}

void InstallPackagesWidget::cancel () {
	RK_TRACE (DIALOGS);
	deleteLater ();
}

/////////////////////// PackageInstallParamsWidget //////////////////////////

#include <qcombobox.h>
#include <qfileinfo.h>
#include <kmessagebox.h>

PackageInstallParamsWidget::PackageInstallParamsWidget (QWidget *parent, bool ask_depends) : QWidget (parent) {
	RK_TRACE (DIALOGS);

	QVBoxLayout *vbox = new QVBoxLayout (this, 0, KDialog::spacingHint ());
	vbox->addWidget (new QLabel (i18n ("Install packages to:"), this));
	libloc_selector = new QComboBox (this);
	vbox->addWidget (libloc_selector);

	if (ask_depends) {
		dependencies = new QCheckBox (i18n ("Include dependencies"), this);
		dependencies->setChecked (true);
		vbox->addStretch ();
		vbox->addWidget (dependencies);
	} else {
		dependencies = 0;
	}
}

PackageInstallParamsWidget::~PackageInstallParamsWidget () {
	RK_TRACE (DIALOGS);
}

bool PackageInstallParamsWidget::installDependencies () {
	RK_TRACE (DIALOGS);

	if (!dependencies) return false;
	return dependencies->isChecked ();
}

QString PackageInstallParamsWidget::libraryLocation () {
	RK_TRACE (DIALOGS);

	return (libloc_selector->currentText ());
}

bool PackageInstallParamsWidget::checkWritable (bool *as_root) {
	RK_TRACE (DIALOGS);
	RK_ASSERT (as_root);

	QFileInfo fi = QFileInfo (libraryLocation ());
	if (!fi.isWritable ()) {
		int res = KMessageBox::questionYesNo (this, i18n ("The directory you are trying to install to (%1) is not writable with your current user permissions. If you are the adminitstrator of this machine, you can try to install the packages as root (you'll be prompted for the root password). Otherwise you'll have to chose a different library location to install to (if none are writable, you will probably want to use the \"Configure Repositories\"-button to set up a writable directory to install packages to).").arg (libraryLocation ()), i18n ("Selected library location not writable"), KGuiItem (i18n ("Become root")), KGuiItem (i18n ("&Cancel")));
		if (res == KMessageBox::Yes) {
			*as_root = true;
			return true;
		}
		return false;
	}

	*as_root = false;
	return true;
}

void PackageInstallParamsWidget::liblocsChanged (const QStringList &newlist) {
	RK_TRACE (DIALOGS);

	libloc_selector->clear ();
	libloc_selector->insertStringList (newlist);
}

#include "rkloadlibsdialog.moc"
