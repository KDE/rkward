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

#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <q3buttongroup.h>
#include <qlayout.h>
#include <qlabel.h>
#include <q3listview.h>
#include <qstringlist.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <kdeversion.h>
#if !KDE_IS_VERSION (3, 2,0)
	#include <kaction.h>
#else
	#include <kactionclasses.h>
#endif
#include <klocale.h>

#include "../settings/rksettingsmodulegeneral.h"
#include "../misc/rkcommonfunctions.h"
#include "../debug.h"

StartupDialog::StartupDialog (QWidget *parent, StartupDialogResult *result, KRecentFilesAction *recent_files) : QDialog (parent, 0, true) {
	RK_TRACE (DIALOGS);
	StartupDialog::result = result;

	setCaption (i18n ("What would you like to do?"));

	Q3VBoxLayout *vbox = new Q3VBoxLayout (this);
	
	logo = new QPixmap (RKCommonFunctions::getRKWardDataDir () + "icons/rkward_logo.png");
	QLabel *pic = new QLabel (this);
	pic->setPixmap (*logo);
	vbox->addWidget (pic);

	choser = new Q3ButtonGroup (this);
	choser->setColumnLayout (0, Qt::Vertical);
	choser->layout()->setSpacing (6);
	choser->layout()->setMargin (11);
	Q3VBoxLayout *choser_layout = new Q3VBoxLayout(choser->layout());
	choser_layout->addWidget (empty_workspace_button = new QRadioButton (i18n ("Start with an empty workspace"), choser));
	choser_layout->addWidget (empty_table_button = new QRadioButton (i18n ("Start with an empty table"), choser));
	open_button = new QRadioButton (i18n ("Load an existing workspace:"), choser);
	connect (open_button, SIGNAL (stateChanged (int)), this, SLOT (openButtonSelected (int)));
	empty_table_button->setChecked (true);
	choser_layout->addWidget (open_button);

	file_list = new Q3ListView (choser);
	file_list->addColumn (i18n ("Filename"));
	file_list->setSorting (-1);
	chose_file_item = new Q3ListViewItem (file_list, i18n ("<<Open another file>>"));
	if (recent_files) {
		QStringList items = recent_files->items ();
		for (QStringList::iterator it = items.begin (); it != items.end (); ++it) {
			if (!(*it).isEmpty ()) new Q3ListViewItem (file_list, (*it));
		}
	}
	connect (file_list, SIGNAL (selectionChanged (Q3ListViewItem *)), this, SLOT (listClicked (Q3ListViewItem*)));
	connect (file_list, SIGNAL (doubleClicked (Q3ListViewItem *, const QPoint &, int)), this, SLOT (listDoubleClicked (Q3ListViewItem*, const QPoint &, int)));
	choser_layout->addWidget (file_list);
	choser_layout->addWidget (remember_box = new QCheckBox (i18n ("Always do this on startup"), choser));
	
	vbox->addWidget (choser);
	
	Q3HBoxLayout *button_hbox = new Q3HBoxLayout (vbox);
	ok_button = new QPushButton (i18n ("Ok"), this);
	connect (ok_button, SIGNAL (clicked ()), this, SLOT (accept ()));
	button_hbox->addWidget (ok_button);
	button_hbox->addStretch ();
	
	cancel_button = new QPushButton (i18n ("Cancel"), this);
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (reject ()));
	button_hbox->addWidget (cancel_button);

	setFixedWidth (minimumWidth ());
}

StartupDialog::~StartupDialog() {
	RK_TRACE (DIALOGS);
	delete logo;
}

void StartupDialog::accept () {
	RK_TRACE (DIALOGS);

	if (empty_workspace_button->isChecked ()) {
		result->result = EmptyWorkspace;
	} else if (empty_table_button->isChecked ()) {
		result->result = EmptyTable;
	} else if (open_button->isChecked ()) {
		Q3ListViewItem *item = file_list->selectedItem ();
		if (item == chose_file_item) {
			result->result = ChoseFile;
		} else {
			result->result = OpenFile;
			result->open_url = KUrl (item->text (0));
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

void StartupDialog::listDoubleClicked (Q3ListViewItem *item, const QPoint &, int) {
	RK_TRACE (DIALOGS);
	
	if (item) {
		open_button->setChecked (true);
		item->setSelected (true);
		accept ();
	}
}

void StartupDialog::listClicked (Q3ListViewItem *item) {
	RK_TRACE (DIALOGS);
	
	if (item) {
		open_button->setChecked (true);
		openButtonSelected (QButton::On);		// always do processing
	}
}

void StartupDialog::openButtonSelected (int state) {
	RK_TRACE (DIALOGS);

	if (state == QButton::On) {
		if (!file_list->selectedItem ()) {
			file_list->setSelected (file_list->firstChild (), true);
		}
		if (file_list->selectedItem () != chose_file_item) {
			remember_box->setChecked (false);
			remember_box->setEnabled (false);
		} else {
			remember_box->setEnabled (true);
		}
	} else if (state == QButton::Off) {
		remember_box->setEnabled (true);
	}
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
	
	RK_DO (qDebug ("startup-dialog result: %d, url: %s", result->result, result->open_url.fileName ().toLatin1 ()), DIALOGS, DL_DEBUG);
	
	return result;
}

#include "startupdialog.moc"
