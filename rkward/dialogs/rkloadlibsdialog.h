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

/**
Dialog which excapsulates widgets to load/unload, update and install R packages

@author Thomas Friedrichsmeier
*/

// TODO: add a static member to create (single) instance of the dialog

class RKLoadLibsDialog : public KDialogBase {
Q_OBJECT
public:
	RKLoadLibsDialog (QWidget *parent, RCommandChain *chain);

	~RKLoadLibsDialog ();
protected:
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
	RKErrorDialog *error_dialog;
	RCommandChain *chain;
	int num_child_widgets;
	bool accepted;
};

/**
Shows which packages are available (installed) / loaded, and lets the user load or detach packages.

@author Thomas Friedrichsmeier
*/
class LoadUnloadWidget : public QWidget, public RCommandReceiver {
Q_OBJECT
public:
	LoadUnloadWidget (RKLoadLibsDialog *parent);
	
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

#endif
