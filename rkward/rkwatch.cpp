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
//#include "windows/rkcommandeditorwindow.h"

#include <qtextedit.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kmenubar.h>

RKwatch::RKwatch(RInterface *parent) : RKToggleWidget () {
	QGridLayout *grid = new QGridLayout (this, 2, 1);

	QSplitter *splitter = new QSplitter (QSplitter::Vertical, this);
	grid->addWidget (splitter, 1, 0);

	watch = new QTextEdit (splitter);
	watch->setTextFormat (PlainText);
	watch->setReadOnly (true);

	QWidget *layout_widget = new QWidget (splitter);
	QHBoxLayout *bottom_hbox = new QHBoxLayout (layout_widget, 0, 6);

	commands = new RKCommandEditor (layout_widget);
	bottom_hbox->addWidget (commands);
	
	// add run & reset buttons
	QVBoxLayout *button_vbox = new QVBoxLayout (0, 0, 6);
	bottom_hbox->addLayout (button_vbox);
	
	submit = new QPushButton(i18n ("&Run"), layout_widget);
	connect (submit, SIGNAL (clicked ()), this, SLOT (submitCommand ()));
	button_vbox->addWidget (submit);

	button_vbox->addStretch ();
	
	clear_commands = new QPushButton (i18n ("Reset"), layout_widget);
	connect (clear_commands, SIGNAL (clicked ()), this, SLOT (clearCommand ()));
	button_vbox->addWidget (clear_commands);

    resize (QSize (600, 511).expandedTo (minimumSizeHint ()));
	setCaption (i18n ("R-Interface Watch"));
	
	clearWatch ();
	
	r_inter = parent;

	// construct menu-bar
	KMenuBar *menu = new KMenuBar (this);
	QPopupMenu *watch_menu = new QPopupMenu (this);
	watch_menu->setItemEnabled (watch_menu->insertItem (i18n ("&Print Log"), 0, 0), false);
	watch_menu->setItemEnabled (watch_menu->insertItem (i18n ("Export Log"), 0, 0), false);
	watch_menu->insertItem (i18n ("&Clear Log"), this, SLOT (clearWatch ()));
	watch_menu->insertSeparator ();
	watch_menu->insertItem (i18n ("Configure"), this, SLOT (configureWatch ()));
	menu->insertItem (i18n ("Watch"), watch_menu);

	QPopupMenu *commands_menu = new QPopupMenu (this);
	commands_menu->setItemEnabled (commands_menu->insertItem (i18n ("&Open new editor window"), this, SLOT (openEditor ())), false);
	//commands_menu->insertItem (i18n ("&Open new editor window"), this, SLOT (openEditor ()));
	commands_menu->insertSeparator ();
	commands_menu->insertItem (i18n ("Configure Editor"), this, SLOT (configureEditor ()));
	menu->insertItem (i18n ("Commands"), commands_menu);
	
	menu->insertSeparator ();
	
	QPopupMenu *help_menu = new QPopupMenu (this);
	help_menu->setItemEnabled (help_menu->insertItem (i18n ("Sorry, no help available so far"), 0, 0), false);
	menu->insertItem (i18n ("Help"), help_menu);
	
	grid->addWidget (menu, 0, 0);
}

RKwatch::~RKwatch(){
	delete commands;
}

void RKwatch::addInput (RCommand *command) {
	if (!RKSettingsModuleWatch::shouldShowInput (command)) return;

	addInputNoCheck (command);
}

void RKwatch::addInputNoCheck (RCommand *command) {
// TODO: make colors/styles configurable
	if (command->type () & RCommand::User) {
		watch->setColor (Qt::red);
	} else if (command->type () & RCommand::Sync) {
		watch->setColor (Qt::gray);
	} else if (command->type () & RCommand::Plugin) {
		watch->setColor (Qt::blue);
	}

	watch->append ("---------------------------\n");
	watch->append ("Issuing command:\n");
	watch->setItalic (true);

	watch->append (command->command ());

	watch->setItalic (false);
}

void RKwatch::addOutput (RCommand *command) {
	if (!RKSettingsModuleWatch::shouldShowOutput (command)) {
		if (!command->hasError ()) {
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
	watch->append ("Got reply:");
    watch->setBold (true);

	watch->append (command->output ());
	watch->append (command->error ());

	watch->setBold (false);	
	watch->setColor (Qt::black);
}

void RKwatch::clearCommand () {
	commands->setText ("");
	commands->setFocus ();
}

void RKwatch::submitCommand () {
	RKGlobals::editorManager ()->syncAllToR (0);
	r_inter->issueCommand (new RCommand (commands->text (), RCommand::User));
	clearCommand ();
}

void RKwatch::configureWatch () {
	RKSettings::configureSettings (RKSettings::Watch, this);
}

void RKwatch::configureEditor () {
	commands->configure ();
}

void RKwatch::openEditor () {
	//new RKCommandEditorWindow (0);
}

void RKwatch::clearWatch () {
	watch->setText ("");

	// set a fixed width font
    QFont font ("Courier");
	watch->setCurrentFont (font);
	watch->setWordWrap (QTextEdit::NoWrap);
}
