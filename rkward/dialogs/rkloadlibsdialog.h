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

#include "../rbackend/rcommandreceiver.h"

class QListView;
class QPushButton;
class RKErrorDialog;
class QWidget;
class QCloseEvent;
class RCommandChain;
class QCheckBox;

/**
Dialog which excapsulates widgets to load/unload, update and install R packages

@author Thomas Friedrichsmeier
*/

// TODO: add a static member to create (single) instance of the dialog

class RKLoadLibsDialog : public KDialogBase, public RCommandReceiver {
Q_OBJECT
public:
	RKLoadLibsDialog (QWidget *parent, RCommandChain *chain);

	~RKLoadLibsDialog ();
	
	bool downloadPackages (const QStringList &packages, QString to_dir = QString::null);
signals:
	void downloadComplete ();
protected:
	void rCommandDone (RCommand *command);
	void closeEvent (QCloseEvent *e);
protected slots:
	void slotOk ();
	void slotApply ();
	void slotCancel ();
	void childDeleted ();
private:
	void tryDestruct ();
	bool should_destruct;
friend class LoadUnloadWidget;
friend class UpdatePackagesWidget;
	RKErrorDialog *error_dialog;
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
	
	RKLoadLibsDialog *parent;
};


#endif
