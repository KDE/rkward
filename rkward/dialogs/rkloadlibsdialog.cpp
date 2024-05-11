/*
rkloadlibsdialog - This file is part of RKWard (https://rkward.kde.org). Created: Mon Sep 6 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkloadlibsdialog.h"

#include <QLayout>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <qcheckbox.h>
#include <QDir>
#include <QTimer>
#include <QTextStream>
#include <QCloseEvent>
#include <QSortFilterProxyModel>
#include <QApplication>
#include <QLineEdit>
#include <QStandardPaths>

#include <KLocalizedString>
#include <kmessagebox.h>
#include <kuser.h>
#include <KMessageWidget>

#include "../rbackend/rkrinterface.h"
#include "../rbackend/rksessionvars.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettings.h"
#include "../core/robjectlist.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkdynamicsearchline.h"

#include "../debug.h"

#include <stdlib.h>


RKLoadLibsDialog::RKLoadLibsDialog (QWidget *parent, RCommandChain *chain, bool modal) : KPageDialog (parent) {
	RK_TRACE (DIALOGS);
	RKLoadLibsDialog::chain = chain;
	installation_process = nullptr;

	setFaceType (KPageDialog::Tabbed);
	setModal (modal);
	setWindowTitle (i18n ("Configure Packages"));
	setStandardButtons (QDialogButtonBox::Apply | QDialogButtonBox::Close);
	disconnect(buttonBox(), &QDialogButtonBox::rejected, this, &RKLoadLibsDialog::reject);
	connect(button(QDialogButtonBox::Close), &QPushButton::clicked, this, &RKLoadLibsDialog::queryClose);
	button(QDialogButtonBox::Apply)->setEnabled(false);
	connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, this, [this]() { button(QDialogButtonBox::Apply)->setEnabled(false); });

	LoadUnloadWidget *luwidget = new LoadUnloadWidget (this);
	addChild (luwidget, i18n ("Load / Unload R packages"));
	connect (this, &RKLoadLibsDialog::installedPackagesChanged, luwidget, &LoadUnloadWidget::updateInstalledPackages);

	install_packages_widget = new InstallPackagesWidget (this);
	install_packages_pageitem = addChild (install_packages_widget, i18n ("Install / Update / Remove R packages"));

	configure_pluginmaps_pageitem = addChild (new RKPluginMapSelectionWidget (this), i18n ("Manage RKWard Plugins"));

	connect(this, &KPageDialog::currentPageChanged, this, &RKLoadLibsDialog::slotPageChanged);
	QTimer::singleShot(0, this, [this](){ slotPageChanged(); });

	RCommand *command = new RCommand(".libPaths()", RCommand::App | RCommand::GetStringVector);
	connect(command->notifier(), &RCommandNotifier::commandFinished, this, [this](RCommand *command) {
		RK_ASSERT (command->getDataType() == RData::StringVector);
		RK_ASSERT (command->getDataLength() > 0);
		// NOTE: The problem is that e.g. R_LIBS_USER is not in .libPaths() if it does not exist, yet. But it should be available as an option, of course
		library_locations = command->stringVector();
		Q_EMIT libraryLocationsChanged(library_locations);
	});
	RInterface::issueCommand(command, chain);
}

RKLoadLibsDialog::~RKLoadLibsDialog () {
	RK_TRACE (DIALOGS);
}

void RKLoadLibsDialog::queryClose() {
	RK_TRACE (DIALOGS);

	bool changes = false;
	bool do_close = true;
	for (int i = 0; i < pages.size(); ++i) {
		changes |= pages[i]->isChanged();
	}
	if (changes) {
		do_close = (KMessageBox::questionTwoActions(this, i18n("Closing will discard pending changes. Are you sure?"), i18n("Discard changes?"), KStandardGuiItem::discard(), KGuiItem(i18nc("@action:button", "Do no close"))) == KMessageBox::PrimaryAction);
	}
	if (do_close) close();
}

KPageWidgetItem* RKLoadLibsDialog::addChild(RKLoadLibsDialogPage *child_page, const QString &caption) {
	RK_TRACE (DIALOGS);

	// TODO: Can't convert these signal/slot connections to new syntax, without creating a common base class for the child pages
	connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, child_page, &RKLoadLibsDialogPage::apply);
	connect(child_page, &RKLoadLibsDialogPage::changed, this, [this]() { button(QDialogButtonBox::Apply)->setEnabled(true); });
	pages.append(child_page);
	return addPage(child_page, caption);
}

void RKLoadLibsDialog::slotPageChanged () {
	RK_TRACE (DIALOGS);

	if (!currentPage()) return;
	QTimer::singleShot(0, dynamic_cast<RKLoadLibsDialogPage*>(currentPage()->widget()), &RKLoadLibsDialogPage::activated);
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

void RKLoadLibsDialog::closeEvent(QCloseEvent *e) {
	RK_TRACE(DIALOGS);
	e->accept();
	deleteLater();
}

void RKLoadLibsDialog::addLibraryLocation (const QString& new_loc) {
	RK_TRACE (DIALOGS);

	if (library_locations.contains (new_loc)) return;

	if (QDir ().mkpath (new_loc)) RKSettingsModuleRPackages::addLibraryLocation (new_loc, chain);
	library_locations.prepend (new_loc);
	Q_EMIT libraryLocationsChanged(library_locations);
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
		int res = KMessageBox::SecondaryAction;
#else
		int res = KMessageBox::questionTwoActionsList (this, i18n ("Your current user permissions do not allow removing the following packages. Do you want to skip these packages, or do you want to proceed with administrator privileges (you will be prompted for the password)?"), not_writable, i18n ("Insufficient user permissions"), KGuiItem (i18nc("@action:button", "Become root")), KGuiItem (i18nc("@action:button", "Skip these packages")));
#endif
		if (res == KMessageBox::PrimaryAction) as_root = true;
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
bool RKLoadLibsDialog::installPackages (const QStringList &packages, QString to_libloc, bool install_suggested_packages) {
	RK_TRACE (DIALOGS);

	if (packages.isEmpty ()) return false;

	bool as_root = false;
	// It is ok, if the selected location does not yet exist. In order to know, whether we can write to it, we have to create it first.
	QDir().mkpath(to_libloc);
	QString altlibloc = RKSettingsModuleRPackages::addUserLibLocTo(library_locations).value(0);
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
		int res = KMessageBox::warningTwoActionsCancel (this, message, mcaption, KGuiItem (i18n ("Install to %1", altlibloc)), KGuiItem (i18n ("Become root")));
		if (res == KMessageBox::PrimaryAction) to_libloc = altlibloc;
		if (res == KMessageBox::SecondaryAction) as_root = true;
#endif
		if (res == KMessageBox::Cancel) return false;
	}

	addLibraryLocation(to_libloc);

	QString command_string = "install.packages (c (\"" + packages.join("\", \"") + "\")" + ", lib=" + RObject::rQuote(to_libloc);
	QString downloaddir = QDir (RKSettingsModuleGeneral::filesPath ()).filePath ("package_archive");
	if (RKSettingsModuleRPackages::archivePackages ()) {
		QDir (RKSettingsModuleGeneral::filesPath ()).mkdir ("package_archive");
		command_string += ", destdir=" + RObject::rQuote (downloaddir);
	}
	if (install_suggested_packages) command_string += ", dependencies=TRUE";
	command_string += ")";

	runInstallationCommand (command_string, as_root, i18n ("Please stand by while installing selected packages"), i18n ("Installing packages"));

	return true;
}

void RKLoadLibsDialog::runInstallationCommand (const QString& command, bool as_root, const QString& message, const QString& title) {
	RK_TRACE (DIALOGS);

	auto control = new RKInlineProgressControl(install_packages_widget, true);
	control->setText(QString("<b>%1</b><br/>%2...").arg(title, message));
	control->show();

	RCommand *rcommand;
	if (as_root) {
		QStringList libexecpath(LIBEXECDIR "/kf5");
		libexecpath << QString(LIBEXECDIR "/kf6");
		QString call = QStandardPaths::findExecutable("kdesu");
		if (call.isEmpty()) call = QStandardPaths::findExecutable("kdesu", libexecpath);
		if (call.isEmpty ()) call = QStandardPaths::findExecutable("kdesudo");
		if (call.isEmpty ()) call = QStandardPaths::findExecutable("kdesudo", libexecpath);
		if (call.isEmpty()) {
			KMessageBox::error(this, i18n("Neither kdesu nor kdesudo could be found. To install as root, please install one of these packages."), i18n("kdesu not found"));
			return;
		}

		QStringList call_with_params(call);
		call_with_params << "-t" << "--" << RKSessionVars::RBinary() << "--no-save" << "--no-restore" << "--file=";
		KUser user;
		QString aux_command = QString("local({ "
			"install_script <- tempfile(\".R\"); f <- file(install_script, \"w\")\n"
			"repos <- options()$repos\n"
			"pkgType <- options()$pkgType\n"
			"libPaths <- .libPaths()\n"
			"dump(c(\"repos\", \"pkgType\", \"libPaths\"), f)\n"
			"cat(\"\\n\", file=f, append=TRUE)\n"
			"cat(") + RObject::rQuote("options(\"repos\"=repos, \"pkgType\"=pkgType)\n") + QString(", file=f, append=TRUE)\n"
			"cat(\".libPaths(libPaths)\\n\"") + QString(", file=f, append=TRUE)\n"
			"cat(") + RObject::rQuote(command + "\n") + QString(", file=f, append=TRUE)\n"
			"cat(") + RObject::rQuote("system(\"chown " + user.loginName() + ' ' + QDir(RKSettingsModuleGeneral::filesPath()).filePath("package_archive") + "/*\")") + QString(", file=f, append=TRUE)\n"
			"cat(\"\\n\", file=f, append=TRUE)\n"
			"close(f)\n"
			"system(paste0(" + RObject::rQuote(call_with_params.join(' ')) + ", install_script))\n"
			"unlink(install_script)\n"
		"})");

		rcommand = new RCommand(aux_command, RCommand::App);
	} else {
		rcommand = new RCommand(command, RCommand::App);
	}

	control->addRCommand(rcommand);
	RInterface::issueCommand(rcommand, chain);
	connect(rcommand->notifier(), &RCommandNotifier::commandFinished, this, [this]() {
		Q_EMIT installedPackagesChanged();
	});
}

////////////////////// LoadUnloadWidget ////////////////////////////

LoadUnloadWidget::LoadUnloadWidget (RKLoadLibsDialog *dialog) : RKLoadLibsDialogPage(nullptr) {
	RK_TRACE (DIALOGS);
	LoadUnloadWidget::parent = dialog;
	
	QVBoxLayout *mvbox = new QVBoxLayout (this);
	
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

void LoadUnloadWidget::updateInstalledPackages () {
	RK_TRACE (DIALOGS);

	installed_view->clear ();
	loaded_view->clear ();

	auto command = new RCommand(".rk.get.installed.packages()", RCommand::App | RCommand::Sync | RCommand::GetStructuredData);
	connect(command->notifier(), &RCommandNotifier::commandFinished, this, [this](RCommand *command) {
		if (command->failed()) return;
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
	});
	RInterface::issueCommand(command, parent->chain);

	command = new RCommand(".packages()", RCommand::App | RCommand::Sync | RCommand::GetStringVector);
	connect(command->notifier(), &RCommandNotifier::commandFinished, this, [this](RCommand *command) {
		if (command->failed()) return;
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
	});
	RInterface::issueCommand(command, parent->chain);
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
	setChanged();
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
	setChanged();
}

void LoadUnloadWidget::updateButtons () {
	RK_TRACE (DIALOGS);

	detach_button->setEnabled (!loaded_view->selectedItems ().isEmpty ());
	load_button->setEnabled (!installed_view->selectedItems ().isEmpty ());
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

	RKInlineProgressControl *control = new RKInlineProgressControl(this, false);
	control->setText(i18n("There has been an error while trying to load / unload packages. See transcript below for details"));

	// load packages previously not loaded
	for (int i = 0; i < loaded_view->topLevelItemCount (); ++i) {
		QTreeWidgetItem* loaded = loaded_view->topLevelItem (i);
		if (!prev_packages.contains (loaded->text (0))) {
			RCommand *command = new RCommand ("library (\"" + loaded->text (0) + "\")", RCommand::App | RCommand::ObjectListUpdate);
			control->addRCommand (command);
			RInterface::issueCommand (command, parent->chain);
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
		if (!messages.isEmpty ()) KMessageBox::error(this, messages.join("\n"));
	}

	// find out, when we're done
	RCommand *command = new RCommand(QString(), RCommand::EmptyCommand);
	connect(command->notifier(), &RCommandNotifier::commandFinished, this, [this](RCommand *) {
		clearChanged();
	});
	control->addRCommand(command);  // this is actually important, in case no commands had been generated, above
	RInterface::issueCommand(command, parent->chain);
	control->setAutoCloseWhenCommandsDone(true);
	control->show(100);
}

void LoadUnloadWidget::apply () {
	RK_TRACE (DIALOGS);

	if (!isChanged()) return;
	doLoadUnload ();
	updateCurrentList ();
}

/////////////////////// InstallPackagesWidget //////////////////////////
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QComboBox>

/** Responsible for drawing the "category" items */
class InstallPackagesDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	explicit InstallPackagesDelegate(QTreeView* parent) : QStyledItemDelegate(parent) {
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
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
		QStyledItemDelegate::paint(painter, option, index);
		if ((!index.parent().isValid()) && (index.data(Qt::UserRole) == RKRPackageInstallationStatus::UpdateablePackages)) {
			QStyleOptionButton button;
			QRect r = option.rect; // rect of the entire cell
			r.setLeft(r.left() + r.width()/2); // for simplicity, use half the width for the button
			button.text = i18n("Select all updates");
			button.state = QStyle::State_Enabled;
			button.rect = r;
			QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
		}
	}
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
		if ((!index.parent().isValid()) && (index.data(Qt::UserRole) == RKRPackageInstallationStatus::UpdateablePackages)) {
			if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonPress)) {
				QMouseEvent *e = (QMouseEvent *) event;
				QRect r = option.rect; // rect of the entire cell
				r.setLeft(r.left() + r.width()/2);
				if (r.contains(e->pos())) {
					if (event->type() == QEvent::MouseButtonRelease) {
						Q_EMIT selectAllUpdates();
					}
					event->accept();
					return true;
				}
			}
		}
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}

	QTreeView* table;
	QIcon expanded;
	QIcon collapsed;
Q_SIGNALS:
	void selectAllUpdates();
};

InstallPackagesWidget::InstallPackagesWidget (RKLoadLibsDialog *dialog) : RKLoadLibsDialogPage(nullptr) {
	RK_TRACE (DIALOGS);
	InstallPackagesWidget::parent = dialog;
	
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);
	QHBoxLayout *filterbox = new QHBoxLayout();
	filterbox->setContentsMargins(
		style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
		style()->pixelMetric(QStyle::PM_LayoutTopMargin),
		style()->pixelMetric(QStyle::PM_LayoutRightMargin),
		style()->pixelMetric(QStyle::PM_LayoutBottomMargin)
	);
	filterbox->setSpacing(style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));
	vbox->addLayout(filterbox);
	auto horizontalSeparator = new QFrame(this);
	horizontalSeparator->setFrameShape(QFrame::HLine);
	vbox->addWidget(horizontalSeparator);

	packages_view = new QTreeView (this);
	packages_status = new RKRPackageInstallationStatus(this, packages_view);
	packages_view->setSortingEnabled (true);
	model = new RKRPackageInstallationStatusSortFilterModel (this);
	model->setSourceModel (packages_status);
	model->setSortCaseSensitivity (Qt::CaseInsensitive);
	packages_view->setModel (model);
	auto delegate = new InstallPackagesDelegate(packages_view);
	packages_view->setItemDelegateForColumn(0, delegate);
	connect(delegate, &InstallPackagesDelegate::selectAllUpdates, this, &InstallPackagesWidget::markAllUpdates);
	connect (packages_view, &QTreeView::clicked, this, &InstallPackagesWidget::rowClicked);
	packages_view->setRootIsDecorated (false);
	packages_view->setIndentation (0);
	packages_view->setMinimumHeight (packages_view->sizeHintForRow (0) * 15);	// force a decent height
	QString dummy("This is to force a sensible min width for the packages view (empty on construction)");
	packages_view->setMinimumWidth(packages_view->fontMetrics().horizontalAdvance(dummy)*2);
	vbox->addWidget (packages_view);

	QLabel *label = new QLabel (i18n ("Show only packages matching:"), this);
	filter_edit = new RKDynamicSearchLine (this);
	RKCommonFunctions::setTips (i18n ("<p>You can limit the packages displayed in the list to with names or titles matching a filter string.</p>") + filter_edit->regexpTip (), label, filter_edit);
	filter_edit->setModelToFilter (model);
	// NOTE: Although the search line sets the filter in the model, automatically, we connect it, here, in order to expand new and updateable sections, when the filter changes.
	connect (filter_edit, &RKDynamicSearchLine::searchChanged, this, &InstallPackagesWidget::filterChanged);
	rkward_packages_only = new QCheckBox (i18n ("Show only packages providing RKWard dialogs"), this);
	RKCommonFunctions::setTips(i18n("<p>Some but not all R packages come with plugins for RKWard. That means they provide a graphical user-interface in addition to R functions. Check this box to show only such packages.</p>"), rkward_packages_only);
	connect (rkward_packages_only, &QCheckBox::stateChanged, this, &InstallPackagesWidget::filterChanged);
	filterChanged ();

	horizontalSeparator = new QFrame(this);
	horizontalSeparator->setFrameShape(QFrame::HLine);
	vbox->addWidget(horizontalSeparator);

	filterbox->addWidget (label);
	filterbox->addWidget (filter_edit);
	filterbox->addWidget (rkward_packages_only);

	auto settingsbox = new QHBoxLayout();
	settingsbox->setContentsMargins(
		style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
		style()->pixelMetric(QStyle::PM_LayoutTopMargin),
		style()->pixelMetric(QStyle::PM_LayoutRightMargin),
		style()->pixelMetric(QStyle::PM_LayoutBottomMargin)
	);
	settingsbox->setSpacing(style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));
	vbox->addLayout(settingsbox);
	settingsbox->addWidget(new QLabel(i18n("Install packages to:")));
	libloc_selector = new QComboBox();
	settingsbox->addWidget(libloc_selector);

	suggested_packages = new QCheckBox(i18n("Install suggested packages"));
	suggested_packages->setChecked(false);
	RKCommonFunctions::setTips(QString("<p>%1</p>").arg(i18n("Some packages \"suggest\" additional packages, which are not strictly necessary for using that package, but which may provide additional related functionality. Check this option to include such additional suggested packages.")), suggested_packages);
	connect(parent, &RKLoadLibsDialog::libraryLocationsChanged, this, [this](const QStringList &newlist) {
		libloc_selector->clear();
		libloc_selector->insertItems(0, RKSettingsModuleRPackages::addUserLibLocTo(newlist));
	});
	settingsbox->addWidget(suggested_packages);
	settingsbox->addStretch();

	QPushButton *configure_repos_button = new QPushButton(i18n("Configure Repositories"));
	configure_repos_button->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionConfigureGeneric));
	RKCommonFunctions::setTips(i18n("Many packages are available on CRAN (Comprehensive R Archive Network), and other repositories.<br>Click this to add more sources."), configure_repos_button);
	connect(configure_repos_button, &QPushButton::clicked, this, &InstallPackagesWidget::configureRepositories);
	settingsbox->addWidget(configure_repos_button);

	status_label = new QLabel();
	vbox->addWidget(status_label);
	status_label->hide();

	connect(parent, &RKLoadLibsDialog::installedPackagesChanged, this, &InstallPackagesWidget::initialize);
	connect(packages_status, &RKRPackageInstallationStatus::changed, this, &InstallPackagesWidget::updateStatus);
}

InstallPackagesWidget::~InstallPackagesWidget () {
	RK_TRACE (DIALOGS);
}

void InstallPackagesWidget::updateStatus() {
	RK_TRACE (DIALOGS);
	QStringList dummy1, dummy2, dummy3;
	packages_status->packagesToRemove(&dummy1, &dummy2);
	dummy3 = packages_status->packagesToInstall();
	if (dummy3.isEmpty() && dummy1.isEmpty()) status_label->hide();
	else {
		// TODO: Obviously, it would be nice to spell out, how many dependencies will be installed
		status_label->setText(i18n("<b>Install %1 packages (plus dependencies), remove %2 packages</b>", dummy3.size(), dummy1.size()));
		status_label->show();
		setChanged();
	}
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

	packages_status->clearStatus();
	packages_status->initialize (parent->chain);

	RInterface::whenAllFinished(this, [this]() {
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
		clearChanged();
		updateStatus();
	}, parent->chain);
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

	RInterface::whenAllFinished(this, [this, package_names]() {
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
			KMessageBox::error(nullptr, i18n("The following package(s) requested by the backend have not been found in the package repositories: \"%1\". Maybe the package name was mis-spelled. Or maybe you need to add additional repositories via the \"Configure Repositories\" button.", failed_names.join("\", \"")), i18n("Package not available"));
		}
	}, parent->chain);
}

void InstallPackagesWidget::markAllUpdates () {
	RK_TRACE (DIALOGS);

	QModelIndex index = packages_status->markAllUpdatesForInstallation();
	QTimer::singleShot(0, this, [this, index](){
		packages_view->setExpanded(model->mapFromSource(index), true);
		packages_view->scrollTo(model->mapFromSource(index));
	});
}

void InstallPackagesWidget::doInstall() {
	RK_TRACE (DIALOGS);

	QStringList remove;
	QStringList remove_locs;
	packages_status->packagesToRemove (&remove, &remove_locs);
	if (!remove.isEmpty ()) {
		RK_ASSERT (remove.count () == remove_locs.count ());
		parent->removePackages (remove, remove_locs);
	}

	QStringList install = packages_status->packagesToInstall ();
	if (!install.isEmpty ()) {
		QString dest = libloc_selector->currentText();
		if (!dest.isEmpty()) {
			parent->installPackages(install, dest, suggested_packages->isChecked());
		}
	}
}

void InstallPackagesWidget::apply () {
	RK_TRACE (DIALOGS);

	if (!isChanged()) return;
	doInstall();
}

void InstallPackagesWidget::configureRepositories () {
	RK_TRACE (DIALOGS);
	RKSettings::configureSettings (RKSettings::PageRPackages, this, parent->chain);
}

/////////// RKRPackageInstallationStatus /////////////////

RKRPackageInstallationStatus::RKRPackageInstallationStatus (QObject* parent, QWidget* display_area) : QAbstractItemModel (parent), display_area(display_area) {
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
	connect(command->notifier(), &RCommandNotifier::commandFinished, this, &RKRPackageInstallationStatus::statusCommandFinished);
	RKInlineProgressControl *control = new RKInlineProgressControl(display_area, true);
	control->setText(i18n("<p>Please stand by while searching for installed and available packages.</p><p><strong>Note:</strong> This requires a working internet connection, and may take some time, esp. if one or more repositories are temporarily unavailable.</p>"));
	control->addRCommand(command);
	control->setAutoCloseWhenCommandsDone(true);
	RInterface::issueCommand(command, chain);
	control->show(100);
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
			if (role == Qt::UserRole) return UpdateablePackages;
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
	if (pos == InstalledPackages) flags |= Qt::ItemIsUserTristate;
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

	Q_EMIT dataChanged(index, index);
	if (bindex.isValid ()) Q_EMIT dataChanged(bindex, bindex);
	Q_EMIT changed();

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
		return (left.row () < right.row ());
	}
	if (left.column () == RKRPackageInstallationStatus::EnhancesRKWard) {
		return (!left.data (Qt::UserRole).toBool ());
	}
	return QSortFilterProxyModel::lessThan (left, right);
}

bool RKRPackageInstallationStatusSortFilterModel::filterAcceptsRow (int source_row, const QModelIndex &source_parent) const {
	if (!source_parent.isValid ()) return true;		// Never filter the top level item

	if (rkward_only) {
		bool enhance_rk = sourceModel()->index(source_row, RKRPackageInstallationStatus::EnhancesRKWard, source_parent).data(Qt::UserRole).toBool();
		if (!enhance_rk) return false;
	}
// filter on Name and Title
	QString name = sourceModel()->index(source_row, RKRPackageInstallationStatus::PackageName, source_parent).data().toString();
	if (name.contains (filterRegularExpression ())) return true;
	QString title = sourceModel()->index(source_row, RKRPackageInstallationStatus::PackageTitle, source_parent).data().toString();
	return (title.contains (filterRegularExpression ()));
}

void RKRPackageInstallationStatusSortFilterModel::setRKWardOnly (bool only) {
	RK_TRACE (DIALOGS);

	bool old_only = rkward_only;
	rkward_only = only;
	if (rkward_only != old_only) invalidate ();
}

/////////////////////////
#include "../misc/multistringselector.h"
RKPluginMapSelectionWidget::RKPluginMapSelectionWidget (RKLoadLibsDialog* dialog) : RKLoadLibsDialogPage(dialog) {
	RK_TRACE (DIALOGS);
	model = nullptr;

	QVBoxLayout *vbox = new QVBoxLayout (this);
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
		connect (selector, &RKMultiStringSelectorV2::listChanged, this, &RKPluginMapSelectionWidget::setChanged);
	}
}

void RKPluginMapSelectionWidget::apply () {
	RK_TRACE (DIALOGS);

	if (!isChanged()) return;
	RK_ASSERT (model);
	auto new_list = RKSettingsModulePlugins::setPluginMaps(model->pluginMaps());
	selector->setModel(nullptr); // we don't want any extra change notification for this
	model->init (new_list);
	selector->setModel (model, 1);
	clearChanged();
}

#include "rkloadlibsdialog.moc"
