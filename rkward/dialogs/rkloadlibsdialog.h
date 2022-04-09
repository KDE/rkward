/*
rkloadlibsdialog - This file is part of the RKWard project. Created: Mon Sep 6 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKLOADLIBSDIALOG_H
#define RKLOADLIBSDIALOG_H

#include <kpagedialog.h>

#include <qstringlist.h>
#include <QProcess>
#include <QSortFilterProxyModel>

#include "../settings/rksettingsmoduler.h"
#include "../rbackend/rcommandreceiver.h"

class QLineEdit;
class QTreeView;
class QTreeWidget;
class QSortFilterProxyModel;
class QComboBox;
class QPushButton;
class RKProgressControl;
class QWidget;
class QCloseEvent;
class RCommandChain;
class QCheckBox;
class PackageInstallParamsWidget;
class InstallPackagesWidget;
class RKDynamicSearchLine;

/**
Dialog which excapsulates widgets to load/unload, update and install R packages

@author Thomas Friedrichsmeier
*/

// TODO: add a static member to create (single) instance of the dialog

class RKLoadLibsDialog : public KPageDialog {
Q_OBJECT
public:
	RKLoadLibsDialog (QWidget *parent, RCommandChain *chain, bool modal=false);

	~RKLoadLibsDialog ();

	bool installPackages (const QStringList &packages, QString to_libloc, bool install_suggested_packages, const QStringList& repos);
	bool removePackages (QStringList packages, QStringList from_liblocs);

/** opens a modal RKLoadLibsDialog with the "Install new Packages" tab on front (To be used when a require () fails in the R backend
@param parent parent QWidget. The dialog will be centered there
@param chain RCommandChain to run the necessary commands in
@param package_name name of the required package */
	static void showInstallPackagesModal (QWidget *parent, RCommandChain *chain, const QStringList &package_names);
	static void showPluginmapConfig (QWidget *parent=0, RCommandChain *chain=0);
	QStringList currentLibraryLocations ()  const { return library_locations; };
	void accept () override;
	void reject () override;
signals:
	void downloadComplete ();
	void installationComplete ();
	void libraryLocationsChanged (const QStringList &liblocs);
	void installationOutput (const QString &output);
	void installationError (const QString &error);
	void installedPackagesChanged ();
protected:
	void closeEvent (QCloseEvent *e) override;
protected slots:
	void childDeleted ();
	void processExited (int exitCode, QProcess::ExitStatus exitStatus);
	void installationProcessOutput ();
	void installationProcessError ();
	void automatedInstall (const QStringList &packages);
	void slotPageChanged ();
private:
	void addLibraryLocation (const QString &new_loc);
	void tryDestruct ();
	void runInstallationCommand (const QString& command, bool as_root, const QString& message, const QString& title);
	KPageWidgetItem* addChild (QWidget *child_page, const QString &caption);
friend class LoadUnloadWidget;
friend class InstallPackagesWidget;
	RCommandChain *chain;

	InstallPackagesWidget *install_packages_widget;	// needed for automated installation
	KPageWidgetItem *install_packages_pageitem;
	KPageWidgetItem *configure_pluginmaps_pageitem;

	QStringList library_locations;

	int num_child_widgets;
	bool was_accepted;

	QProcess* installation_process;
};

/**
Shows which packages are available (installed) / loaded, and lets the user load or detach packages.
To be used in RKLoadLibsDialog

@author Thomas Friedrichsmeier
*/
class LoadUnloadWidget : public QWidget {
Q_OBJECT
public:
	explicit LoadUnloadWidget (RKLoadLibsDialog *dialog);
	
	~LoadUnloadWidget ();
signals:
	void loadUnloadDone ();
public slots:
	void loadButtonClicked ();
	void detachButtonClicked ();
	void ok ();
	void apply ();
	void cancel ();
	void updateInstalledPackages ();
	void updateButtons ();
	void activated ();
private:
	void updateCurrentList ();
	void doLoadUnload ();
	
	QTreeWidget *loaded_view;
	QTreeWidget *installed_view;

	QPushButton *load_button;
	QPushButton *detach_button;
	
	QStringList prev_packages;
	
	RKLoadLibsDialog *parent;
};

/** Item model and encapsulation for package status (used in InstallPackagesWidget) */
class RKRPackageInstallationStatus : public QAbstractItemModel {
	Q_OBJECT
public:
	explicit RKRPackageInstallationStatus (QObject* parent);
	~RKRPackageInstallationStatus ();

	void initialize (RCommandChain *chain);

	enum Columns {
		EnhancesRKWard,
		InstallationStatus,
		PackageName,
		PackageTitle,
		Version,
		Location,
		COLUMN_COUNT
	};
	enum ToplevelItems {
		UpdateablePackages,
		NewPackages,
		InstalledPackages,
		TOPLEVELITEM_COUNT
	};
	enum PackageStatusChange {
		Install,
		Remove,
		NoAction
	};

/* Item model implementation */
	int rowCount (const QModelIndex &parent = QModelIndex()) const override;
	int columnCount (const QModelIndex &) const override { return COLUMN_COUNT; };
	QVariant data (const QModelIndex &index, int role=Qt::DisplayRole) const override;
	QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
	Qt::ItemFlags flags (const QModelIndex &index) const override;
	QModelIndex index (int row, int column, const QModelIndex &parent=QModelIndex()) const override;
	QModelIndex parent (const QModelIndex &index) const override;
	bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

/** returns a list of packages selected for installation / update */
	QStringList packagesToInstall () const;
/** fills a list of packages selected for removal, and a parallel list of the locations, from which to remove.
 * @return true, if any packages are marked for removal, false otherwise. */
	bool packagesToRemove (QStringList *packages, QStringList *liblocs);
/** mark a package for installation.
 * @returns the index of the package, if the package is available, an invalid index, if it is not available */
	QModelIndex markPackageForInstallation (const QString& package_name);
/** mark all available updates for installation.
 * @returns the index of the "Updateable Packages" item */
	QModelIndex markAllUpdatesForInstallation ();
/** reset all installation states to NoAction */
	void clearStatus ();
	QStringList currentRepositories () const { return current_repos; };
	bool initialized () const { return _initialized; };
private slots:
	void statusCommandFinished (RCommand *command);
private:
	QStringList available_packages, available_titles, available_versions, available_repos;
	QStringList installed_packages, installed_titles, installed_versions, installed_libpaths;
	RData::IntStorage enhance_rk_in_available;
	RData::IntStorage enhance_rk_in_installed;
	RData::IntStorage new_packages_in_available;
	RData::IntStorage updateable_packages_in_available;
	RData::IntStorage updateable_packages_in_installed;
	QVector<PackageStatusChange> installed_status;
	QVector<PackageStatusChange> available_status;
	QVector<bool> installed_has_update;
	bool _initialized;

	QStringList current_repos;
};

class RKRPackageInstallationStatusSortFilterModel : public QSortFilterProxyModel {
public:
	explicit RKRPackageInstallationStatusSortFilterModel (QObject* parent = 0);
	~RKRPackageInstallationStatusSortFilterModel ();
	void setRKWardOnly (bool only);
protected:
	bool lessThan (const QModelIndex &left, const QModelIndex &right) const override;
	bool filterAcceptsRow (int source_row, const QModelIndex &source_parent) const override;
	bool rkward_only;
};

/**
Allows the user to update / install R packages.
To be used in RKLoadLibsDialog.

@author Thomas Friedrichsmeier
*/
class InstallPackagesWidget : public QWidget {
Q_OBJECT
public:
	explicit InstallPackagesWidget (RKLoadLibsDialog *dialog);
	
	~InstallPackagesWidget ();
	void trySelectPackages (const QStringList &package_names);
	void initialize ();
public slots:
	void ok ();
	void apply ();
	void cancel ();
	void filterChanged ();
	void activated ();
	void markAllUpdates ();
	void configureRepositories ();
	void rowClicked (const QModelIndex& row);
private:
	void doInstall (bool refresh);
	QTreeView *packages_view;
	RKRPackageInstallationStatus *packages_status;
	RKRPackageInstallationStatusSortFilterModel *model;

	QPushButton *mark_all_updates_button;
	RKDynamicSearchLine *filter_edit;
	QCheckBox *rkward_packages_only;
	PackageInstallParamsWidget *install_params;
	
	RKLoadLibsDialog *parent;
};

/**
Simple helper class for RKLoadLibsDialog to allow selection of installation parameters

@author Thomas Friedrichsmeier
*/
class PackageInstallParamsWidget : public QWidget {
Q_OBJECT
public:
	explicit PackageInstallParamsWidget (QWidget *parent);
	
	~PackageInstallParamsWidget ();

	bool installSuggestedPackages ();
	QString installLocation ();
public slots:
	void liblocsChanged (const QStringList &newlist);
private:
	QComboBox *libloc_selector;
	QCheckBox *suggested_packages;
};


#include "../settings/rksettingsmoduleplugins.h"

class RKPluginMapSelectionWidget : public QWidget {
Q_OBJECT
public:
	explicit RKPluginMapSelectionWidget (RKLoadLibsDialog *dialog);
	virtual ~RKPluginMapSelectionWidget ();
public slots:
	void ok ();
	void apply ();
	void cancel ();
	void activated ();
	void changed () { changes_pending = true; };
private:
	RKMultiStringSelectorV2* selector;
	RKSettingsModulePluginsModel* model;
	bool changes_pending;
};

#endif
