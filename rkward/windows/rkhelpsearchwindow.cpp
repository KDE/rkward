/***************************************************************************
                          rkhelpsearchwindow  -  description
                             -------------------
    begin                : Fri Feb 25 2005
    copyright            : (C) 2005, 2006, 2007 by Thomas Friedrichsmeier
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

#include "rkhelpsearchwindow.h"

#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <q3listview.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QFocusEvent>
#include <Q3VBoxLayout>

#include "../rbackend/rinterface.h"
#include "../rbackend/rcommandreceiver.h"
#include "../debug.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkdummypart.h"

#define GET_HELP_URL 1
#define HELP_SEARCH 2
#define GET_INSTALLED_PACKAGES 3

RKHelpSearchWindow* RKHelpSearchWindow::main_help_search = 0;

RKHelpSearchWindow::RKHelpSearchWindow (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, SearchHelpWindow, tool_window, name) {
	RK_TRACE (APP);
	setPart (new RKDummyPart (0, this));
	initializeActivationSignals ();
	setFocusPolicy (QWidget::ClickFocus);

	Q3VBoxLayout* main_layout = new Q3VBoxLayout (this, RKGlobals::marginHint (), RKGlobals::spacingHint ());
	Q3HBoxLayout* selection_layout = new Q3HBoxLayout (main_layout, RKGlobals::spacingHint ());

	Q3VBoxLayout* labels_layout = new Q3VBoxLayout (selection_layout, RKGlobals::spacingHint ());
	QLabel *label = new QLabel (i18n ("Find:"), this);
	label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Minimum);
	labels_layout->addWidget (label);
	label = new QLabel (i18n ("Fields:"), this);
	label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Minimum);
	labels_layout->addWidget (label);

	Q3VBoxLayout* main_settings_layout = new Q3VBoxLayout (selection_layout, RKGlobals::spacingHint ());
	field = new QComboBox (true, this);
	field->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	connect (field->lineEdit () , SIGNAL (returnPressed ()), this, SLOT (slotFindButtonClicked ()));
	main_settings_layout->addWidget (field);

	Q3HBoxLayout* fields_packages_layout = new Q3HBoxLayout (main_settings_layout, RKGlobals::spacingHint ());
	fieldsList = new QComboBox (false, this);
	// HACK the sequence of options is hardcoded, do not modify
	fieldsList->insertItem (i18n("All"));
	fieldsList->insertItem (i18n("All but keywords"));
	fieldsList->insertItem (i18n("Keywords"));
	fieldsList->insertItem (i18n("Title"));
	fields_packages_layout->addWidget (fieldsList);

	label = new QLabel (i18n ("Package:"), this);
	label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Minimum);
	fields_packages_layout->addWidget (label);

	packagesList = new QComboBox (false, this);
	packagesList->insertItem (i18n("All"));
	fields_packages_layout->addWidget (packagesList);

	Q3VBoxLayout* checkboxes_layout = new Q3VBoxLayout (selection_layout, RKGlobals::spacingHint ());
	caseSensitiveCheckBox = new QCheckBox (i18n ("Case sensitive"), this);
	checkboxes_layout->addWidget (caseSensitiveCheckBox);
	fuzzyCheckBox = new QCheckBox (i18n ("Fuzzy matching"), this);
	fuzzyCheckBox->setChecked (true);
	checkboxes_layout->addWidget (fuzzyCheckBox);

	findButton = new QPushButton (i18n ("Find"), this);
	findButton->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect (findButton, SIGNAL (clicked ()), this, SLOT (slotFindButtonClicked ()));
	selection_layout->addWidget (findButton);

	resultsList = new Q3ListView (this);
	resultsList->addColumn (i18n ("Topic"));
	resultsList->addColumn (i18n ("Title"));
	resultsList->addColumn (i18n ("Package"));
	connect (resultsList, SIGNAL (doubleClicked (Q3ListViewItem*, const QPoint&, int)), this, SLOT (slotResultsListDblClicked (Q3ListViewItem*, const QPoint&, int)));
	main_layout->addWidget (resultsList);

	RKGlobals::rInterface ()->issueCommand (".rk.get.installed.packages ()[[1]]", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_INSTALLED_PACKAGES, 0);

	setCaption (i18n ("Help search"));
}

RKHelpSearchWindow::~RKHelpSearchWindow () {
	RK_TRACE (APP);
}

void RKHelpSearchWindow::focusInEvent (QFocusEvent *e) {
	RK_TRACE (APP);

	RKMDIWindow::focusInEvent (e);
	if (e->reason () != QFocusEvent::Mouse) {
		field->setFocus ();
	}
}

void RKHelpSearchWindow::getContextHelp (const QString &context_line, int cursor_pos) {
	RK_TRACE (APP);
	QString result = RKCommonFunctions::getCurrentSymbol (context_line, cursor_pos);
	if (result.isEmpty ()) return;

	getFunctionHelp (result);
}

void RKHelpSearchWindow::getFunctionHelp (const QString &function_name) {
	RK_TRACE (APP);
	RKGlobals::rInterface ()->issueCommand ("help(\"" + function_name + "\", htmlhelp=TRUE)[1]", RCommand::App | RCommand::GetStringVector, QString::null, this, GET_HELP_URL, 0);
}

void RKHelpSearchWindow::slotFindButtonClicked () {
	RK_TRACE (APP);

	if (field->currentText ().isEmpty ()) {
		return;
	}
	
	QString agrep = "FALSE";
	if (fuzzyCheckBox->isChecked ()) {
		agrep="NULL";
	}
	
	QString ignoreCase = "TRUE";
	if(caseSensitiveCheckBox->isChecked ()) {
		ignoreCase="FALSE";
	}
	
	QString package = "NULL";
	if (packagesList->currentItem ()!=0) {
		package="\"";
		package.append (packagesList->currentText ());
		package.append ("\"");
	}

	// HACK the sequence of options is hardcoded, do not modify
	QString fields;
	
	switch (fieldsList->currentItem ()) {
		case 1: fields = "c(\"alias\", \"concept\", \"title\")";break;
		case 2: fields = "c(\"keyword\")";break;
		case 3: fields = "c(\"title\")";break;
		default: fields = "c(\"alias\", \"concept\", \"title\",\"keyword\")";
	}

	QString s = ".rk.get.search.results (\"" + field->currentText () + "\",agrep=" + agrep + ", ignore.case=" + ignoreCase + ", package=" + package + ", fields=" + fields +")";
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, HELP_SEARCH, 0);
	setEnabled (false);
	field->insertItem (field->currentText ());
}

void RKHelpSearchWindow::slotResultsListDblClicked (Q3ListViewItem * item, const QPoint &, int) {
	RK_TRACE (APP);
	if (item == NULL) {
		return;
	}
	if (item->text(0).isEmpty ()) {
		return;
	}
	
	QString s="help(\"";
	s.append (item->text (0));
	s.append ("\", htmlhelp=TRUE, package= \"");
	s.append (item->text (2));
	s.append ("\")");
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_HELP_URL, 0);
}

void RKHelpSearchWindow::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	KURL url;
	if (command->getFlags () == HELP_SEARCH) {
		resultsList->clear ();
		RK_ASSERT ((command->getDataLength () % 3) == 0);
		int count = (command->getDataLength () / 3);
		for (int i=0; i < count; ++i) {
			new Q3ListViewItem (resultsList, command->getStringVector ()[i], command->getStringVector ()[count + i], command->getStringVector ()[2*count + i]);
		}
		setEnabled(true);
	} else if (command->getFlags () == GET_HELP_URL) {
		RK_ASSERT (command->getDataLength ());
		url.setPath(command->getStringVector ()[0]);
		if (QFile::exists (url.path ())) {
			RKWardMainWindow::getMain ()->openHTML (url);
			return;
		} else {
			KMessageBox::sorry (this, i18n ("No help found on '%1'. Maybe the corresponding package is not installed/loaded, or maybe you mistyped the command. Try using Help->Search R Help for more options.").arg (command->command ().section ("\"", 1, 1)), i18n ("No help found"));
		}
	} else if (command->getFlags () == GET_INSTALLED_PACKAGES) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		unsigned int count = command->getDataLength ();
		for (unsigned int i=0; i < count; ++i) {
			packagesList->insertItem (command->getStringVector ()[i]);
		}
	} else {
		RK_ASSERT (false);
	}
}

#include "rkhelpsearchwindow.moc"
