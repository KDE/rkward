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
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qstringlist.h>
#include <qpixmap.h>

#include <kdeversion.h>
#if !KDE_IS_VERSION (3, 2,0)
	#include <kaction.h>
#else
	#include <kactionclasses.h>
#endif
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "../debug.h"

StartupDialog::StartupDialog (QWidget *parent, StartupDialogResult *result, KRecentFilesAction *recent_files) : QDialog (parent, 0, true) {
	RK_TRACE (DIALOGS);
	StartupDialog::result = result;

	QVBoxLayout *vbox = new QVBoxLayout (this);
	
	logo = new QPixmap (KGlobal::dirs ()->findResourceDir ("data", "rkward/phpfiles/common.php") + "rkward/icons/rkward_logo.png");
	QLabel *pic = new QLabel (this);
	pic->setPixmap (*logo);
	vbox->addWidget (pic);

	choser = new QButtonGroup (this);
	choser->setColumnLayout (0, Qt::Vertical);
	choser->layout()->setSpacing (6);
	choser->layout()->setMargin (11);
	QVBoxLayout *choser_layout = new QVBoxLayout(choser->layout());
	choser_layout->addWidget (new QRadioButton (i18n ("Start with an empty workspace"), choser));
	choser_layout->addWidget (new QRadioButton (i18n ("Start with an empty table"), choser));
	choser_layout->addWidget (new QRadioButton (i18n ("Load an existing workspace:"), choser));
	choser->setButton (static_cast<int> (EmptyTable));

	file_list = new QListView (choser);
	file_list->addColumn (i18n ("Filename"));
	chose_file_item = new QListViewItem (file_list, i18n ("<<Open another file>>"));
	if (recent_files) {
		QStringList items = recent_files->items ();
		for (QStringList::iterator it = items.begin (); it != items.end (); ++it) {
			if (!(*it).isEmpty ()) new QListViewItem (file_list, (*it));
		}
	}
	connect (file_list, SIGNAL (clicked (QListViewItem *)), this, SLOT (listClicked (QListViewItem*)));
	connect (file_list, SIGNAL (doubleClicked (QListViewItem *, const QPoint &, int)), this, SLOT (listDoubleClicked (QListViewItem*, const QPoint &, int)));
	choser_layout->addWidget (file_list);
	
	vbox->addWidget (choser);
	
	QHBoxLayout *button_hbox = new QHBoxLayout (vbox);
	ok_button = new QPushButton (i18n ("Ok"), this);
	connect (ok_button, SIGNAL (clicked ()), this, SLOT (accept ()));
	button_hbox->addWidget (ok_button);
	button_hbox->addStretch ();
	
	cancel_button = new QPushButton (i18n ("Cancel"), this);
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (reject ()));
	button_hbox->addWidget (cancel_button);

	setCaption (i18n ("What would you like to do?"));
	
	setFixedWidth (minimumWidth ());
}

StartupDialog::~StartupDialog() {
	RK_TRACE (DIALOGS);
	delete logo;
}

void StartupDialog::accept () {
	RK_TRACE (DIALOGS);
#if QT_VERSION < 0x030200
	int selected = choser->id (choser->selected ());
#else
	int selected = choser->selectedId ();
#endif

	if (selected == (int) EmptyWorkspace) {
		result->result = EmptyWorkspace;
	} else if (selected == (int) EmptyTable) {
		result->result = EmptyTable;
	} else if (selected == (int) OpenFile) {
		QListViewItem *item = file_list->selectedItem ();
		if (item == chose_file_item) {
			result->result = ChoseFile;
		} else {
			result->result = OpenFile;
			result->open_url = KURL (item->text (0));
		}
	} else {
		RK_ASSERT (false);
	}
	QDialog::accept ();
}

void StartupDialog::reject () {
	RK_TRACE (DIALOGS);
	
	result->result = EmptyWorkspace;
	
	QDialog::reject ();
}

void StartupDialog::listDoubleClicked (QListViewItem *item, const QPoint &, int) {
	RK_TRACE (DIALOGS);
	
	if (item) {
		choser->setButton ((int) OpenFile);
		item->setSelected (true);
		accept ();
	}
}

void StartupDialog::listClicked (QListViewItem *item) {
	RK_TRACE (DIALOGS);
	
	if (item) choser->setButton ((int) OpenFile);
}

//static
StartupDialog::StartupDialogResult *StartupDialog::getStartupAction (QWidget *parent, KRecentFilesAction *recent_files) {
	RK_TRACE (DIALOGS);
	StartupDialogResult *result = new StartupDialogResult;
	result->result = EmptyWorkspace;

	StartupDialog *dialog = new StartupDialog (parent, result, recent_files);
	dialog->exec ();
	delete dialog;
	
	RK_DO (qDebug ("startup-dialog result: %d, url: %s", result->result, result->open_url.fileName ().latin1 ()), DIALOGS, DL_DEBUG);
	
	return result;
}

#include "startupdialog.moc"
