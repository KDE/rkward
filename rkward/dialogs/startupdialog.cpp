/***************************************************************************
                          startupdialog  -  description
                             -------------------
    begin                : Thu Aug 26 2004
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
#include "startupdialog.h"

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <QGroupBox>
#include <qlabel.h>
#include <QListWidget>
#include <qstringlist.h>
#include <qpixmap.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klocale.h>
#include <kvbox.h>
#include <krecentfilesaction.h>

#include "../settings/rksettingsmodulegeneral.h"
#include "../misc/rkcommonfunctions.h"
#include "../debug.h"

StartupDialog::StartupDialog (QWidget *parent, StartupDialogResult *result, KRecentFilesAction *recent_files) : KDialog (parent) {
	RK_TRACE (DIALOGS);

	setModal (true);
	setButtons (KDialog::Ok | KDialog::Cancel);

	StartupDialog::result = result;

	setCaption (i18n ("What would you like to do?"));

	KVBox *vbox = new KVBox (this);
	setMainWidget (vbox);
	
	QLabel *pic = new QLabel (vbox);
	pic->setPixmap (QPixmap (RKCommonFunctions::getRKWardDataDir () + "icons/rkward_logo.png"));

	choser = new QButtonGroup (this);
	QGroupBox* choser_box = new QGroupBox (vbox);
	QVBoxLayout*choser_layout = new QVBoxLayout(choser_box);

	choser_layout->addWidget (empty_workspace_button = new QRadioButton (i18n ("Start with an empty workspace"), choser_box));
	choser->addButton (empty_workspace_button);
	choser_layout->addWidget (empty_table_button = new QRadioButton (i18n ("Start with an empty table"), choser_box));
	choser->addButton (empty_table_button);
	choser_layout->addWidget (open_button = new QRadioButton (i18n ("Load an existing workspace:"), choser_box));
	choser->addButton (open_button);
	connect (open_button, SIGNAL (toggled (bool)), this, SLOT (openButtonSelected (bool)));
	empty_table_button->setChecked (true);

	file_list = new QListWidget (choser_box);
	file_list->setSelectionMode (QAbstractItemView::SingleSelection);
	file_list->setSortingEnabled (false);
	chose_file_item = new QListWidgetItem (i18n ("<<Open another file>>"), file_list);
	if (recent_files) {
		file_list->addItems (recent_files->urls ().toStringList ());
	}
	connect (file_list, SIGNAL (itemClicked (QListWidgetItem*)), this, SLOT (listClicked (QListWidgetItem*)));
	connect (file_list, SIGNAL (itemDoubleClicked (QListWidgetItem*)), this, SLOT (listDoubleClicked (QListWidgetItem*)));
	choser_layout->addWidget (file_list);
	choser_layout->addWidget (remember_box = new QCheckBox (i18n ("Always do this on startup"), choser_box));
}

StartupDialog::~StartupDialog() {
	RK_TRACE (DIALOGS);
}

void StartupDialog::accept () {
	RK_TRACE (DIALOGS);

	if (empty_workspace_button->isChecked ()) {
		result->result = EmptyWorkspace;
	} else if (empty_table_button->isChecked ()) {
		result->result = EmptyTable;
	} else if (open_button->isChecked ()) {
		QListWidgetItem *item = file_list->currentItem ();
		if (item == chose_file_item) {
			result->result = ChoseFile;
		} else {
			result->result = OpenFile;
			result->open_url = KUrl (item->text ());
		}
	} else {
		RK_ASSERT (false);
	}
	if (remember_box->isChecked ()) RKSettingsModuleGeneral::setStartupAction (result->result);
	QDialog::accept ();
}

void StartupDialog::reject () {
	RK_TRACE (DIALOGS);
	
	result->result = EmptyWorkspace;
	
	QDialog::reject ();
}

void StartupDialog::listDoubleClicked (QListWidgetItem *item) {
	RK_TRACE (DIALOGS);
	
	if (item) {
		open_button->setChecked (true);
		file_list->setCurrentItem (item);
		item->setSelected (true);
		accept ();
	}
}

void StartupDialog::listClicked (QListWidgetItem *item) {
	RK_TRACE (DIALOGS);
	
	if (item) {
		open_button->setChecked (true);
		openButtonSelected (true);		// always do processing
	}
}

void StartupDialog::openButtonSelected (bool checked) {
	RK_TRACE (DIALOGS);

	if (checked) {
		if (!file_list->currentItem ()) {
			file_list->setCurrentRow (0);
		}
		if (file_list->currentItem () != chose_file_item) {
			remember_box->setChecked (false);
			remember_box->setEnabled (false);
		} else {
			remember_box->setEnabled (true);
		}
	} else {
		remember_box->setEnabled (true);
	}
}

void StartupDialog::showEvent (QShowEvent *event) {
	RK_TRACE (DIALOGS);

	// somehow, trying to achieve this in the ctor leads to the dialog never actually being shown (KDE4.0 beta)
	setFixedWidth (width ());
	KDialog::showEvent (event);
}

//static
StartupDialog::StartupDialogResult *StartupDialog::getStartupAction (QWidget *parent, KRecentFilesAction *recent_files) {
	RK_TRACE (DIALOGS);

	StartupDialogResult *result = new StartupDialogResult;
	result->result = RKSettingsModuleGeneral::startupAction ();

	if (result->result != NoSavedSetting) {
		return result;
	}

	StartupDialog *dialog = new StartupDialog (parent, result, recent_files);
	dialog->exec ();
	delete dialog;
	
	RK_DO (qDebug ("startup-dialog result: %d, url: %s", result->result, result->open_url.fileName ().toLatin1 ().data ()), DIALOGS, DL_DEBUG);
	
	return result;
}

#include "startupdialog.moc"
