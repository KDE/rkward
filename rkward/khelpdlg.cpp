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
#include <qtimer.h>

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
	
	// HACK: apparantly, we have to wait a little bit before we lauch an R command. So we wait half a second.
        QTimer *timer = new QTimer (this);
        connect(timer, SIGNAL(timeout ()), this, SLOT(slotTimerDone ()));
        timer->start(2000, TRUE); // 0.5 seconds single-shot timer
	
	
	// HACK again: it looks like we need to issue a command here?!
	RKGlobals::rInterface ()->issueCommand ("cat("")");
	
}

KHelpDlg::~KHelpDlg()
{}

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
	
	
	QString s = ".rk.get.search.results(\"";
	s.append(field->currentText());
	s.append("\",agrep=");
	s.append(agrep);
	s.append(", ignore.case=");
	s.append(ignoreCase);
	s.append(", package=");
	s.append(package);
	s.append(")");
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, HELP_SEARCH, chain);
	
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
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_HELP_URL, chain);
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
	} 
	else if (command->getFlags () == GET_HELP_URL) {
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





/*!
    \fn KHelpDlg::slotTimerDone ()
    
    We use a timer to load the package list after a little while. We souln't have to do this.
 */
void KHelpDlg::slotTimerDone ()
{
    //RKGlobals::rInterface ()->issueCommand (".rk.get.installed.packages ()", RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_INSTALLED_PACKAGES, chain);
}
