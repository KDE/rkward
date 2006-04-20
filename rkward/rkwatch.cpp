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

RKwatch::RKwatch () : KMdiChildView () {
	RK_TRACE (APP);
	
	
	watch = new QTextEdit (this);
	watch->setTextFormat (PlainText);
	watch->setUndoRedoEnabled (false);
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

// commands submitted via the console are often incomplete. We delay showing the input, until the command is finished.
	if (command->type () & RCommand::Console) return;

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

	watch->append ("\n");
	watch->setItalic (true);

	watch->append (command->command ());

	if (RKSettingsModuleWatch::shouldRaiseWindow (command)) {
		if (!(command->type () & RCommand::Console)) {
			emit (raiseWatch ());
		}
	}

	linesAdded ();
	watch->setItalic (false);
}

void RKwatch::addOutput (RCommand *command) {
	RK_TRACE (APP);

	if (command->type () & RCommand::Console) {
		if (command->errorIncomplete ()) return;
		if (RKSettingsModuleWatch::shouldShowInput (command)) addInputNoCheck (command);
	}

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

	if (RKSettingsModuleWatch::shouldRaiseWindow (command)) {
		if (!(command->type () & RCommand::Console)) {
			emit (raiseWatch ());
		}
	}

	linesAdded ();
	watch->setBold (false);
	watch->setColor (Qt::black);
}

void RKwatch::linesAdded () {
	RK_TRACE (APP);

// limit number of lines shown
	if (RKSettingsModuleWatch::maxLogLines ()) {
		uint c = (uint) watch->paragraphs ();
		if (c > RKSettingsModuleWatch::maxLogLines ()) {
			watch->setUpdatesEnabled (false);			// major performance boost while removing lines!
			watch->setSelection (0, 0, c - RKSettingsModuleWatch::maxLogLines (), 0, 1);
			watch->removeSelectedText (1);
			watch->setUpdatesEnabled (true);
			watch->update ();
		}
	}

// scroll to bottom
	watch->moveCursor (QTextEdit::MoveEnd, false);
	watch->scrollToBottom ();
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

#include "rkwatch.moc"
