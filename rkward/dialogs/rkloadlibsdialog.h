/***************************************************************************
                          rkloadlibsdialog  -  description
                             -------------------
    begin                : Mon Sep 6 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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

#include "../settings/rksettingsmoduler.h"
#include "../rbackend/rcommandreceiver.h"

class QTreeWidget;
class QTreeWidgetItem;
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

TODO: The logic of passing on the button presses (ok, cancel, etc.) is rather surprising and a bit confusing. This should be made more straight-forward.

@author Thomas Friedrichsmeier
*/

// TODO: add a static member to create (single) instance of the dialog

class RKLoadLibsDialog : public KPageDialog, public RCommandReceiver {
Q_OBJECT
public:
	RKLoadLibsDialog (QWidget *parent, RCommandChain *chain, bool modal=false);

	~RKLoadLibsDialog ();

	bool installPackages (const QStringList &packages, const QString &to_libloc, bool install_dependencies, bool as_root);

/** opens a modal RKLoadLibsDialog with the "Install new Packages" tab on front (To be used when a require () fails in the R backend
@param parent parent QWidget. The dialog will be centered there
@param chain RCommandChain to run the necessary commands in
@param package_name name of the required package */
	static void showInstallPackagesModal (QWidget *parent, RCommandChain *chain, const QString &package_name);
signals:
	void downloadComplete ();
	void installationComplete ();
	void libraryLocationsChanged (const QStringList &);
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
private:
	void tryDestruct ();
friend class LoadUnloadWidget;
friend class UpdatePackagesWidget;
friend class InstallPackagesWidget;
	RCommandChain *chain;

	InstallPackagesWidget *install_packages_widget;	// needed for automated installation
	KPageWidgetItem *install_packages_pageitem;

	QString auto_install_package;
	int num_child_widgets;
	bool accepted;
	QString repos_string;

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
	LoadUnloadWidget (RKLoadLibsDialog *dialog, QWidget *parent);
	
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

/**
Shows which packages are can be updated from CRAN.
Ro be used in RKLoadLibsDialog.

@author Thomas Friedrichsmeier
*/
class UpdatePackagesWidget : public QWidget, public RCommandReceiver {
Q_OBJECT
public:
	UpdatePackagesWidget (RKLoadLibsDialog *dialog, QWidget *parent);
	
	~UpdatePackagesWidget ();
public slots:
	void updateSelectedButtonClicked ();
	void updateAllButtonClicked ();
	void getListButtonClicked ();
	void ok ();
	void cancel ();
protected:
	void rCommandDone (RCommand *command);
private:
	void updatePackages (const QStringList &list);
	QTreeWidget *updateable_view;
	QTreeWidgetItem *placeholder;

	QPushButton *update_selected_button;
	QPushButton *update_all_button;
	QPushButton *get_list_button;
	PackageInstallParamsWidget *install_params;
	
	RKLoadLibsDialog *parent;
};

/**
Allows the user to install further R packages. For now only packages from CRAN.
Ro be used in RKLoadLibsDialog.

@author Thomas Friedrichsmeier
*/
class InstallPackagesWidget : public QWidget, public RCommandReceiver {
Q_OBJECT
public:
	InstallPackagesWidget (RKLoadLibsDialog *dialog, QWidget *parent);
	
	~InstallPackagesWidget ();
	void trySelectPackage (const QString &package_name);
public slots:
	void installSelectedButtonClicked ();
	void getListButtonClicked ();
	void ok ();
	void cancel ();
protected:
	void rCommandDone (RCommand *command);
private:
	void installPackages (const QStringList &list);
	QTreeWidget *installable_view;
	QTreeWidgetItem *placeholder;

	QPushButton *install_selected_button;
	QPushButton *get_list_button;
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
	PackageInstallParamsWidget (QWidget *parent, bool ask_depends);
	
	~PackageInstallParamsWidget ();

	bool installDependencies ();
	QString libraryLocation ();
	bool checkWritable (bool *as_root);
public slots:
	void liblocsChanged (const QStringList &newlist);
private:
	QComboBox *libloc_selector;
	QCheckBox *dependencies;
};

#endif
