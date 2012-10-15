/***************************************************************************
                          rkloadlibsdialog  -  description
                             -------------------
    begin                : Mon Sep 6 2004
    copyright            : (C) 2004, 2006, 2007, 2009, 2011, 2012 by Thomas Friedrichsmeier
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

/**
Dialog which excapsulates widgets to load/unload, update and install R packages

@author Thomas Friedrichsmeier
*/

// TODO: add a static member to create (single) instance of the dialog

class RKLoadLibsDialog : public KPageDialog, public RCommandReceiver {
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
	static void showInstallPackagesModal (QWidget *parent, RCommandChain *chain, const QString &package_name);
	QStringList currentLibraryLocations ()  const { return library_locations; };
signals:
	void downloadComplete ();
	void installationComplete ();
	void libraryLocationsChanged (const QStringList &liblocs);
	void installationOutput (const QString &output);
	void installationError (const QString &error);
	void installedPackagesChanged ();
protected:
	void rCommandDone (RCommand *command);
	void closeEvent (QCloseEvent *e);
protected slots:
/** overloaded to catch button presses. */
	void slotButtonClicked (int button);
	void childDeleted ();
	void processExited (int exitCode, QProcess::ExitStatus exitStatus);
	void installationProcessOutput ();
	void installationProcessError ();
	void automatedInstall ();
	void slotPageChanged ();
private:
	void addLibraryLocation (const QString &new_loc);
	void tryDestruct ();
	void runInstallationCommand (const QString& command, bool as_root, const QString& message, const QString& title);
friend class LoadUnloadWidget;
friend class InstallPackagesWidget;
	RCommandChain *chain;

	InstallPackagesWidget *install_packages_widget;	// needed for automated installation
	KPageWidgetItem *install_packages_pageitem;

	QStringList library_locations;

	QString auto_install_package;
	int num_child_widgets;
	bool accepted;

	QProcess* installation_process;
};

/**
Shows which packages are available (installed) / loaded, and lets the user load or detach packages.
To be used in RKLoadLibsDialog

@author Thomas Friedrichsmeier
*/
class LoadUnloadWidget : public QWidget, public RCommandReceiver {
Q_OBJECT
public:
	LoadUnloadWidget (RKLoadLibsDialog *dialog);
	
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
protected:
	void rCommandDone (RCommand *command);
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
	RKRPackageInstallationStatus (QObject* parent);
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
	int rowCount (const QModelIndex &parent = QModelIndex()) const;
	int columnCount (const QModelIndex &) const { return COLUMN_COUNT; };
	QVariant data (const QModelIndex &index, int role=Qt::DisplayRole) const;
	QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	Qt::ItemFlags flags (const QModelIndex &index) const;
	QModelIndex index (int row, int column, const QModelIndex &parent=QModelIndex()) const;
	QModelIndex parent (const QModelIndex &index) const;
	bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

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
	RKRPackageInstallationStatusSortFilterModel (QObject* parent = 0);
	~RKRPackageInstallationStatusSortFilterModel ();
protected:
	bool lessThan (const QModelIndex &left, const QModelIndex &right) const;
	bool filterAcceptsRow (int source_row, const QModelIndex &source_parent) const;
};

/**
Allows the user to update / install R packages.
To be used in RKLoadLibsDialog.

@author Thomas Friedrichsmeier
*/
class InstallPackagesWidget : public QWidget {
Q_OBJECT
public:
	InstallPackagesWidget (RKLoadLibsDialog *dialog);
	
	~InstallPackagesWidget ();
	void trySelectPackage (const QString &package_name);
	void initialize ();
public slots:
	void ok ();
	void apply ();
	void cancel ();
	void filterStringChanged (const QString &new_filter);
	void activated ();
	void markAllUpdates ();
private:
	void doInstall (bool refresh);
	QTreeView *packages_view;
	RKRPackageInstallationStatus *packages_status;
	QSortFilterProxyModel *model;

	QPushButton *mark_all_updates_button;
	QLineEdit *filter_edit;
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
	PackageInstallParamsWidget (QWidget *parent);
	
	~PackageInstallParamsWidget ();

	bool installSuggestedPackages ();
	QString installLocation ();
public slots:
	void liblocsChanged (const QStringList &newlist);
private:
	QComboBox *libloc_selector;
	QCheckBox *suggested_packages;
};

#endif
