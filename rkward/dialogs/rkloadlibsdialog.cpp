/***************************************************************************
                          rkloadlibsdialog  -  description
                             -------------------
    begin                : Mon Sep 6 2004
    copyright            : (C) 2004 - 2020 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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
#include <QStandardPaths>

#include <KLocalizedString>
#include <kmessagebox.h>
#include <kuser.h>

#include "../rkglobals.h"
#include "../rbackend/rkrinterface.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettings.h"
#include "../core/robjectlist.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkdynamicsearchline.h"

#include "../debug.h"

#include <stdlib.h>


#define GET_CURRENT_LIBLOCS_COMMAND 1

RKLoadLibsDialog::RKLoadLibsDialog (QWidget *parent, RCommandChain *chain, bool modal) : KPageDialog (parent) {
	RK_TRACE (DIALOGS);
	RKLoadLibsDialog::chain = chain;
	installation_process = 0;

	setFaceType (KPageDialog::Tabbed);
	setModal (modal);
	setWindowTitle (i18n ("Configure Packages"));
	setStandardButtons (QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);

	LoadUnloadWidget *luwidget = new LoadUnloadWidget (this);
	addChild (luwidget, i18n ("Load / Unload R packages"));
	connect (this, &RKLoadLibsDialog::installedPackagesChanged, luwidget, &LoadUnloadWidget::updateInstalledPackages);

	install_packages_widget = new InstallPackagesWidget (this);
	install_packages_pageitem = addChild (install_packages_widget, i18n ("Install / Update / Remove R packages"));

	configure_pluginmaps_pageitem = addChild (new RKPluginMapSelectionWidget (this), i18n ("Manage RKWard Plugins"));

	connect (this, &KPageDialog::currentPageChanged, this, &RKLoadLibsDialog::slotPageChanged);
	QTimer::singleShot (0, this, SLOT (slotPageChanged()));
	num_child_widgets = 4;
	was_accepted = false;

	RKGlobals::rInterface ()->issueCommand (".libPaths ()", RCommand::App | RCommand::GetStringVector, QString (), this, GET_CURRENT_LIBLOCS_COMMAND, chain);
}

RKLoadLibsDialog::~RKLoadLibsDialog () {
	RK_TRACE (DIALOGS);

	if (was_accepted) KPageDialog::accept ();
	else KPageDialog::reject ();
}

KPageWidgetItem* RKLoadLibsDialog::addChild (QWidget *child_page, const QString &caption) {
	RK_TRACE (DIALOGS);

	// TODO: Can't convert these signal/slot connections to new syntax, without creating a common base class for the child pages
	connect (this, SIGNAL (accepted()), child_page, SLOT (ok()));
	connect (button (QDialogButtonBox::Apply), SIGNAL (clicked(bool)), child_page, SLOT (apply()));
	connect (this, SIGNAL(rejected()), child_page, SLOT (cancel()));
	connect (child_page, &QObject::destroyed, this, &RKLoadLibsDialog::childDeleted);
	return addPage (child_page, caption);
}

void RKLoadLibsDialog::slotPageChanged () {
	RK_TRACE (DIALOGS);

	if (!currentPage ()) return;
	QTimer::singleShot (0, currentPage ()->widget (), SLOT (activated()));
}

//static
void RKLoadLibsDialog::showInstallPackagesModal(QWidget *parent, RCommandChain *chain, const QStringList &package_names) {
	RK_TRACE(DIALOGS);

	RKLoadLibsDialog *dialog = new RKLoadLibsDialog(parent, chain, true);
	QTimer::singleShot(0, dialog, [dialog, package_names]() { dialog->automatedInstall(package_names); });   // to get past the dialog->exec, below
	dialog->setCurrentPage(dialog->install_packages_pageitem);
	dialog->exec();
	RK_TRACE(DIALOGS);
}

// static
void RKLoadLibsDialog::showPluginmapConfig (QWidget* parent, RCommandChain* chain) {
	RK_TRACE (DIALOGS);

	RKLoadLibsDialog *dialog = new RKLoadLibsDialog (parent, chain, false);
	dialog->setCurrentPage (dialog->configure_pluginmaps_pageitem);
	dialog->show ();
}

void RKLoadLibsDialog::automatedInstall(const QStringList &packages) {
	RK_TRACE (DIALOGS);

	install_packages_widget->initialize();
	install_packages_widget->trySelectPackages(packages);
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

void RKLoadLibsDialog::closeEvent (QCloseEvent *e) {
	RK_TRACE (DIALOGS);
	e->accept ();

	// do as if cancel button was clicked:
	reject ();
}

void RKLoadLibsDialog::accept () {
	was_accepted = true;
	hide ();
	// will self-destruct once all children are done
	emit accepted();
}

void RKLoadLibsDialog::reject () {
	was_accepted = false;
	hide ();
	// will self-destruct once all children are done
	emit rejected();
}

void RKLoadLibsDialog::rCommandDone (RCommand *command) {
	RK_TRACE (DIALOGS);
	if (command->getFlags () == GET_CURRENT_LIBLOCS_COMMAND) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		RK_ASSERT (command->getDataLength () > 0);
		// NOTE: The problem is that e.g. R_LIBS_USER is not in .libPaths() if it does not exist, yet. But it should be available as an option, of course
		library_locations = command->stringVector ();
		emit libraryLocationsChanged(library_locations);
	} else {
		RK_ASSERT (false);
	}
}

void RKLoadLibsDialog::addLibraryLocation (const QString& new_loc) {
	RK_TRACE (DIALOGS);

	if (library_locations.contains (new_loc)) return;

	if (QDir ().mkpath (new_loc)) RKSettingsModuleRPackages::addLibraryLocation (new_loc, chain);
	library_locations.prepend (new_loc);
	emit libraryLocationsChanged(library_locations);
}

bool RKLoadLibsDialog::removePackages (QStringList packages, QStringList from_liblocs) {
	RK_TRACE (DIALOGS);

	bool as_root = false;
	QStringList not_removing;
	QStringList not_writable;
	QList<int> not_writable_int;
	for (int i = 0; i < packages.count (); ++i) {
		if (RKSettingsModuleRPackages::essentialPackages ().contains (packages[i])) {
			not_removing.append (i18n ("Package %1 at %2", packages[i], from_liblocs[i]));
			packages.removeAt (i);
			from_liblocs.removeAt (i);
			--i;
		} else {
			QFileInfo fi (from_liblocs[i]);
			if (!fi.isWritable ()) {
				not_writable.append (i18n ("Package %1 at %2", packages[i], from_liblocs[i]));
				not_writable_int.append (i);
			}
		}
	}
	if (!not_removing.isEmpty ()) {
		KMessageBox::informationList (this, i18n ("The following packages, which you have selected for removal, are essential to the operation of RKWard, and will not be removed. If you are absolutely sure, that you want to remove these packages, please do so on the R command line."), not_removing, i18n ("Not removing certain packages"));
	}
	if (packages.isEmpty ()) return false;

	if (!not_writable.isEmpty ()) {
#ifdef Q_OS_WIN
		KMessageBox::informationList (this, i18n ("Your current user permissions do not allow removing the following packages. These will be skipped."), not_writable, i18n ("Insufficient user permissions"));
		int res = KMessageBox::No;
#else
		int res = KMessageBox::questionYesNoList (this, i18n ("Your current user permissions do not allow removing the following packages. Do you want to skip these packages, or do you want to proceed with administrator privileges (you will be prompted for the password)?"), not_writable, i18n ("Insufficient user permissions"), KGuiItem ("Become root"), KGuiItem ("Skip these packages"));
#endif
		if (res == KMessageBox::Yes) as_root = true;
		else {
			for (int i = not_writable_int.count () - 1; i >= 0; --i) {
				packages.removeAt (not_writable_int[i]);
				from_liblocs.removeAt (not_writable_int[i]);
			}
		}
	}
	if (packages.isEmpty ()) return false;
	RK_ASSERT (packages.count () == from_liblocs.count ());

	QStringList descriptions;
	QString command;
	for (int i = 0; i < packages.count (); ++i) {
		descriptions.append (i18n ("Package %1 at %2", packages[i], from_liblocs[i]));
		// NOTE: the "lib"-parameter to remove.packages is NOT matched to the pkgs-parameter. Therefore, we simply concatenate a bunch of single removals.
		command.append ("remove.packages (" + RObject::rQuote (packages[i]) + ", " + RObject::rQuote (from_liblocs[i]) + ")\n");
	}

	// last check. This may be an annoying third dialog, in the worst case, but at least it can be turned off.
	int res = KMessageBox::warningContinueCancelList (this, i18n ("You are about to remove the following packages. Are you sure you want to proceed?"), descriptions, i18n ("About to remove packages"), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), "remove_packages_warning");
	if (res != KMessageBox::Continue) return false;

	runInstallationCommand (command, as_root, i18n ("Please stand by while removing selected packages"), i18n ("Removing packages"));

	return true;
}

#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif
bool RKLoadLibsDialog::installPackages (const QStringList &packages, QString to_libloc, bool install_suggested_packages, const QStringList& repos) {
	RK_TRACE (DIALOGS);

	if (packages.isEmpty ()) return false;

	bool as_root = false;
	// It is ok, if the selected location does not yet exist. In order to know, whether we can write to it, we have to create it first.
	QDir().mkpath (to_libloc);
	QString altlibloc = RKSettingsModuleRPackages::addUserLibLocTo (library_locations).value (0);
#ifdef Q_OS_WIN
	extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
	qt_ntfs_permission_lookup++;
#endif
	QFileInfo fi = QFileInfo (to_libloc);
	bool writable = fi.isWritable ();
#ifdef Q_OS_WIN
	qt_ntfs_permission_lookup--;
#endif
	if (!writable) {
		QString mcaption = i18n ("Selected library location not writable");
		QString message = i18n ("<p>The directory you have selected for installation (%1) is not writable with your current user permissions.</p>"
			"<p>Would you like to install to %2, instead (you can also press \"Cancel\" and use the \"Configure Repositories\"-button to set up a different directory)?</p>", to_libloc, altlibloc);
#ifdef Q_OS_WIN
		message.append (i18n ("<p>Alternatively, if you have access to an administrator account on this machine, you can use that to install the package(s), or "
			"you could change the permissions of '%1'. Sorry, automatic switching to Administrator is not yet supported in RKWard on Windows.</p>", to_libloc));
		int res = KMessageBox::warningContinueCancel (this, message, mcaption, KGuiItem (i18n ("Install to %1", altlibloc)));
		if (res == KMessageBox::Continue) to_libloc = altlibloc;
#else
		message.append (i18n ("<p>Alternatively, if you are the administrator of this machine, you can try to install the packages as root (you'll be prompted for the root password).</p>"));
		int res = KMessageBox::warningYesNoCancel (this, message, mcaption, KGuiItem (i18n ("Install to %1", altlibloc)), KGuiItem (i18n ("Become root")));
		if (res == KMessageBox::Yes) to_libloc = altlibloc;
		if (res == KMessageBox::No) as_root = true;
#endif
		if (res == KMessageBox::Cancel) return false;
	}

	addLibraryLocation (to_libloc);

	QString command_string = "install.packages (c (\"" + packages.join ("\", \"") + "\")" + ", lib=" + RObject::rQuote (to_libloc);
	QString downloaddir = QDir (RKSettingsModuleGeneral::filesPath ()).filePath ("package_archive");
	if (RKSettingsModuleRPackages::archivePackages ()) {
		QDir (RKSettingsModuleGeneral::filesPath ()).mkdir ("package_archive");
		command_string += ", destdir=" + RObject::rQuote (downloaddir);
	}
	if (install_suggested_packages) command_string += ", dependencies=TRUE";
	command_string += ")\n";

	QString repos_string = "options (repos= c(";
	for (int i = 0; i < repos.count (); ++i) {
		if (i) repos_string.append (", ");
		repos_string.append (RObject::rQuote (repos[i]));
	}
	repos_string.append ("))\n");

	repos_string.append (RKSettingsModuleRPackages::pkgTypeOption ());

	if (as_root) {
		KUser user;
		command_string.append ("system (\"chown " + user.loginName() + ' ' + downloaddir + "/*\")\n");
	}

	runInstallationCommand (repos_string + command_string, as_root, i18n ("Please stand by while installing selected packages"), i18n ("Installing packages"));

	return true;
}

void RKLoadLibsDialog::runInstallationCommand (const QString& command, bool as_root, const QString& message, const QString& title) {
	RK_TRACE (DIALOGS);

	// TODO: won't work with some versions of GCC (which ones exactly)?
//	QFile file (QDir (RKSettingsModuleGeneral::filesPath ()).filePath ("install_script.R"));
// WORKAROUND:
	QDir dir = RKSettingsModuleGeneral::filesPath ();
	QFile file (dir.filePath ("install_script.R"));
// WORKADOUND END
	if (file.open (QIODevice::WriteOnly)) {
		QTextStream stream (&file);
		stream << command;
		stream << "q ()\n";
		file.close();
	} else {
		RK_ASSERT (false);
	}

	QString R_binary (getenv ("R_BINARY"));
	QString call;
	QStringList params;
#ifdef Q_OS_WIN
	RK_ASSERT (!as_root);
	call = R_binary;
#else
	if (as_root) {
		call = QStandardPaths::findExecutable ("kdesu");
		if (call.isEmpty ()) call = QStandardPaths::findExecutable ("kdesudo");
		params << "-t" << "--" << R_binary;
	} else {
		call = R_binary;
	}
#endif
	params << "--no-save" << "--no-restore" << "--file=" + file.fileName ();

	installation_process = new QProcess ();
	installation_process->setProcessChannelMode (QProcess::SeparateChannels);

	connect (installation_process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &RKLoadLibsDialog::processExited);
	connect (installation_process, &QProcess::readyReadStandardOutput, this, &RKLoadLibsDialog::installationProcessOutput);
	connect (installation_process, &QProcess::readyReadStandardError, this, &RKLoadLibsDialog::installationProcessError);

	RKProgressControl *installation_progress = new RKProgressControl (this, message, title, RKProgressControl::CancellableProgress);
	connect (this, &RKLoadLibsDialog::installationComplete, installation_progress, &RKProgressControl::done);
	connect (this, &RKLoadLibsDialog::installationOutput, installation_progress, static_cast<void (RKProgressControl::*)(const QString&)>(&RKProgressControl::newOutput));
	connect (this, &RKLoadLibsDialog::installationError, installation_progress, &RKProgressControl::newError);

	installation_process->start (call, params, QIODevice::ReadWrite | QIODevice::Unbuffered);

	if (!installation_progress->doModal (true)) {
		installation_process->kill ();
		installation_process->waitForFinished (5000);
	}

	file.remove ();
	delete installation_process;
	installation_process = 0;
	emit installedPackagesChanged();
}

void RKLoadLibsDialog::installationProcessOutput () {
	RK_TRACE (DIALOGS);
	RK_ASSERT (installation_process);

	emit installationOutput(QString::fromLocal8Bit(installation_process->readAllStandardOutput()));
}

void RKLoadLibsDialog::installationProcessError () {
	RK_TRACE (DIALOGS);
	RK_ASSERT (installation_process);

	emit installationError(QString::fromLocal8Bit(installation_process->readAllStandardError()));
}

void RKLoadLibsDialog::processExited (int exitCode, QProcess::ExitStatus exitStatus) {
	RK_TRACE (DIALOGS);

	if (exitCode || (exitStatus != QProcess::NormalExit)) {
		emit installationError('\n' + i18n ("Installation process died with exit code %1", exitCode));
	}

	emit installationComplete();
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
	connect (load_button, &QPushButton::clicked, this, &LoadUnloadWidget::loadButtonClicked);
	detach_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionRemoveLeft), i18n ("Unload"), this);
	connect (detach_button, &QPushButton::clicked, this, &LoadUnloadWidget::detachButtonClicked);
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

	connect (loaded_view, &QTreeWidget::itemSelectionChanged, this, &LoadUnloadWidget::updateButtons);
	connect (installed_view, &QTreeWidget::itemSelectionChanged, this, &LoadUnloadWidget::updateButtons);

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
		RK_ASSERT (command->getDataLength () == 5);

		RData::RDataStorage data = command->structureVector ();
		QStringList package = data.at (0)->stringVector ();
		QStringList title = data.at (1)->stringVector ();
		QStringList version = data.at (2)->stringVector ();
		QStringList libpath = data.at (3)->stringVector ();

		int count = package.size ();
		RK_ASSERT (count == title.size ());
		RK_ASSERT (count == version.size ());
		RK_ASSERT (count == libpath.size ());
		for (int i=0; i < count; ++i) {
			QTreeWidgetItem* item = new QTreeWidgetItem (installed_view);
			item->setText (0, package.at (i));
			item->setText (1, title.at (i));
			item->setText (2, version.at (i));
			item->setText (3, libpath.at (i));
		}
		installed_view->resizeColumnToContents (0);
		installed_view->setSortingEnabled (true);
		installed_view->sortItems (0, Qt::AscendingOrder);
	} else if (command->getFlags () == GET_LOADED_PACKAGES) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		QStringList data = command->stringVector ();
		for (int i=0; i < data.size (); ++i) {
			QTreeWidgetItem* item = new QTreeWidgetItem (loaded_view);
			item->setText (0, data.at (i));
			if (RKSettingsModuleRPackages::essentialPackages ().contains (data.at (i))) {
				item->setFlags (Qt::NoItemFlags);
			}
		}
		loaded_view->resizeColumnToContents (0);
		loaded_view->setSortingEnabled (true);
		loaded_view->sortItems (0, Qt::AscendingOrder);
		updateCurrentList ();
	} else if (command->getFlags () == LOAD_PACKAGE_COMMAND) {
		emit loadUnloadDone();
	} else {
		RK_ASSERT (false);
	}
}

void LoadUnloadWidget::updateInstalledPackages () {
	RK_TRACE (DIALOGS);

	installed_view->clear ();
	loaded_view->clear ();

	RKGlobals::rInterface ()->issueCommand (".rk.get.installed.packages ()", RCommand::App | RCommand::Sync | RCommand::GetStructuredData, QString (), this, GET_INSTALLED_PACKAGES, parent->chain);
	RKGlobals::rInterface ()->issueCommand (".packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString (), this, GET_LOADED_PACKAGES, parent->chain);
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
	connect (this, &LoadUnloadWidget::loadUnloadDone, control, &RKProgressControl::done);

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
#include <QHeaderView>
#include <QStyledItemDelegate>

/** Responsible for drawing the "category" items */
class InstallPackagesDelegate : public QStyledItemDelegate {
public:
	InstallPackagesDelegate (QTreeView* parent) : QStyledItemDelegate (parent) {
		table = parent;
		expanded = RKStandardIcons::getIcon (RKStandardIcons::ActionCollapseUp);
		collapsed = RKStandardIcons::getIcon (RKStandardIcons::ActionExpandDown);
	}
	void initStyleOption (QStyleOptionViewItem* option, const QModelIndex& index) const override {
		QStyledItemDelegate::initStyleOption (option, index);
		if (!index.parent ().isValid ()) {
			int ccount = index.model ()->rowCount (index);
			option->text = option->text + " (" + QString::number (ccount) + ')';
			if (ccount) {
				option->icon = table->isExpanded (index) ? expanded : collapsed;
			} else {
				option->icon = QIcon ();    // empty dummy icon to reserve space
			}
			option->features |= QStyleOptionViewItem::HasDecoration;
			option->font.setBold (true);
			option->backgroundBrush = table->palette ().mid ();
		}
	}
	QTreeView* table;
	QIcon expanded;
	QIcon collapsed;
};

InstallPackagesWidget::InstallPackagesWidget (RKLoadLibsDialog *dialog) : QWidget (0) {
	RK_TRACE (DIALOGS);
	InstallPackagesWidget::parent = dialog;
	
	QHBoxLayout *hbox = new QHBoxLayout (this);
	hbox->setContentsMargins (0, 0, 0, 0);

	QVBoxLayout *vbox = new QVBoxLayout ();
	vbox->setContentsMargins (0, 0, 0, 0);
	hbox->addLayout (vbox);
	hbox->setStretchFactor (vbox, 2);

	packages_status = new RKRPackageInstallationStatus (this);
	packages_view = new QTreeView (this);
	packages_view->setSortingEnabled (true);
	model = new RKRPackageInstallationStatusSortFilterModel (this);
	model->setSourceModel (packages_status);
	model->setSortCaseSensitivity (Qt::CaseInsensitive);
	packages_view->setModel (model);
	packages_view->setItemDelegateForColumn (0, new InstallPackagesDelegate (packages_view));
	for (int i = 0; i < model->rowCount (); ++i) {  // the root level captions
		packages_view->setFirstColumnSpanned (i, QModelIndex (), true);
	}
	connect (packages_view, &QTreeView::clicked, this, &InstallPackagesWidget::rowClicked);
	packages_view->setRootIsDecorated (false);
	packages_view->setIndentation (0);
	packages_view->setEnabled (false);
	packages_view->setMinimumHeight (packages_view->sizeHintForRow (0) * 15);	// force a decent height
	packages_view->setMinimumWidth (packages_view->fontMetrics ().width ("This is to force a sensible min width for the packages view (empty on construction)"));
	vbox->addWidget (packages_view);

	QPushButton *configure_repos_button = new QPushButton (i18n ("Configure Repositories"), this);
	RKCommonFunctions::setTips (i18n ("Many packages are available on CRAN (Comprehensive R Archive Network), and other repositories.<br>Click this to add more sources."), configure_repos_button);
	connect (configure_repos_button, &QPushButton::clicked, this, &InstallPackagesWidget::configureRepositories);
	vbox->addWidget (configure_repos_button);

	QVBoxLayout *buttonvbox = new QVBoxLayout ();
	hbox->addLayout (buttonvbox);
	buttonvbox->setContentsMargins (0, 0, 0, 0);
	QLabel *label = new QLabel (i18n ("Show only packages matching:"), this);
	filter_edit = new RKDynamicSearchLine (this);
	RKCommonFunctions::setTips (i18n ("<p>You can limit the packages displayed in the list to with names or titles matching a filter string.</p>") + filter_edit->regexpTip (), label, filter_edit);
	filter_edit->setModelToFilter (model);
	// NOTE: Although the search line sets the filter in the model, automatically, we connect it, here, in order to expand new and updateable sections, when the filter changes.
	connect (filter_edit, &RKDynamicSearchLine::searchChanged, this, &InstallPackagesWidget::filterChanged);
	rkward_packages_only = new QCheckBox (i18n ("Show only packages providing RKWard dialogs"), this);
	RKCommonFunctions::setTips (i18n ("<p>Some but not all R packages come with plugins for RKWard. That means they provide a graphical user-interface in addition to R functions. Check this box to show only such packages.</p><p></p>"), rkward_packages_only);
	connect (rkward_packages_only, &QCheckBox::stateChanged, this, &InstallPackagesWidget::filterChanged);
	filterChanged ();

	mark_all_updates_button = new QPushButton (i18n ("Select all updates"), this);
	connect (mark_all_updates_button, &QPushButton::clicked, this, &InstallPackagesWidget::markAllUpdates);

	install_params = new PackageInstallParamsWidget (this);
	connect (parent, &RKLoadLibsDialog::libraryLocationsChanged, install_params, &PackageInstallParamsWidget::liblocsChanged);

	buttonvbox->addWidget (label);
	buttonvbox->addWidget (filter_edit);
	buttonvbox->addWidget (rkward_packages_only);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (mark_all_updates_button);
	buttonvbox->addStretch (1);
	buttonvbox->addWidget (install_params);
	buttonvbox->addStretch (1);
}

InstallPackagesWidget::~InstallPackagesWidget () {
	RK_TRACE (DIALOGS);
}

void InstallPackagesWidget::activated () {
	RK_TRACE (DIALOGS);

	filter_edit->setFocus ();
	if (!packages_status->initialized ()) {
		initialize ();
		packages_view->sortByColumn (RKRPackageInstallationStatus::PackageName, Qt::AscendingOrder);
	}
}

void InstallPackagesWidget::initialize () {
	RK_TRACE (DIALOGS);

	packages_status->initialize (parent->chain);
	packages_view->setEnabled (true);
	// Force a good width for the icon column, particularly for MacOS X.
	packages_view->header ()->resizeSection (0, packages_view->sizeHintForIndex (model->index (0, 0, model->index (RKRPackageInstallationStatus::NewPackages, 0, QModelIndex ()))).width () + packages_view->indentation ());
	for (int i = 1; i <= RKRPackageInstallationStatus::PackageName; ++i) {
		packages_view->resizeColumnToContents (i);
	}
	// For whatever reason, we have to re-set these, here.
	for (int i = 0; i < model->rowCount (); ++i) {
		packages_view->setFirstColumnSpanned (i, QModelIndex (), true);
	}
	window()->raise(); // needed on Mac, otherwise the dialog may go hiding behind the main app window, after the progress control window closes, for some reason
}

void InstallPackagesWidget::rowClicked (const QModelIndex& row) {
	RK_TRACE (DIALOGS);

	if (!row.parent ().isValid ()) {
		QModelIndex fixed_row = model->index (row.row (), 0, row.parent ());
		packages_view->setExpanded (fixed_row, !packages_view->isExpanded (fixed_row));
	}
}

void InstallPackagesWidget::filterChanged () {
	RK_TRACE (DIALOGS);

	model->setRKWardOnly (rkward_packages_only->isChecked ());
	packages_view->expand (model->mapFromSource (packages_status->index(RKRPackageInstallationStatus::UpdateablePackages, 0)));
	packages_view->expand (model->mapFromSource (packages_status->index(RKRPackageInstallationStatus::NewPackages, 0)));
	// NOTE: filter string already set by RKDynamicSearchLine
}

void InstallPackagesWidget::trySelectPackages (const QStringList &package_names) {
	RK_TRACE (DIALOGS);

	QStringList failed_names;
	for (int i = 0; i < package_names.size(); ++i) {
		QModelIndex index = packages_status->markPackageForInstallation(package_names[i]);
		if (!index.isValid()) {
			failed_names.append(package_names[i]);
		} else {
			packages_view->scrollTo(model->mapFromSource(index));
		}
	}
	if (!failed_names.isEmpty()) {
		KMessageBox::sorry (0, i18n ("The following package(s) requested by the backend have not been found in the package repositories: \"%1\". Maybe the package name was mis-spelled. Or maybe you need to add additional repositories via the \"Configure Repositories\" button.", failed_names.join("\", \"")), i18n ("Package not available"));
	}
}

void InstallPackagesWidget::markAllUpdates () {
	RK_TRACE (DIALOGS);

	QModelIndex index = packages_status->markAllUpdatesForInstallation ();
	packages_view->setExpanded (model->mapFromSource (index), true);
	packages_view->scrollTo (model->mapFromSource (index));
}

void InstallPackagesWidget::doInstall (bool refresh) {
	RK_TRACE (DIALOGS);

	bool changed = false;
	QStringList remove;
	QStringList remove_locs;
	packages_status->packagesToRemove (&remove, &remove_locs);
	if (!remove.isEmpty ()) {
		RK_ASSERT (remove.count () == remove_locs.count ());
		changed |= parent->removePackages (remove, remove_locs);
	}

	QStringList install = packages_status->packagesToInstall ();
	if (!install.isEmpty ()) {
		QString dest = install_params->installLocation ();
		if (!dest.isEmpty ()) {
			changed |= parent->installPackages (install, dest, install_params->installSuggestedPackages (), packages_status->currentRepositories ());
		}
	}

	if (changed && refresh) {
		packages_status->clearStatus ();
		initialize ();
	}
}

void InstallPackagesWidget::apply () {
	RK_TRACE (DIALOGS);

	doInstall (true);
}

void InstallPackagesWidget::ok () {
	RK_TRACE (DIALOGS);

	doInstall (false);
	deleteLater ();
}

void InstallPackagesWidget::cancel () {
	RK_TRACE (DIALOGS);
	deleteLater ();
}

void InstallPackagesWidget::configureRepositories () {
	RK_TRACE (DIALOGS);
	RKSettings::configureSettings (RKSettings::PageRPackages, this, parent->chain);
}

/////////////////////// PackageInstallParamsWidget //////////////////////////

#include <qcombobox.h>
#include <qfileinfo.h>

PackageInstallParamsWidget::PackageInstallParamsWidget (QWidget *parent) : QWidget (parent) {
	RK_TRACE (DIALOGS);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	vbox->addWidget (new QLabel (i18n ("Install packages to:"), this));
	libloc_selector = new QComboBox (this);
	vbox->addWidget (libloc_selector);

	suggested_packages = new QCheckBox (i18n ("Install suggested packages"), this);
	suggested_packages->setChecked (false);
	RKCommonFunctions::setTips (QString ("<p>%1</p>").arg (i18n ("Some packages \"suggest\" additional packages, which are not strictly necessary for using that package, but which may provide additional related functionality. Check this option to include such additional suggested packages.")), suggested_packages);
	vbox->addStretch ();
	vbox->addWidget (suggested_packages);
}

PackageInstallParamsWidget::~PackageInstallParamsWidget () {
	RK_TRACE (DIALOGS);
}

bool PackageInstallParamsWidget::installSuggestedPackages () {
	RK_TRACE (DIALOGS);

	return suggested_packages->isChecked ();
}

QString PackageInstallParamsWidget::installLocation () {
	RK_TRACE (DIALOGS);

	return libloc_selector->currentText ();
}

void PackageInstallParamsWidget::liblocsChanged (const QStringList &newlist) {
	RK_TRACE (DIALOGS);

	libloc_selector->clear ();
	libloc_selector->insertItems (0, RKSettingsModuleRPackages::addUserLibLocTo (newlist));
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
	for (int i = updateable_packages_in_installed.count () - 1; i >= 0; --i) {
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

	RCommand *command = new RCommand (".rk.get.package.installation.state ()", RCommand::App | RCommand::GetStructuredData);
	connect (command->notifier (), &RCommandNotifier::commandFinished, this, &RKRPackageInstallationStatus::statusCommandFinished);
	RKProgressControl *control = new RKProgressControl (this, i18n ("<p>Please stand by while searching for installed and available packages.</p><p><strong>Note:</strong> This requires a working internet connection, and may take some time, esp. if one or more repositories are temporarily unavailable.</p>"), i18n ("Searching for packages"), RKProgressControl::CancellableProgress | RKProgressControl::AutoCancelCommands);
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
	RK_ASSERT (command->getDataLength () == 5);

	RData::RDataStorage top = command->structureVector ();
	RData::RDataStorage available = top[0]->structureVector ();
	available_packages = available[0]->stringVector ();
	available_titles = available[1]->stringVector ();
	available_versions = available[2]->stringVector ();
	available_repos = available[3]->stringVector ();
	enhance_rk_in_available = available[4]->intVector ();

	RData::RDataStorage installed = top[1]->structureVector ();
	installed_packages = installed[0]->stringVector ();
	installed_titles = installed[1]->stringVector ();
	installed_versions = installed[2]->stringVector ();
	installed_libpaths = installed[3]->stringVector ();
	enhance_rk_in_installed = installed[4]->intVector ();
	installed_has_update.fill (false, installed_packages.count ());

	new_packages_in_available = top[2]->intVector ();
	RData::RDataStorage updateable = top[3]->structureVector ();
	updateable_packages_in_installed = updateable[0]->intVector ();
	updateable_packages_in_available = updateable[1]->intVector ();

	for (int i = updateable_packages_in_installed.count () - 1; i >= 0; --i) {
		installed_has_update[updateable_packages_in_installed[i]] = true;
	}

	current_repos = top[4]->stringVector ();

	clearStatus ();
}

void RKRPackageInstallationStatus::clearStatus () {
	RK_TRACE (DIALOGS);

	beginResetModel ();
	available_status.fill (NoAction, available_packages.count ());
	installed_status.fill (NoAction, installed_packages.count ());
	endResetModel ();
}

QVariant RKRPackageInstallationStatus::headerData (int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal) return QVariant ();

	if ((role == Qt::DecorationRole) && (section == EnhancesRKWard)) return RKStandardIcons::getIcon (RKStandardIcons::RKWardIcon);

	if (role == Qt::DisplayRole) {
		if (section == InstallationStatus) return QVariant (i18n ("Status"));
		if (section == PackageName) return QVariant (i18n ("Name"));
		if (section == PackageTitle) return QVariant (i18n ("Title"));
		if (section == Version) return QVariant (i18n ("Version"));
		if (section == Location) return QVariant (i18n ("Location"));
	}
	if ((role == Qt::ToolTipRole) || (role == Qt::WhatsThisRole)) {
		if (section == EnhancesRKWard) return QVariant (i18n ("<p>Packages marked with an RKWard icon in this column provide enhancements to RKWard, typically in the form of additional graphical dialogs.</p>"));
		if (section == InstallationStatus) return QVariant (i18n ("<p>You can select packages for installation / removal by checking / unchecking the corresponding boxes in this column.</p>"));
		if (section == PackageName) return QVariant (i18n ("<p>The name of the package.</p>"));
		if (section == PackageTitle) return QVariant (i18n ("<p>A descriptive title for the package. Currently this is not available for packages in non-local repositories.</p>"));
		if (section == Version) return QVariant (i18n ("<p>Installed and / or available version of the package</p>"));
		if (section == Location) return QVariant (i18n ("<p>Location where the package is installed / available</p>"));
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
			if (role == Qt::DisplayRole) return QVariant (i18n ("Updateable Packages"));
			if (role == Qt::ToolTipRole) return QVariant (i18n ("Packages for which an update is available. This may include packages which were merely built against a newer version of R."));
		} else if (row == NewPackages) {
			if (role == Qt::DisplayRole) return QVariant (i18n ("New Packages"));
			if (role == Qt::ToolTipRole) return QVariant (i18n ("Packages which are available for installation, but which are not currently installed."));
		} else if (row == InstalledPackages) {
			if (role == Qt::DisplayRole) return QVariant (i18n ("Installed Packages"));
			if (role == Qt::ToolTipRole) return QVariant (i18n ("Packages which are installed locally. Note that updates may be available for these packages."));
		}
	} else if (!index.parent ().parent ().isValid ()) {		// model has exactly two levels
		const int col = index.column ();
		const int prow = index.parent ().row ();
		int arow, irow;		// row numbers in the lists of available_packages / installed_packages
		arow = irow = 0;	// Suppress bogus GCC warning (doesn't seem to understand that irow is not needed for NewPackages, and arow is not needed for InstalledPackages)
		if (prow == UpdateablePackages) {
			arow = updateable_packages_in_available.value (index.row ());
			irow = updateable_packages_in_installed.value (index.row ());
		} else if (prow == NewPackages) {
			arow = new_packages_in_available.value (index.row ());
		} else {
			RK_ASSERT (prow == InstalledPackages);
			irow = index.row ();
		}

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
				if (enhance_rk) return RKStandardIcons::getIcon (RKStandardIcons::RKWardIcon);
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
				else {
					RK_ASSERT (prow == UpdateablePackages);
					return QVariant (installed_libpaths.value (irow) + " -> " + available_repos.value (arow));
				}
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
	if (!parent.isValid ()) return createIndex (row, column, (quintptr) std::numeric_limits<quintptr>::max);	// toplevel items
	return createIndex (row, column, parent.row ());				// parent.row () identifies, which toplevel item is the parent.
}

QModelIndex RKRPackageInstallationStatus::parent (const QModelIndex& index) const {
	if (index.internalId () == (quintptr) std::numeric_limits<quintptr>::max) return QModelIndex ();
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

	emit dataChanged(index, index);
	if (bindex.isValid ()) emit dataChanged(bindex, bindex);

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
	rkward_only = false;
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

	if (rkward_only) {
		bool enhance_rk = source_parent.child (source_row, RKRPackageInstallationStatus::EnhancesRKWard).data (Qt::UserRole).toBool ();
		if (!enhance_rk) return false;
	}
// filter on Name and Title
	QString name = source_parent.child (source_row, RKRPackageInstallationStatus::PackageName).data ().toString ();
	if (name.contains (filterRegExp ())) return true;
	QString title = source_parent.child (source_row, RKRPackageInstallationStatus::PackageTitle).data ().toString ();
	return (title.contains (filterRegExp ()));
}

void RKRPackageInstallationStatusSortFilterModel::setRKWardOnly (bool only) {
	RK_TRACE (DIALOGS);

	bool old_only = rkward_only;
	rkward_only = only;
	if (rkward_only != old_only) invalidate ();
}

/////////////////////////
#include "../misc/multistringselector.h"
RKPluginMapSelectionWidget::RKPluginMapSelectionWidget (RKLoadLibsDialog* dialog) : QWidget (dialog) {
	RK_TRACE (DIALOGS);
	model = 0;
	changes_pending = false;

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	vbox->addWidget (new QLabel (i18n ("Installed plugin groups (.pluginmap files)"), this));
	selector = new RKMultiStringSelectorV2 (QString (), this);
	selector->setAlwaysAddAtBottom (true);
	vbox->addWidget (selector);
}

RKPluginMapSelectionWidget::~RKPluginMapSelectionWidget () {
	RK_TRACE (DIALOGS);
}

void RKPluginMapSelectionWidget::activated () {
	RK_TRACE (DIALOGS);

	if (!model) {
		model = new RKSettingsModulePluginsModel (this);
		model->init (RKSettingsModulePlugins::knownPluginmaps ());
		selector->setModel (model, 1);
		connect (selector, &RKMultiStringSelectorV2::insertNewStrings, model, &RKSettingsModulePluginsModel::insertNewStrings);
		connect (selector, &RKMultiStringSelectorV2::swapRows, model, &RKSettingsModulePluginsModel::swapRows);
		connect (selector, &RKMultiStringSelectorV2::listChanged, this, &RKPluginMapSelectionWidget::changed);
	}
}

void RKPluginMapSelectionWidget::apply () {
	RK_TRACE (DIALOGS);

	if (!changes_pending) return;
	RK_ASSERT (model);
	RKSettingsModulePlugins::PluginMapList new_list = RKSettingsModulePlugins::setPluginMaps (model->pluginMaps ());
	selector->setModel (0); // we don't want any extra change notification for this
	model->init (new_list);
	selector->setModel (model, 1);
	changes_pending = false;
}

void RKPluginMapSelectionWidget::cancel () {
	RK_TRACE (DIALOGS);
	deleteLater ();
}

void RKPluginMapSelectionWidget::ok () {
	RK_TRACE (DIALOGS);

	if (!changes_pending) return;
	RK_ASSERT (model);
	RKSettingsModulePlugins::setPluginMaps (model->pluginMaps ());
	deleteLater ();
}

