/***************************************************************************
                          khelpdlg  -  description
                             -------------------
    begin                : Fri Feb 25 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

#include "khelpdlg.h"

#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qlabel.h>

#include "rbackend/rinterface.h"
#include "rbackend/rcommandreceiver.h"
#include "debug.h"
#include "rkglobals.h"
#include "rkward.h"

#define GET_HELP_URL 1
#define HELP_SEARCH 2
#define GET_INSTALLED_PACKAGES 3

KHelpDlg::KHelpDlg (QWidget* parent, const char* name) : QWidget (parent, name) {
	QVBoxLayout* main_layout = new QVBoxLayout (this, RKGlobals::marginHint (), RKGlobals::spacingHint ());
	QHBoxLayout* selection_layout = new QHBoxLayout (main_layout, RKGlobals::spacingHint ());


	QVBoxLayout* labels_layout = new QVBoxLayout (selection_layout, RKGlobals::spacingHint ());
	QLabel *label = new QLabel (i18n ("Find:"), this);
	label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Minimum);
	labels_layout->addWidget (label);
	label = new QLabel (i18n ("Fields:"), this);
	label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Minimum);
	labels_layout->addWidget (label);


	QVBoxLayout* main_settings_layout = new QVBoxLayout (selection_layout, RKGlobals::spacingHint ());
	field = new QComboBox (true, this);
	field->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	connect (field->lineEdit () , SIGNAL (returnPressed ()), this, SLOT (slotFindButtonClicked ()));
	main_settings_layout->addWidget (field);

	QHBoxLayout* fields_packages_layout = new QHBoxLayout (main_settings_layout, RKGlobals::spacingHint ());
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


	QVBoxLayout* checkboxes_layout = new QVBoxLayout (selection_layout, RKGlobals::spacingHint ());
	caseSensitiveCheckBox = new QCheckBox (i18n ("Case sensitive"), this);
	checkboxes_layout->addWidget (caseSensitiveCheckBox);
	fuzzyCheckBox = new QCheckBox (i18n ("Fuzzy matching"), this);
	fuzzyCheckBox->setChecked (true);
	checkboxes_layout->addWidget (fuzzyCheckBox);

	findButton = new QPushButton (i18n ("Find"), this);
	findButton->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect (findButton, SIGNAL (clicked ()), this, SLOT (slotFindButtonClicked ()));
	selection_layout->addWidget (findButton);

	resultsList = new QListView (this);
	resultsList->addColumn (i18n ("Topic"));
	resultsList->addColumn (i18n ("Title"));
	resultsList->addColumn (i18n ("Package"));
	connect (resultsList, SIGNAL (doubleClicked (QListViewItem*, const QPoint&, int)), this, SLOT (slotResultsListDblClicked (QListViewItem*, const QPoint&, int)));
	main_layout->addWidget (resultsList);


	RKGlobals::rInterface ()->issueCommand (".rk.get.installed.packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_INSTALLED_PACKAGES, 0);

	setCaption (i18n ("Help search"));
}

KHelpDlg::~KHelpDlg()
{}

void KHelpDlg::getContextHelp (const QString &context_line, int cursor_pos) {
	if (context_line.isEmpty () || context_line.isNull ()) return;

	// step 1: find out word under cursor
	// We want to match any valid R name, that is, everything composed of letters, 0-9, '.'s and '_'s..
	QRegExp rx_no_word ("[^A-Za-z0-9\\._]");

	// find out the next non-word stuff left and right of the current cursor position
	int current_word_start = context_line.findRev (rx_no_word, cursor_pos-1) + 1;
	int current_word_end = context_line.find (rx_no_word, cursor_pos);

	// if both return the same position, we're on a non-word.
	if (current_word_start == current_word_end) return;

	QString result = context_line.mid (current_word_start, current_word_end - current_word_start);

	// step 2: retrieve help
	RKGlobals::rInterface ()->issueCommand ("help(\"" + result + "\", htmlhelp=TRUE)[1]", RCommand::App | RCommand::GetStringVector, QString::null, this, GET_HELP_URL, 0);
}

/*$SPECIALIZATION$*/
void KHelpDlg::slotFindButtonClicked()
{

	if (field->currentText().isEmpty()) {
		return;
	}
	
	QString agrep = "FALSE";
	if(fuzzyCheckBox->isChecked ()==TRUE){
		agrep="NULL";
	}
	
	QString ignoreCase = "TRUE";
	if(caseSensitiveCheckBox->isChecked ()==TRUE){
		ignoreCase="FALSE";
	}
	
	QString package = "NULL";
	if(packagesList->currentItem()!=0){
		package="\"";
		package.append(packagesList->currentText());
		package.append("\"");
	}

	// HACK the sequence of options is hardcoded, do not modify
	QString fields;
	
	switch(fieldsList->currentItem()){
		case 1: fields = "c(\"alias\", \"concept\", \"title\")";break;
		case 2: fields = "c(\"keyword\")";break;
		case 3: fields = "c(\"title\")";break;
		default: fields = "c(\"alias\", \"concept\", \"title\",\"keyword\")";
			
	}
	
	
	QString s = ".rk.get.search.results(\"" +field->currentText() +"\",agrep=" 
		+ agrep +", ignore.case=" + ignoreCase + ", package=" + package +", fields=" + fields +")";
		
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, HELP_SEARCH, 0);
	setEnabled(false);
	field->insertItem(field->currentText());
	
	
}

void KHelpDlg::slotResultsListDblClicked( QListViewItem * item, const QPoint &, int )
{
	if (item == NULL) {
		return;
	}
	if (item->text(0).isEmpty()) {
		return;
	}
	
	chain=0;
	QString s="help(\"";
	s.append(item->text(0));
	s.append("\", htmlhelp=TRUE, package= \"");
	s.append(item->text(2));
	s.append("\")");
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_HELP_URL, 0);
}


void KHelpDlg::rCommandDone (RCommand *command) {
	KURL url;
	if (command->getFlags () == HELP_SEARCH) {
		resultsList->clear();
		RK_ASSERT ((command->stringVectorLength () % 3) == 0);
		int count = (command->stringVectorLength () / 3);
		for (int i=0; i < count; ++i) {
			new QListViewItem (resultsList, command->getStringVector ()[i], command->getStringVector ()[count + i], command->getStringVector ()[2*count + i]);
		}
		setEnabled(true);
	} 
	else if (command->getFlags () == GET_HELP_URL) {
		RK_ASSERT (command->stringVectorLength ());
		url.setPath(command->getStringVector ()[0]);
		if (QFile::exists (url.path ())) {
			RKGlobals::rkApp()->openHTML(url);
			return;
		} else {
			KMessageBox::sorry (this, i18n ("No help found on '%1'. Maybe the corresponding package is not installed/loaded, or maybe you mistyped the command. Try using Help->Search R Help for more options.").arg (command->command ().section ("\"", 1, 1)), i18n ("No help found"));
		}
	} else if (command->getFlags () == GET_INSTALLED_PACKAGES) {
		RK_ASSERT ((command->stringVectorLength () % 4) == 0);
		int count = (command->stringVectorLength () / 4);
		for (int i=0; i < count; ++i) {
			packagesList->insertItem(command->getStringVector ()[i]);
		}
	}else {
		RK_ASSERT (false);
	}
}



#include "khelpdlg.moc"

