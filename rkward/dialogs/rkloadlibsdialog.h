/***************************************************************************
                          rkloadlibsdialog  -  description
                             -------------------
    begin                : Mon Sep 6 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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

#include <kdialogbase.h>

#include <qstringlist.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "../settings/rksettingsmoduler.h"
#include "../rbackend/rcommandreceiver.h"

class Q3ListView;
class QComboBox;
class QPushButton;
class RKProgressControl;
class QWidget;
class QCloseEvent;
class RCommandChain;
class QCheckBox;
class K3Process;
class PackageInstallParamsWidget;
class InstallPackagesWidget;

/**
Dialog which excapsulates widgets to load/unload, update and install R packages

@author Thomas Friedrichsmeier
*/

// TODO: add a static member to create (single) instance of the dialog

class RKLoadLibsDialog : public KDialogBase, public RCommandReceiver {
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
	void slotOk ();
	void slotApply ();
	void slotCancel ();
/** User1-button was clicked, i.e.: "Configure Repositories" */
	void slotUser1 ();
	void childDeleted ();
	void processExited (K3Process *);
	void installationProcessOutput (K3Process *proc, char *buffer, int buflen);
	void installationProcessError (K3Process *proc, char *buffer, int buflen);
	void automatedInstall ();
private:
	void tryDestruct ();
friend class LoadUnloadWidget;
friend class UpdatePackagesWidget;
friend class InstallPackagesWidget;
	RCommandChain *chain;

	InstallPackagesWidget *install_packages_widget;	// needed for automated installation

	QString auto_install_package;
	int num_child_widgets;
	bool accepted;
	QString repos_string;
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
	
	Q3ListView *loaded_view;
	Q3ListView *installed_view;

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
	Q3ListView *updateable_view;
	Q3ListViewItem *placeholder;

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
	Q3ListView *installable_view;
	Q3ListViewItem *placeholder;

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
