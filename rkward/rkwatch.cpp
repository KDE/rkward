/***************************************************************************
                          rkwatch.cpp  -  description
                             -------------------
    begin                : Sun Nov 3 2002
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

#include "rkwatch.h"

#include "rbackend/rinterface.h"
#include "rkcommandeditor.h"
#include "rkglobals.h"
#include "settings/rksettingsmodulewatch.h"
#include "settings/rksettings.h"
#include "rkeditormanager.h"
#include "windows/rkcommandeditorwindow.h"

#include <qtextedit.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kmenubar.h>

#include "debug.h"

RKwatch::RKwatch () : QWidget () {
	RK_TRACE (APP);
	
	
	watch = new QTextEdit (this);
	watch->setTextFormat (PlainText);
	watch->setReadOnly (true);

	pLayout = new QHBoxLayout( this, 0, -1, "layout");
	pLayout->addWidget(watch);

	setCaption (i18n ("Command log"));
	
	clearWatch ();
}

RKwatch::~RKwatch(){
	RK_TRACE (APP);
}

void RKwatch::addInput (RCommand *command) {
	RK_TRACE (APP);
	if (!RKSettingsModuleWatch::shouldShowInput (command)) return;

	addInputNoCheck (command);
}

void RKwatch::addInputNoCheck (RCommand *command) {
	RK_TRACE (APP);
// TODO: make colors/styles configurable
	if (command->type () & RCommand::User) {
		watch->setColor (Qt::red);
	} else if (command->type () & RCommand::Sync) {
		watch->setColor (Qt::gray);
	} else if (command->type () & RCommand::Plugin) {
		watch->setColor (Qt::blue);
	}

	watch->append ("---------------------------\n");
	watch->append (i18n ("Issuing command:\n"));
	watch->setItalic (true);

	watch->append (command->command ());

	watch->setItalic (false);
	
	if (RKSettingsModuleWatch::shouldRaiseWindow (command)) {
		show ();
		raise ();
	}
	RK_TRACE (APP);
}

void RKwatch::addOutput (RCommand *command) {
	RK_TRACE (APP);

	if (!RKSettingsModuleWatch::shouldShowOutput (command)) {
		if (!command->failed ()) {
			return;
		} else {
		// if the command has an error and the error should be shown, but the command itself has not been shown so far, add it now.
			if (RKSettingsModuleWatch::shouldShowError (command)) {
				if (!RKSettingsModuleWatch::shouldShowInput (command)) {
					addInputNoCheck (command);
				}
			} else {
				return;
			}
		}
	}

	if (command->type () & RCommand::User) {
		watch->setColor (Qt::red);
	} else if (command->type () & RCommand::Sync) {
		watch->setColor (Qt::gray);
	} else if (command->type () & RCommand::Plugin) {
		watch->setColor (Qt::blue);
	}
	
	watch->append ("---------------------------\n");
	watch->append (i18n ("Got reply:"));
    watch->setBold (true);

	watch->append (command->output ());
	watch->append (command->error ());
	if (command->failed () && (command->error ().isEmpty ())) {
		if (command->errorIncomplete ()) {
			watch->append (i18n ("Incomplete statement.\n"));
		} else if (command->errorSyntax ()) {
			watch->append (i18n ("Syntax error.\n"));
		} else {
			watch->append (i18n ("An unspecified error occured while running the command.\n"));
		}
	}

	watch->setBold (false);	
	watch->setColor (Qt::black);
	
	
	if (RKSettingsModuleWatch::shouldRaiseWindow (command)) {
		show ();
		raise ();
	}
}




void RKwatch::configureWatch () {
	RK_TRACE (APP);
	RKSettings::configureSettings (RKSettings::Watch, this);
}


void RKwatch::clearWatch () {
	RK_TRACE (APP);
	
	
	watch->setText (QString::null);

	// set a fixed width font
	QFont font ("Courier");
	watch->setCurrentFont (font);
	watch->setWordWrap (QTextEdit::NoWrap);
}



