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

#include <klocale.h>
#include "kurl.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlistview.h>

#include "rbackend/rinterface.h"
#include "debug.h"
#include "rkglobals.h"
#include "rkward.h"

#include "khelpdlg.h"

#define GET_HELP_URL 1
#define HELP_SEARCH 2


KHelpDlg::KHelpDlg(QWidget* parent, const char* name, bool modal, WFlags fl)
    : helpDlg(parent,name, modal,fl)
{
	resultsList->clear();
	// HACK : I should'nt have to do that.
	resultsList->removeColumn(0);
	resultsList->addColumn (i18n ("Topic"));
	resultsList->addColumn (i18n ("Title"));
	resultsList->addColumn (i18n ("Package"));
}

KHelpDlg::~KHelpDlg()
{}

/*$SPECIALIZATION$*/
void KHelpDlg::slotFindButtonClicked()
{
	QString agrep = "FALSE";
	QString ignoreCase = "TRUE";
	if(fuzzyCheckBox->isChecked ()==TRUE){
		agrep="NULL";
	}
	if(caseSensitiveCheckBox->isChecked ()==TRUE){
		ignoreCase="FALSE";
	}
	
	
	QString s = ".rk.get.search.results(\"";
	s.append(field->currentText());
	s.append("\",agrep=");
	s.append(agrep);
	s.append(", ignore.case=");
	s.append(ignoreCase);
	s.append(")");

	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, HELP_SEARCH, chain);
}

void KHelpDlg::slotResultsListDblClicked( QListViewItem * item, const QPoint &, int )
{
	chain=0;
	QString s="help(\"";
	s.append(item->text(0));
	s.append("\", htmlhelp=TRUE, package= \"");
	s.append(item->text(2));
	s.append("\")");
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, "", this, GET_HELP_URL, chain);
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
	} else {
		RK_ASSERT (false);
	}
}



#include "khelpdlg.moc"

