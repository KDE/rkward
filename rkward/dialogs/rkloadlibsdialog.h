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
#ifndef RKLOADLIBSDIALOG_H
#define RKLOADLIBSDIALOG_H

#include <kdialogbase.h>

#include <qstringlist.h>

#include "../settings/rksettingsmoduler.h"
#include "../rbackend/rcommandreceiver.h"

class QListView;
class QComboBox;
class QPushButton;
class RKErrorDialog;
class QWidget;
class QCloseEvent;
class RCommandChain;
class QCheckBox;
class KProcess;
class PackageInstallParamsWidget;

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
	
	bool downloadPackages (const QStringList &packages);
	void installDownloadedPackages (bool become_root);
	bool installPackages (const QStringList &packages, const QString &to_libloc, bool install_dependencies);

	/** opens a modal RKLoadLibsDialog with the "Install new Packages" tab on front (To be used when a require () fails in the R backend */
	static void showInstallPackagesModal (QWidget *parent, RCommandChain *chain);
signals:
	void downloadComplete ();
	void installationComplete ();
	void libraryLocationsChanged (const QStringList &);
protected:
	void rCommandDone (RCommand *command);
	void closeEvent (QCloseEvent *e);
/** reimplemented from RCommandReceiver to call QObject::deleteLater () instead of delete this. Not entirely sure, what's going on, but otherwise we
get a double deletion -> crash */
	void deleteThisNow ();
protected slots:
	void slotOk ();
	void slotApply ();
	void slotCancel ();
/** User1-button was clicked, i.e.: "Configure Repositories" */
	void slotUser1 ();
	void childDeleted ();
	void processExited (KProcess *);
	void installationProcessOutput (KProcess *proc, char *buffer, int buflen);
private:
	void tryDestruct ();
friend class LoadUnloadWidget;
friend class UpdatePackagesWidget;
friend class InstallPackagesWidget;
	RKErrorDialog *error_dialog;
	RKErrorDialog *installation_error_dialog;
	RCommandChain *chain;
	int num_child_widgets;
	bool accepted;
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
public slots:
	void loadButtonClicked ();
	void detachButtonClicked ();
	void ok ();
	void apply ();
	void cancel ();
protected:
	void rCommandDone (RCommand *command);
private:
	void updateCurrentList ();
	void doLoadUnload ();
	
	QListView *loaded_view;
	QListView *installed_view;

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
signals:
	void actionDone ();
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
	QListView *updateable_view;
	QListViewItem *placeholder;

	QPushButton *update_selected_button;
	QPushButton *update_all_button;
	QPushButton *get_list_button;
	QCheckBox *become_root_box;
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
signals:
	void actionDone ();
public slots:
	void installSelectedButtonClicked ();
	void getListButtonClicked ();
	void ok ();
	void cancel ();
protected:
	void rCommandDone (RCommand *command);
private:
	void installPackages (const QStringList &list);
	QListView *installable_view;
	QListViewItem *placeholder;

	QPushButton *install_selected_button;
	QPushButton *get_list_button;
	QCheckBox *become_root_box;
	
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
	bool checkWritable ();
public slots:
	void liblocsChanged (const QStringList &newlist);
private:
	QComboBox *libloc_selector;
	QCheckBox *dependencies;
};

#endif
