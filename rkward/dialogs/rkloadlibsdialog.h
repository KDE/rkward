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
#include "../rbackend/rcommand.h"

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
class InstallPackagesWidget;
class RKDynamicSearchLine;
class QLabel;

class RKLoadLibsDialogPage : public QWidget {
	Q_OBJECT
public:
	RKLoadLibsDialogPage(QWidget* parent) : QWidget(parent), _changed(false) {};
	virtual void apply() = 0;
	virtual void activated() = 0;
	bool isChanged() const { return _changed; };
Q_SIGNALS:
	void changed();
protected:
	void setChanged() { _changed = true; Q_EMIT changed(); };
	void clearChanged() { _changed = false; };
private:
	bool _changed;
};

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

	bool installPackages (const QStringList &packages, QString to_libloc, bool install_suggested_packages);
	bool removePackages (QStringList packages, QStringList from_liblocs);

/** opens a modal RKLoadLibsDialog with the "Install new Packages" tab on front (To be used when a require () fails in the R backend
@param parent parent QWidget. The dialog will be centered there
@param chain RCommandChain to run the necessary commands in
@param package_name name of the required package */
	static void showInstallPackagesModal (QWidget *parent, RCommandChain *chain, const QStringList &package_names);
	static void showPluginmapConfig(QWidget *parent=nullptr, RCommandChain *chain=nullptr);
	QStringList currentLibraryLocations ()  const { return library_locations; };
Q_SIGNALS:
	void libraryLocationsChanged (const QStringList &liblocs);
	void installedPackagesChanged ();
protected:
	void closeEvent (QCloseEvent *e) override;
protected Q_SLOTS:
	void automatedInstall (const QStringList &packages);
	void slotPageChanged ();
private:
	void queryClose();
	void addLibraryLocation (const QString &new_loc);
	void tryDestruct ();
	void runInstallationCommand (const QString& command, bool as_root, const QString& message, const QString& title);
	KPageWidgetItem* addChild(RKLoadLibsDialogPage *child_page, const QString &caption);
friend class LoadUnloadWidget;
friend class InstallPackagesWidget;
	RCommandChain *chain;

	InstallPackagesWidget *install_packages_widget;	// needed for automated installation
	KPageWidgetItem *install_packages_pageitem;
	KPageWidgetItem *configure_pluginmaps_pageitem;

	QStringList library_locations;

	QList<RKLoadLibsDialogPage*> pages;

	QProcess* installation_process;
};

/**
Shows which packages are available (installed) / loaded, and lets the user load or detach packages.
To be used in RKLoadLibsDialog

@author Thomas Friedrichsmeier
*/
class LoadUnloadWidget : public RKLoadLibsDialogPage {
Q_OBJECT
public:
	explicit LoadUnloadWidget (RKLoadLibsDialog *dialog);
	
	~LoadUnloadWidget ();
	void apply() override;
	void activated() override;
Q_SIGNALS:
	void loadUnloadDone ();
public Q_SLOTS:
	void loadButtonClicked ();
	void detachButtonClicked ();
	void updateInstalledPackages ();
	void updateButtons ();
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
	explicit RKRPackageInstallationStatus (QObject* parent, QWidget* diplay_area);
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
		InstalledPackages,
		NewPackages,
		UpdateablePackages,
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
	bool initialized () const { return _initialized; };
Q_SIGNALS:
	void changed();
private Q_SLOTS:
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

	QWidget *display_area;
};

class RKRPackageInstallationStatusSortFilterModel : public QSortFilterProxyModel {
public:
	explicit RKRPackageInstallationStatusSortFilterModel(QObject* parent = nullptr);
	~RKRPackageInstallationStatusSortFilterModel();
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
class InstallPackagesWidget : public RKLoadLibsDialogPage {
Q_OBJECT
public:
	explicit InstallPackagesWidget (RKLoadLibsDialog *dialog);
	
	~InstallPackagesWidget ();
	void trySelectPackages (const QStringList &package_names);
	void initialize ();
	void apply() override;
	void activated() override;
public Q_SLOTS:
	void filterChanged ();
	void markAllUpdates ();
	void configureRepositories ();
	void rowClicked (const QModelIndex& row);
private:
	void doInstall();
	void updateStatus();
	QTreeView *packages_view;
	RKRPackageInstallationStatus *packages_status;
	RKRPackageInstallationStatusSortFilterModel *model;

	RKDynamicSearchLine *filter_edit;
	QCheckBox *rkward_packages_only;
	QComboBox *libloc_selector;
	QCheckBox *suggested_packages;
	QLabel *status_label;

	RKLoadLibsDialog *parent;
};

#include "../settings/rksettingsmoduleplugins.h"

class RKPluginMapSelectionWidget : public RKLoadLibsDialogPage {
Q_OBJECT
public:
	explicit RKPluginMapSelectionWidget (RKLoadLibsDialog *dialog);
	virtual ~RKPluginMapSelectionWidget ();
	void apply() override;
	void activated() override;
private:
	RKMultiStringSelectorV2* selector;
	RKSettingsModulePluginsModel* model;
};

#endif
