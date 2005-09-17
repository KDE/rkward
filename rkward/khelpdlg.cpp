/***************************************************************************
                          rkward.h  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002 
    copyright            : (C) 2002 by Thomas Friedrichsmeier 
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

 // To be removed :
#include <kmessagebox.h>
 
 
#include <klocale.h>
#include "kurl.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qregexp.h>

#include "rbackend/rinterface.h"
#include "rbackend/rcommandreceiver.h"
#include "debug.h"
#include "rkglobals.h"
#include "rkward.h"

#include "khelpdlg.h"
#include <kmessagebox.h>

#define GET_HELP_URL 1
#define HELP_SEARCH 2
#define GET_INSTALLED_PACKAGES 3

KHelpDlg::KHelpDlg(QWidget* parent, const char* name, bool modal, WFlags fl)
    : helpDlg(parent,name, modal,fl)
{
	resultsList->clear();
	resultsList->removeColumn(0);
	
	resultsList->addColumn (i18n ("Topic"));
	resultsList->addColumn (i18n ("Title"));
	resultsList->addColumn (i18n ("Package"));
	packagesList->insertItem (i18n("All"));

	// HACK the following is hardcoded, do not modify
	fieldsList->insertItem (i18n("All"));
	fieldsList->insertItem (i18n("All but keywords"));
	fieldsList->insertItem (i18n("Keywords"));
	fieldsList->insertItem (i18n("Title"));

	QLineEdit *edit=field->lineEdit();

	connect(edit, SIGNAL(returnPressed()), this, SLOT(slotFieldReturnPressed ()));


	RKGlobals::rInterface ()->issueCommand (".rk.get.installed.packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_INSTALLED_PACKAGES, 0);


	
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
	RKGlobals::rInterface ()->issueCommand ("help(\"" + result + "\", htmlhelp=TRUE)[1]", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_HELP_URL, 0);
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

	// HACK the following is hardcoded, do not modify
	QString fields="";
	
	switch(fieldsList->currentItem()){
		case 1: fields = "c(\"alias\", \"concept\", \"title\")";break;
		case 2: fields = "c(\"keyword\")";break;
		case 3: fields = "c(\"title\")";break;
		default: fields = "c(\"alias\", \"concept\", \"title\",\"keyword\")";
			
	}
	
	
	QString s = ".rk.get.search.results(\"" +field->currentText() +"\",agrep=" 
		+ agrep +", ignore.case=" + ignoreCase + ", package=" + package +", fields=" + fields +")";
		
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, HELP_SEARCH, 0);
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
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_HELP_URL, 0);
}

void KHelpDlg::slotPackageListActivated()
{

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
		if (QFile::exists( url.path() )) {
			RKGlobals::rkApp()->openHTML(url);
			return;
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




void KHelpDlg::slotFieldReturnPressed (  )
{
	slotFindButtonClicked ();
}

