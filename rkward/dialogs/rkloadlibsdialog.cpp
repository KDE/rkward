/***************************************************************************
                          rkloadlibsdialog  -  description
                             -------------------
    begin                : Mon Sep 6 2004
    copyright            : (C) 2004, 2006, 2007, 2008, 2009, 2010, 2011 by Thomas Friedrichsmeier
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
#include <QTreeWidget>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtextstream.h>
#include <QCloseEvent>
#include <QSortFilterProxyModel>
#include <QApplication>
#include <QLineEdit>

#include <klocale.h>
#include <kmessagebox.h>
#include <kvbox.h>
#include <kuser.h>
#include <kstandarddirs.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettings.h"
#include "../core/robjectlist.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkstandardicons.h"

#include "../debug.h"

#include <stdlib.h>


#define GET_CURRENT_LIBLOCS_COMMAND 1

RKLoadLibsDialog::RKLoadLibsDialog (QWidget *parent, RCommandChain *chain, bool modal) : KPageDialog (parent) {
	RK_TRACE (DIALOGS);
	RKLoadLibsDialog::chain = chain;
	installation_process = 0;

	setFaceType (KPageDialog::Tabbed);
	setModal (modal);
	setCaption (i18n ("Configure Packages"));
	setButtons (KDialog::Ok | KDialog::Apply | KDialog::Cancel | KDialog::User1);

	LoadUnloadWidget *luwidget = new LoadUnloadWidget (this);
	addPage (luwidget, i18n ("Local packages"));
	connect (this, SIGNAL (installedPackagesChanged ()), luwidget, SLOT (updateInstalledPackages ()));

	install_packages_widget = new InstallPackagesWidget (this);
	install_packages_pageitem = addPage (install_packages_widget, i18n ("Install / Update / Remove"));

	setButtonText (KDialog::User1, i18n ("Configure Repositories"));

	connect (this, SIGNAL (currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)), this, SLOT (slotPageChanged()));
	QTimer::singleShot (0, this, SLOT (slotPageChanged()));
	num_child_widgets = 3;
	accepted = false;

	RKGlobals::rInterface ()->issueCommand (".libPaths ()", RCommand::App | RCommand::GetStringVector, QString (), this, GET_CURRENT_LIBLOCS_COMMAND, chain);
}

RKLoadLibsDialog::~RKLoadLibsDialog () {
	RK_TRACE (DIALOGS);

	if (accepted) KPageDialog::accept ();
	else KPageDialog::reject ();
}

void RKLoadLibsDialog::slotPageChanged () {
	RK_TRACE (DIALOGS);

	if (!currentPage ()) return;
	QTimer::singleShot (0, currentPage ()->widget (), SLOT (activated()));
}

//static
void RKLoadLibsDialog::showInstallPackagesModal (QWidget *parent, RCommandChain *chain, const QString &package_name) {
	RK_TRACE (DIALOGS);

	RKLoadLibsDialog *dialog = new RKLoadLibsDialog (parent, chain, true);
	dialog->auto_install_package = package_name;
	QTimer::singleShot (0, dialog, SLOT (automatedInstall ()));		// to get past the dialog->exec, below
	dialog->setCurrentPage (dialog->install_packages_pageitem);
	dialog->exec ();
	RK_TRACE (DIALOGS);
}

void RKLoadLibsDialog::automatedInstall () {
	RK_TRACE (DIALOGS);

	install_packages_widget->initialize ();
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

void RKLoadLibsDialog::slotButtonClicked (int button) {
	RK_TRACE (DIALOGS);

	switch (button) {
	case KDialog::Ok:
		accepted = true;
		hide ();
		emit (okClicked ()); // will self-destruct via childDeleted ()
		break;
	case KDialog::Cancel:
		accepted = false;
		hide ();
		emit (cancelClicked ()); // will self-destruct via childDeleted ()
		break;
	case KDialog::Apply:
		emit (applyClicked ());
		break;
	case KDialog::User1:
		RKSettings::configureSettings (RKSettings::PageRPackages, this, chain);
		break;
	}
}

void RKLoadLibsDialog::closeEvent (QCloseEvent *e) {
	RK_TRACE (DIALOGS);
	e->accept ();

	// do as if cancel button was clicked:
	slotButtonClicked (KDialog::Cancel);
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

bool RKLoadLibsDialog::installPackages (const QStringList &packages, const QString &to_libloc, bool install_dependencies, bool as_root, const QStringList& repos) {
	RK_TRACE (DIALOGS);

	if (packages.isEmpty ()) return false;
	QString command_string = "install.packages (pkgs=c (\"" + packages.join ("\", \"") + "\")" + ", lib=\"" + to_libloc + "\"";
	QString downloaddir = QDir (RKSettingsModuleGeneral::filesPath ()).filePath ("package_archive");
	if (RKSettingsModuleRPackages::archivePackages ()) {
		QDir (RKSettingsModuleGeneral::filesPath ()).mkdir ("package_archive");
		command_string += ", destdir=\"" + downloaddir + "\"";
	}
	if (install_dependencies) command_string += ", dependencies=TRUE";
	command_string += ")\n";

	QString repos_string = "options (repos= c(";
	for (int i = 0; i < repos.count (); ++i) {
		if (i) repos_string.append (", ");
		repos_string.append (RObject::rQuote (repos[i]));
	}
	repos_string.append ("))\n");

// TODO: won't work with some versions of GCC (which ones exactly)?
//	QFile file (QDir (RKSettingsModuleGeneral::filesPath ()).filePath ("install_script.R"));
// WORKAROUND:
	QDir dir = RKSettingsModuleGeneral::filesPath ();
	QFile file (dir.filePath ("install_script.R"));
// WORKADOUND END
	if (file.open (QIODevice::WriteOnly)) {
		QTextStream stream (&file);
		stream << repos_string + command_string;
		if (as_root) {
#ifdef Q_WS_WIN
			RK_ASSERT (false);
#else
			KUser user;
			stream << QString ("system (\"chown ") + user.loginName() + ' ' + downloaddir + "/*\")\n";
#endif
		}
		stream << "q ()\n";
		file.close();
	} else {
		RK_ASSERT (false);
	}

	QString R_binary (getenv ("R_binary"));
	QString call;
	QStringList params;
#ifdef Q_WS_WIN
	RK_ASSERT (!as_root);
	call = R_binary;
#else
	if (as_root) {
		call = KStandardDirs::findExe ("kdesu");
		params << "-t" << "--" << R_binary;
	} else {
		call = R_binary;
	}
#endif
	params << "--no-save" << "--no-restore" << "--file=" + file.fileName ();

	installation_process = new QProcess ();
	installation_process->setProcessChannelMode (QProcess::SeparateChannels);

	connect (installation_process, SIGNAL (finished(int,QProcess::ExitStatus)), this, SLOT (processExited(int,QProcess::ExitStatus)));
	connect (installation_process, SIGNAL (readyReadStandardOutput()), this, SLOT (installationProcessOutput()));
	connect (installation_process, SIGNAL (readyReadStandardError()), this, SLOT (installationProcessError()));

	RKProgressControl *installation_progress = new RKProgressControl (this, i18n ("Please stand by while installing selected packages"), i18n ("Installing packages"), RKProgressControl::CancellableProgress);
	connect (this, SIGNAL (installationComplete()), installation_progress, SLOT (done()));
	connect (this, SIGNAL (installationOutput(const QString&)), installation_progress, SLOT (newOutput(const QString&)));
	connect (this, SIGNAL (installationError(const QString&)), installation_progress, SLOT (newError(const QString&)));

	installation_process->start (call, params, QIODevice::ReadWrite | QIODevice::Unbuffered);

	if (!installation_progress->doModal (true)) {
		installation_process->kill ();
		installation_process->waitForFinished (5000);
	}

	file.remove ();
	delete installation_process;
	installation_process = 0;

	emit (installedPackagesChanged ());
	return true;
}

void RKLoadLibsDialog::installationProcessOutput () {
	RK_TRACE (DIALOGS);
	RK_ASSERT (installation_process);

	emit (installationOutput (QString::fromLocal8Bit (installation_process->readAllStandardOutput())));
}

void RKLoadLibsDialog::installationProcessError () {
	RK_TRACE (DIALOGS);
	RK_ASSERT (installation_process);

	emit (installationError (QString::fromLocal8Bit (installation_process->readAllStandardError ())));
}

void RKLoadLibsDialog::processExited (int exitCode, QProcess::ExitStatus exitStatus) {
	RK_TRACE (DIALOGS);

	if (exitCode || (exitStatus != QProcess::NormalExit)) {
		installationError ("\n" + i18n ("Installation process died with exit code %1", exitCode));
	}

	emit (installationComplete ());
}

////////////////////// LoadUnloadWidget ////////////////////////////

#define GET_INSTALLED_PACKAGES 1
#define GET_LOADED_PACKAGES 2
#define LOAD_PACKAGE_COMMAND 3

LoadUnloadWidget::LoadUnloadWidget (RKLoadLibsDialog *dialog) : QWidget (0) {
	RK_TRACE (DIALOGS);
	LoadUnloadWidget::parent = dialog;
	
	QVBoxLayout *mvbox = new QVBoxLayout (this);
	mvbox->setContentsMargins (0, 0, 0, 0);
	
	QHBoxLayout *hbox = new QHBoxLayout ();
	mvbox->addLayout (hbox);
	hbox->setContentsMargins (0, 0, 0, 0);
	QVBoxLayout *instvbox = new QVBoxLayout ();
	hbox->addLayout (instvbox);
	instvbox->setContentsMargins (0, 0, 0, 0);
	QVBoxLayout *buttonvbox = new QVBoxLayout ();
	hbox->addLayout (buttonvbox);
	buttonvbox->setContentsMargins (0, 0, 0, 0);
	QVBoxLayout *loadedvbox = new QVBoxLayout ();
	hbox->addLayout (loadedvbox);
	loadedvbox->setContentsMargins (0, 0, 0, 0);
	
	QLabel *label = new QLabel (i18n ("Installed packages"), this);
	installed_view = new QTreeWidget (this);
	installed_view->setHeaderLabels (QStringList () << i18n ("Name") << i18n ("Title") << i18n ("Version") << i18n ("Location"));
	installed_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
	instvbox->addWidget (label);
	instvbox->addWidget (installed_view);

	load_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionAddRight), i18n ("Load"), this);
	connect (load_button, SIGNAL (clicked ()), this, SLOT (loadButtonClicked ()));
	detach_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionRemoveLeft), i18n ("Unload"), this);
	connect (detach_button, SIGNAL (clicked ()), this, SLOT (detachButtonClicked ()));
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (load_button);
	buttonvbox->addWidget (detach_button);
	buttonvbox->addStretch (2);
	
	label = new QLabel (i18n ("Loaded packages"), this);
	loaded_view = new QTreeWidget (this);
	loaded_view->setHeaderLabel (i18n ("Name"));
	loaded_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
	loadedvbox->addWidget (label);
	loadedvbox->addWidget (loaded_view);

	connect (loaded_view, SIGNAL (itemSelectionChanged()), this, SLOT (updateButtons()));
	connect (installed_view, SIGNAL (itemSelectionChanged()), this, SLOT (updateButtons()));

	connect (dialog, SIGNAL (okClicked ()), this, SLOT (ok ()));
	connect (dialog, SIGNAL (applyClicked ()), this, SLOT (apply ()));
	connect (dialog, SIGNAL (cancelClicked ()), this, SLOT (cancel ()));
	connect (this, SIGNAL (destroyed ()), dialog, SLOT (childDeleted ()));

	updateInstalledPackages ();
	updateButtons ();
}

LoadUnloadWidget::~LoadUnloadWidget () {
	RK_TRACE (DIALOGS);
}

void LoadUnloadWidget::activated () {
	RK_TRACE (DIALOGS);

	installed_view->setFocus ();
}

void LoadUnloadWidget::rCommandDone (RCommand *command) {
	RK_TRACE (DIALOGS);
	if (command->failed ()) return;
	if (command->getFlags () == GET_INSTALLED_PACKAGES) {
		RK_ASSERT (command->getDataLength () == 6);

		RData *package = command->getStructureVector ()[0];
		RData *title = command->getStructureVector ()[1];
		RData *version = command->getStructureVector ()[2];
		RData *libpath = command->getStructureVector ()[3];

		unsigned int count = package->getDataLength ();
		RK_ASSERT (count == title->getDataLength ());
		RK_ASSERT (count == version->getDataLength ());
		RK_ASSERT (count == libpath->getDataLength ());
		for (unsigned int i=0; i < count; ++i) {
			QTreeWidgetItem* item = new QTreeWidgetItem (installed_view);
			item->setText (0, package->getStringVector ()[i]);
			item->setText (1, title->getStringVector ()[i]);
			item->setText (2, version->getStringVector ()[i]);
			item->setText (3, libpath->getStringVector ()[i]);
		}
		installed_view->resizeColumnToContents (0);
		installed_view->setSortingEnabled (true);
		installed_view->sortItems (0, Qt::AscendingOrder);
	} else if (command->getFlags () == GET_LOADED_PACKAGES) {
		RK_ASSERT (command->getDataType () == RData::StringVector);

		for (unsigned int i=0; i < command->getDataLength (); ++i) {
			QTreeWidgetItem* item = new QTreeWidgetItem (loaded_view);
			item->setText (0, command->getStringVector ()[i]);
			if (RKSettingsModuleRPackages::essentialPackages ().contains (command->getStringVector ()[i])) {
#if QT_VERSION >= 0x040400
				item->setFlags (Qt::NoItemFlags);
#else
				item->setFlags (0);
#endif
			}
		}
		loaded_view->resizeColumnToContents (0);
		loaded_view->setSortingEnabled (true);
		loaded_view->sortItems (0, Qt::AscendingOrder);
		updateCurrentList ();
	} else if (command->getFlags () == LOAD_PACKAGE_COMMAND) {
		emit (loadUnloadDone ());
	} else {
		RK_ASSERT (false);
	}
}

void LoadUnloadWidget::updateInstalledPackages () {
	RK_TRACE (DIALOGS);

	installed_view->clear ();
	loaded_view->clear ();

	RCommand *c = new RCommand (".rk.get.installed.packages ()", RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString::null, this, GET_INSTALLED_PACKAGES);
	connect (c->notifier (), SIGNAL (commandFinished(RCommand*)), RKGlobals::rInterface (), SLOT (installedPackagesCommandFinished(RCommand*)));
	RKGlobals::rInterface ()->issueCommand (c, parent->chain);
	RKGlobals::rInterface ()->issueCommand (".packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_LOADED_PACKAGES, parent->chain);
}

void LoadUnloadWidget::loadButtonClicked () {
	RK_TRACE (DIALOGS);

	loaded_view->clearSelection ();
	QList<QTreeWidgetItem*> sel = installed_view->selectedItems ();
	for (int i = 0; i < sel.size (); ++i) {
		QString package_name = sel[i]->text (0);

		// is this package already loaded?
		QList<QTreeWidgetItem*> loaded = loaded_view->findItems (package_name, Qt::MatchExactly, 0);
		if (loaded.isEmpty ()) {
			QTreeWidgetItem* item = new QTreeWidgetItem (loaded_view);
			item->setText (0, package_name);
			item->setSelected (true);
		}
	}
}

void LoadUnloadWidget::detachButtonClicked () {
	RK_TRACE (DIALOGS);

	installed_view->clearSelection ();
	QList<QTreeWidgetItem*> sel = loaded_view->selectedItems ();
	for (int i = 0; i < sel.size (); ++i) {
		QString package_name = sel[i]->text (0);

		delete (sel[i]);	// remove from list of loaded packages

		// select corresponding package in list of available packages
		QList<QTreeWidgetItem*> installed = installed_view->findItems (package_name, Qt::MatchExactly, 0);
		if (!installed.isEmpty ()) {
			//RK_ASSERT (installed.count () == 1); // In fact, several versions of one package can be installed in several library locations
			installed[0]->setSelected (true);
		}
	}
}

void LoadUnloadWidget::updateButtons () {
	RK_TRACE (DIALOGS);

	detach_button->setEnabled (!loaded_view->selectedItems ().isEmpty ());
	load_button->setEnabled (!installed_view->selectedItems ().isEmpty ());
}

void LoadUnloadWidget::ok () {
	RK_TRACE (DIALOGS);
	doLoadUnload ();
	deleteLater ();
}

void LoadUnloadWidget::updateCurrentList () {
	RK_TRACE (DIALOGS);

	prev_packages.clear ();
	for (int i = 0; i < loaded_view->topLevelItemCount (); ++i) {
		prev_packages.append (loaded_view->topLevelItem (i)->text (0));
	}
}

void LoadUnloadWidget::doLoadUnload () {
	RK_TRACE (DIALOGS);

	RKProgressControl *control = new RKProgressControl (this, i18n ("There has been an error while trying to load / unload packages. See transcript below for details"), i18n ("Error while handling packages"), RKProgressControl::DetailedError);
	connect (this, SIGNAL (loadUnloadDone ()), control, SLOT (done ()));

	// load packages previously not loaded
	for (int i = 0; i < loaded_view->topLevelItemCount (); ++i) {
		QTreeWidgetItem* loaded = loaded_view->topLevelItem (i);
		if (!prev_packages.contains (loaded->text (0))) {
			RCommand *command = new RCommand ("library (\"" + loaded->text (0) + "\")", RCommand::App | RCommand::ObjectListUpdate);
			control->addRCommand (command);
			RKGlobals::rInterface ()->issueCommand (command, parent->chain);
		}
	}
	
	// detach packages previously attached
	QStringList packages_to_remove;
	for (QStringList::Iterator it = prev_packages.begin (); it != prev_packages.end (); ++it) {
		QList<QTreeWidgetItem*> loaded = loaded_view->findItems ((*it), Qt::MatchExactly, 0);
		if (loaded.isEmpty ()) {	// no longer in the list
			packages_to_remove.append ("package:" + *it);
		}
	}
	if (!packages_to_remove.isEmpty ()) {
		QStringList messages = RObjectList::getObjectList ()->detachPackages (packages_to_remove, parent->chain, control);
		if (!messages.isEmpty ()) KMessageBox::sorry (this, messages.join ("\n"));
	}

	// find out, when we're done
	RCommand *command = new RCommand (QString (), RCommand::EmptyCommand, QString (), this, LOAD_PACKAGE_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, parent->chain);

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


/////////////////////// InstallPackagesWidget //////////////////////////

InstallPackagesWidget::InstallPackagesWidget (RKLoadLibsDialog *dialog) : QWidget (0) {
	RK_TRACE (DIALOGS);
	InstallPackagesWidget::parent = dialog;
	
	QVBoxLayout *mvbox = new QVBoxLayout (this);
	mvbox->setContentsMargins (0, 0, 0, 0);
	QLabel *label = new QLabel (i18n ("Many packages are available on CRAN (Comprehensive R Archive Network), and other repositories<br>(click \"Configure Repositories\" to add more sources)."), this);
	mvbox->addWidget (label);
	QHBoxLayout *hbox = new QHBoxLayout ();
	mvbox->addLayout (hbox);
	hbox->setContentsMargins (0, 0, 0, 0);

	packages_status = new RKRPackageInstallationStatus (this);
	packages_view = new QTreeView (this);
	packages_view->setSortingEnabled (true);
	model = new RKRPackageInstallationStatusSortFilterModel (this);
	model->setSourceModel (packages_status);
	model->setFilterCaseSensitivity (Qt::CaseInsensitive);
	model->setSortCaseSensitivity (Qt::CaseInsensitive);
	packages_view->setModel (model);
	packages_view->setEnabled (false);
	packages_view->setMinimumHeight (packages_view->sizeHintForRow (0) * 15);	// force a decent height
	hbox->addWidget (packages_view);

	QVBoxLayout *buttonvbox = new QVBoxLayout ();
	hbox->addLayout (buttonvbox);
	buttonvbox->setContentsMargins (0, 0, 0, 0);
	label = new QLabel (i18n ("Show only packages matching:"), this);
	filter_edit = new QLineEdit ("*", this);
	connect (filter_edit, SIGNAL (textChanged(const QString&)), this, SLOT (filterStringChanged(const QString&)));
	filterStringChanged (filter_edit->text ());

	mark_all_updates_button = new QPushButton (i18n ("Select all updates"), this);
	connect (mark_all_updates_button, SIGNAL (clicked()), this, SLOT (markAllUpdates()));

	install_params = new PackageInstallParamsWidget (this, true);
	connect (parent, SIGNAL (libraryLocationsChanged (const QStringList &)), install_params, SLOT (liblocsChanged (const QStringList &)));

	buttonvbox->addWidget (label);
	buttonvbox->addWidget (filter_edit);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (mark_all_updates_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (install_params);
	buttonvbox->addStretch (1);

	connect (dialog, SIGNAL (okClicked ()), this, SLOT (ok ()));
	connect (dialog, SIGNAL (applyClicked ()), this, SLOT (apply ()));
	connect (dialog, SIGNAL (cancelClicked ()), this, SLOT (cancel ()));
	connect (this, SIGNAL (destroyed ()), dialog, SLOT (childDeleted ()));
}

InstallPackagesWidget::~InstallPackagesWidget () {
	RK_TRACE (DIALOGS);
}

void InstallPackagesWidget::activated () {
	RK_TRACE (DIALOGS);

	filter_edit->setFocus ();
	if (!packages_status->initialized ()) initialize ();
}

void InstallPackagesWidget::installPackages (const QStringList &list) {
	RK_TRACE (DIALOGS);
	bool as_root = false;

	if (list.isEmpty ()) return;
	if (!install_params->checkWritable (&as_root)) return;

	parent->installPackages (list, install_params->libraryLocation (), install_params->installDependencies (), as_root, packages_status->currentRepositories ());
}

void InstallPackagesWidget::initialize () {
	RK_TRACE (DIALOGS);

	packages_status->initialize (parent->chain);
	packages_view->setEnabled (true);
	for (int i = 0; i <= RKRPackageInstallationStatus::PackageName; ++i) packages_view->resizeColumnToContents (i);
}

void InstallPackagesWidget::filterStringChanged (const QString& new_filter) {
	RK_TRACE (DIALOGS);

	QString f = new_filter;
	if (f.isEmpty ()) f = "*";
	model->setFilterWildcard (f);
}

void InstallPackagesWidget::trySelectPackage (const QString &package_name) {
	RK_TRACE (DIALOGS);

	QModelIndex index = packages_status->markPackageForInstallation (package_name);
	if (!index.isValid ()) {
		KMessageBox::sorry (0, i18n ("The package requested by the backend (\"%1\") was not found in the package repositories. Maybe the package name was mis-spelled. Or maybe you need to add additional repositories via the \"Configure Repositories\"-button.", package_name), i18n ("Package not available"));
	} else {
		packages_view->scrollTo (model->mapFromSource (index));
	}
}

void InstallPackagesWidget::markAllUpdates () {
	RK_TRACE (DIALOGS);

	QModelIndex index = packages_status->markAllUpdatesForInstallation ();
	packages_view->setExpanded (model->mapFromSource (index), true);
	packages_view->scrollTo (model->mapFromSource (index));
}

void InstallPackagesWidget::doInstall () {
	RK_TRACE (DIALOGS);

	installPackages (packages_status->packagesToInstall ());
	packages_status->clearStatus ();
}

void InstallPackagesWidget::apply () {
	RK_TRACE (DIALOGS);

	doInstall ();
	initialize ();	// refresh list
}

void InstallPackagesWidget::ok () {
	RK_TRACE (DIALOGS);

	doInstall ();
	deleteLater ();
}

void InstallPackagesWidget::cancel () {
	RK_TRACE (DIALOGS);
	deleteLater ();
}

/////////////////////// PackageInstallParamsWidget //////////////////////////

#include <qcombobox.h>
#include <qfileinfo.h>

PackageInstallParamsWidget::PackageInstallParamsWidget (QWidget *parent, bool ask_depends) : QWidget (parent) {
	RK_TRACE (DIALOGS);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
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
		QString mcaption = i18n ("Selected library location not writable");
		QString message = i18n ("The directory you are trying to install to (%1) is not writable with your current user permissions. "
			"You can chose a different library location to install to (if none are wirtable, you will probably want to use the "
			"\"Configure Repositories\"-button to set up a writable directory to install package to).\n", libraryLocation ());
#ifdef Q_WS_WIN
		message.append (i18n ("If have access to an administrator account on this machine, you can use that to install the package(s), or "
			"you could change the permissions of '%1'. Sorry, automatic switching to Administrator is not yet supported in RKWard on Windows", libraryLocation ()));
		int res = KMessageBox::warningContinueCancel (this, message, mcaption, KGuiItem (i18n ("Attempt installation, anyway")));
		if (res == KMessageBox::Continue) return true;
#else
		message.append (i18n ("If you are the administrator of this machine, you can try to install the packages as root (you'll be prompted for the root password)."));
		int res = KMessageBox::warningContinueCancel (this, message, mcaption, KGuiItem (i18n ("Become root")));
		if (res == KMessageBox::Continue) {
			*as_root = true;
			return true;
		}
#endif
		return false;
	}

	*as_root = false;
	return true;
}

void PackageInstallParamsWidget::liblocsChanged (const QStringList &newlist) {
	RK_TRACE (DIALOGS);

	libloc_selector->clear ();
	libloc_selector->insertItems (0, newlist);
}

/////////// RKRPackageInstallationStatus /////////////////

RKRPackageInstallationStatus::RKRPackageInstallationStatus (QObject* parent) : QAbstractItemModel (parent) {
	RK_TRACE (DIALOGS);
	_initialized = false;
}

RKRPackageInstallationStatus::~RKRPackageInstallationStatus () {
	RK_TRACE (DIALOGS);
}

QModelIndex RKRPackageInstallationStatus::markAllUpdatesForInstallation () {
	RK_TRACE (DIALOGS);

	// inefficient, but so what...
	for (int i = updateable_packages_in_installed.count (); i >= 0; --i) {
		markPackageForInstallation (installed_packages[updateable_packages_in_installed[i]]);
	}
	return index (UpdateablePackages, 0, QModelIndex ());
}

QModelIndex RKRPackageInstallationStatus::markPackageForInstallation (const QString& package_name) {
	RK_TRACE (DIALOGS);

	// is the package available at all?
	QModelIndex pindex;
	int row = available_packages.indexOf (package_name);
	if (row < 0) return pindex;

	// find out, whether it is a new or and updateable package
	QModelIndex parent;
	int urow = updateable_packages_in_available.indexOf (row);
	if (urow >= 0) {
		parent = index (UpdateablePackages, 0);
		row = urow;
	} else {
		row = new_packages_in_available.indexOf (row);
		parent = index (NewPackages, 0);
	}
	if (row < 0) {
		RK_ASSERT (false);
		return pindex;
	}

	// mark for installation
	pindex = index (row, InstallationStatus, parent);
	setData (pindex, QVariant (Qt::Checked), Qt::CheckStateRole);
	return pindex;
}

void RKRPackageInstallationStatus::initialize (RCommandChain *chain) {
	RK_TRACE (DIALOGS);

	_initialized = true;	// will be re-set to false, should the command fail / be cancelled

	RCommand *command = new RCommand (".rk.get.package.intallation.state ()", RCommand::App | RCommand::GetStructuredData);
	connect (command->notifier (), SIGNAL (commandFinished(RCommand*)), this, SLOT (statusCommandFinished(RCommand*)));
	RKProgressControl *control = new RKProgressControl (this, i18n ("<p>Please stand by while searching for installed and available packages.</p><p><strong>Note:</strong> This requires a working internet connection.</p>"), i18n ("Searching for packages"), RKProgressControl::CancellableProgress | RKProgressControl::AutoCancelCommands);
	control->addRCommand (command, true);
	RKGlobals::rInterface ()->issueCommand (command, chain);
	control->doModal (true);
}

void RKRPackageInstallationStatus::statusCommandFinished (RCommand *command) {
	RK_TRACE (DIALOGS);

	if (!command->succeeded ()) {
		RK_ASSERT (false);
		_initialized = false;
		return;
	}
	RK_ASSERT (command->getDataType () == RCommand::StructureVector);
	RK_ASSERT (command->getDataLength () == 4);

	RData::RDataStorage top = command->getStructureVector ();
	RData::RDataStorage available = top[0]->getStructureVector ();
	available_packages = available[0]->getStringVector ();
	available_titles = available[1]->getStringVector ();
	available_versions = available[2]->getStringVector ();
	available_repos = available[3]->getStringVector ();
	enhance_rk_in_available = available[4]->getIntVector ();

	RData::RDataStorage installed = top[1]->getStructureVector ();
	installed_packages = installed[0]->getStringVector ();
	installed_titles = installed[1]->getStringVector ();
	installed_versions = installed[2]->getStringVector ();
	installed_libpaths = installed[3]->getStringVector ();
	enhance_rk_in_installed = installed[4]->getIntVector ();
	installed_has_update.fill (false, installed_packages.count ());

	new_packages_in_available = top[2]->getIntVector ();
	RData::RDataStorage updateable = top[3]->getStructureVector ();
	updateable_packages_in_installed = updateable[0]->getIntVector ();
	updateable_packages_in_available = updateable[1]->getIntVector ();

	for (int i = updateable_packages_in_installed.count () - 1; i >= 0; --i) {
		installed_has_update[updateable_packages_in_installed[i]] = true;
	}

	current_repos = top[4]->getStringVector ();

	clearStatus ();
}

void RKRPackageInstallationStatus::clearStatus () {
	RK_TRACE (DIALOGS);

	available_status.fill (NoAction, available_packages.count ());
	installed_status.fill (NoAction, installed_packages.count ());
	reset ();
}

QVariant RKRPackageInstallationStatus::headerData (int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal) return QVariant ();

	if ((role == Qt::DecorationRole) && (section == EnhancesRKWard)) return QApplication::windowIcon ();

	if (role == Qt::DisplayRole) {
		if (section == InstallationStatus) return QVariant (i18n ("Status"));
		if (section == PackageName) return QVariant (i18n ("Name"));
		if (section == PackageTitle) return QVariant (i18n ("Title"));
		if (section == Version) return QVariant (i18n ("Version"));
		if (section == Location) return QVariant (i18n ("Location"));
	}
	return QVariant ();
}

int RKRPackageInstallationStatus::rowCount (const QModelIndex &parent) const {
	if (!parent.isValid ()) return TOPLEVELITEM_COUNT;	// top level
	if (parent.parent ().isValid ()) return 0;			// model has exactly two levels

	int row = parent.row ();
	if (row == UpdateablePackages) return updateable_packages_in_available.count ();
	if (row == NewPackages) return new_packages_in_available.count ();
	if (row == InstalledPackages) return installed_packages.count ();

	RK_ASSERT (false);
	return 0;
}

QVariant RKRPackageInstallationStatus::data (const QModelIndex &index, int role) const {
	if (!index.isValid ()) return QVariant ();
	if (!index.parent ().isValid ()) {	// top level item
		int row = index.row ();
		if (row == UpdateablePackages) {
			if ((role == Qt::DisplayRole) && (index.column () == PackageName)) return QVariant (i18n ("Updateable Packages"));
			if (role == Qt::ToolTipRole) return QVariant (i18n ("Packages for which an update is available. This may include packages which were merely built against a newer version of R."));
		} else if (row == NewPackages) {
			if ((role == Qt::DisplayRole) && (index.column () == PackageName)) return QVariant (i18n ("New Packages"));
			if (role == Qt::ToolTipRole) return QVariant (i18n ("Packages for which available for installation, but which are not currently installed."));
		} else if (row == InstalledPackages) {
			if ((role == Qt::DisplayRole) && (index.column () == PackageName)) return QVariant (i18n ("Installed Packages"));
			if (role == Qt::ToolTipRole) return QVariant (i18n ("Packages for which installed locally. Note that updates may be available for these packages."));
		}
		if (role == Qt::BackgroundColorRole) return QVariant (QColor (200, 200, 200));
	} else if (!index.parent ().parent ().isValid ()) {		// model has exactly two levels
		int col = index.column ();
		int prow = index.parent ().row ();
		int arow, irow;		// row numbers in the lists of available_packages / installed_packages
		if (prow == UpdateablePackages) {
			arow = updateable_packages_in_available.value (index.row ());
			irow = updateable_packages_in_installed.value (index.row ());
		} else if (prow == NewPackages) arow = new_packages_in_available.value (index.row ());
		else irow = index.row ();

		if (col == InstallationStatus) {
			PackageStatusChange stat;
			if (prow == InstalledPackages) stat = installed_status.value (irow, NoAction);
			else stat = available_status.value (arow, NoAction);
			if (stat == NoAction) {
				if (role == Qt::CheckStateRole) {
					if (prow == InstalledPackages) return Qt::PartiallyChecked;
					return Qt::Unchecked;
				}
			} else if (stat == Install) {
				if (role == Qt::CheckStateRole) return Qt::Checked;
				if (role == Qt::DisplayRole) return QVariant (i18n ("Install"));
			} else {
				if (role == Qt::CheckStateRole) return Qt::Unchecked;
				if (role == Qt::DisplayRole) return QVariant (i18n ("Remove"));
			}
		} else if (col == EnhancesRKWard) {
			if (role == Qt::DisplayRole) return QVariant (QString (" "));	// must have a placeholder, here, or Qt will collapse the column
			if ((role == Qt::DecorationRole) || (role == Qt::UserRole)) {
				bool enhance_rk;
				if (prow == InstalledPackages) enhance_rk = enhance_rk_in_installed.value (irow);
				else enhance_rk = enhance_rk_in_available.value (arow);
				if (role == Qt::UserRole) return QVariant (enhance_rk);
				if (enhance_rk) return QApplication::windowIcon ();
			}
		} else if (col == PackageName) {
			if (role == Qt::DisplayRole) {
				if (prow == InstalledPackages) return installed_packages.value (irow);
				else return available_packages.value (arow);
			}
		} else if (col == PackageTitle) {
			if (role == Qt::DisplayRole) {
				if (prow == InstalledPackages) return installed_titles.value (irow);
				else return available_titles.value (arow);
			}
		} else if (col == Version) {
			if (role == Qt::DisplayRole) {
				if (prow == InstalledPackages) return installed_versions.value (irow);
				else if (prow == NewPackages) return available_versions.value (arow);
				else return QVariant (installed_versions.value (irow) + " -> " + available_versions.value (arow));
			}
		} else if (col == Location) {
			if (role == Qt::DisplayRole) {
				if (prow == InstalledPackages) return installed_libpaths.value (irow);
				else if (prow == NewPackages) return available_repos.value (arow);
				else return QVariant (installed_libpaths.value (irow) + " -> " + available_repos.value (arow));
			}
		}
	}
	return QVariant ();
}

Qt::ItemFlags RKRPackageInstallationStatus::flags (const QModelIndex &index) const {
	qint64 pos = index.internalId ();
	Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	if (pos >= 0) flags |= Qt::ItemIsUserCheckable;
	if (pos == InstalledPackages) flags |= Qt::ItemIsTristate;
	return flags;
}

QModelIndex RKRPackageInstallationStatus::index (int row, int column, const QModelIndex &parent) const {
	if (!parent.isValid ()) return createIndex (row, column, -1);	// toplevel items
	return createIndex (row, column, parent.row ());				// parent.row () identifies, which toplevel item is the parent.
}

QModelIndex RKRPackageInstallationStatus::parent (const QModelIndex& index) const {
	if (index.internalId () == -1) return QModelIndex ();
	return (RKRPackageInstallationStatus::index (index.internalId (), 0, QModelIndex ()));
}

bool RKRPackageInstallationStatus::setData (const QModelIndex &index, const QVariant &value, int role) {
	RK_TRACE (DIALOGS);

	if (role != Qt::CheckStateRole) return false;
	if (!index.isValid ()) return false;
	if (!index.parent ().isValid ()) return false;
	QModelIndex bindex;

	PackageStatusChange stat = NoAction;
	int irow = -1;
	int arow = -1;
	if (value.toInt () == Qt::Checked) stat = Install;
	else if (value.toInt () == Qt::Unchecked) stat = Remove;

	if (index.internalId () == InstalledPackages) {
		irow = index.row ();
		if ((stat == Install) && (installed_status[irow] == Remove)) stat = NoAction;
		if (installed_has_update.value (irow, false)) {
			// NOTE: installed, and updatable packages are coupled
			int urow = updateable_packages_in_installed.indexOf (irow);
			RK_ASSERT (urow >= 0);
			arow = updateable_packages_in_available.value (urow);
			bindex = RKRPackageInstallationStatus::index (urow, InstallationStatus, RKRPackageInstallationStatus::index (UpdateablePackages, 0));
		}
	} else {
		if (stat == Remove) stat = NoAction;
		if (index.internalId () == UpdateablePackages) {
			// NOTE: installed, and updatable packages are coupled
			irow = updateable_packages_in_installed.value (index.row ());
			arow = updateable_packages_in_available.value (index.row ());
			bindex = RKRPackageInstallationStatus::index (irow, InstallationStatus, RKRPackageInstallationStatus::index (InstalledPackages, 0));
		} else {
			arow = new_packages_in_available.value (index.row ());
		}
	}

	if (irow >= 0) installed_status[irow] = stat;
	if (arow >= 0) available_status[arow] = stat;

	dataChanged (index, index);
	if (bindex.isValid ()) dataChanged (bindex, bindex);

	return true;
}

QStringList RKRPackageInstallationStatus::packagesToInstall () const {
	RK_TRACE (DIALOGS);

	QStringList ret;
	for (int i = installed_status.count () - 1; i >= 0; --i) {
		if (installed_status[i] == Install) ret.append (installed_packages[i]);
	}
	for (int i = available_status.count () - 1; i >= 0; --i) {
		if (available_status[i] == Install) {
			QString package = available_packages[i];
			if (!ret.contains (package)) ret.append (package);
		}
	}
	return ret;
}

bool RKRPackageInstallationStatus::packagesToRemove (QStringList *packages, QStringList *liblocs) {
	RK_TRACE (DIALOGS);

	bool anyfound = false;
	for (int i = installed_status.count () - 1; i >= 0; --i) {
		if (installed_status[i] == Remove) {
			packages->append (installed_packages[i]);
			liblocs->append (installed_libpaths[i]);
			anyfound = true;
		}
	}
	return anyfound;
}


RKRPackageInstallationStatusSortFilterModel::RKRPackageInstallationStatusSortFilterModel (QObject* parent) : QSortFilterProxyModel (parent) {
	RK_TRACE (DIALOGS);
}

RKRPackageInstallationStatusSortFilterModel::~RKRPackageInstallationStatusSortFilterModel () {
	RK_TRACE (DIALOGS);
}

bool RKRPackageInstallationStatusSortFilterModel::lessThan (const QModelIndex &left, const QModelIndex &right) const {
	if (!left.parent ().isValid ()) {		// Disable sorting for the top level items
		return (left.row () > right.row ());
	}
	if (left.column () == RKRPackageInstallationStatus::EnhancesRKWard) {
		return (!left.data (Qt::UserRole).toBool ());
	}
	return QSortFilterProxyModel::lessThan (left, right);
}

bool RKRPackageInstallationStatusSortFilterModel::filterAcceptsRow (int source_row, const QModelIndex &source_parent) const {
	if (!source_parent.isValid ()) return true;		// Never filter the top level item

// filter on Name and Title
	QString name = source_parent.child (source_row, RKRPackageInstallationStatus::PackageName).data ().toString ();
	if (filterRegExp ().exactMatch (name)) return true;
	QString title = source_parent.child (source_row, RKRPackageInstallationStatus::PackageTitle).data ().toString ();
	return (filterRegExp ().exactMatch (title));
}

#include "rkloadlibsdialog.moc"
