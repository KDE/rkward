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

/**
Shows which packages are available (installed) / loaded, and lets the user load or detach packages.

@author Thomas Friedrichsmeier
*/

// TODO: add a static member to create (single) instance of the dialog

class RKLoadLibsDialog : public KDialogBase, public RCommandReceiver {
Q_OBJECT
public:
	RKLoadLibsDialog (QWidget *parent = 0);

	~RKLoadLibsDialog ();
public slots:
	void loadButtonClicked ();
	void detachButtonClicked ();
protected:
	void rCommandDone (RCommand *command);
	void closeEvent (QCloseEvent *e);
protected slots:
	void slotOk ();
	void slotApply ();
	void slotCancel ();
private:
	void updateCurrentList ();
	void doLoadUnload ();

	QWidget *widget;

	QListView *loaded_view;
	QListView *installed_view;

	QPushButton *load_button;
	QPushButton *detach_button;
	
	QStringList prev_packages;
	
	RKErrorDialog *error_dialog;
};

#endif
