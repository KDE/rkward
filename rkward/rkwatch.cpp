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

RKwatch::RKwatch(RInterface *parent) : RKToggleWidget () {
	RK_TRACE (APP);
	QGridLayout *grid = new QGridLayout (this, 2, 1);
	
	QSplitter *splitter = new QSplitter (QSplitter::Vertical, this);
	grid->addWidget (splitter, 1, 0);

	QWidget *layout_widget = new QWidget (splitter);
	QHBoxLayout *bottom_hbox = new QHBoxLayout (layout_widget, 0, 6);
	
	watch = new QTextEdit (splitter);
	watch->setTextFormat (PlainText);
	watch->setReadOnly (true);
	
	commands = new RKCommandEditor (layout_widget);
	bottom_hbox->addWidget (commands);
	
	// add run & reset buttons
	QHBoxLayout *button_hbox = new QHBoxLayout (0, 0, 6);
	bottom_hbox->addLayout (button_hbox);
	
	submit = new QPushButton(i18n ("&Run"), layout_widget);
	connect (submit, SIGNAL (clicked ()), this, SLOT (submitCommand ()));
	button_hbox->addWidget (submit);
	

	
	interrupt_command = new QPushButton (i18n ("&Interrupt"), layout_widget);
	connect (interrupt_command, SIGNAL (clicked ()), this, SLOT (interruptCommand ()));
	interrupt_command->setEnabled (false);
	button_hbox->addWidget (interrupt_command);

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
	commands_menu->insertItem (i18n ("&Open new editor window"), this, SLOT (openEditor ()));
	commands_menu->insertSeparator ();
	commands_menu->insertItem (i18n ("Configure Editor"), this, SLOT (configureEditor ()));
	menu->insertItem (i18n ("Commands"), commands_menu);
	
	menu->insertSeparator ();
	
	QPopupMenu *help_menu = new QPopupMenu (this);
	help_menu->setItemEnabled (help_menu->insertItem (i18n ("Sorry, no help available so far"), 0, 0), false);
	menu->insertItem (i18n ("Help"), help_menu);
	menu->setFixedHeight (menu->height ());
	
	grid->addWidget (menu, 0, 0);

// resize, set caption
	resize (QSize (600, 520).expandedTo (minimumSizeHint ()));
	setCaption (i18n ("Console"));
	
	clearWatch ();
	
	r_inter = parent;
	user_command = 0;
}

RKwatch::~RKwatch(){
	RK_TRACE (APP);
	delete commands;
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
	if (command == user_command){
		user_command = 0;
		interrupt_command->setEnabled (false);
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
	
	watch->append ("---------------------------\n");
	watch->append (i18n ("Got reply:"));
    watch->setBold (true);

	watch->append (command->output ());
	watch->append (command->error ());
	if (command->failed () && (command->error () == "")) {
		watch->append (i18n ("An unspecified error occured while running the command.\nProbably a syntax error.\nThose - unfortunately - are not handled very well, yet.\n"));
	}

	watch->setBold (false);	
	watch->setColor (Qt::black);

	if (RKSettingsModuleWatch::shouldRaiseWindow (command)) {
		show ();
		raise ();
	}
}

void RKwatch::interruptCommand () {
	RK_TRACE (APP);
	
	RKGlobals::rInterface ()->cancelCommand (user_command);
}

void RKwatch::submitCommand () {
	RK_TRACE (APP);
	RKGlobals::editorManager ()->flushAll ();
	if (! commands->text ().isEmpty()){
		r_inter->issueCommand (user_command = new RCommand (commands->text (), RCommand::User));
		interrupt_command->setEnabled (true);
	}
	
	commands->setText ("");
	commands->setFocus ();
}



void RKwatch::configureWatch () {
	RK_TRACE (APP);
	RKSettings::configureSettings (RKSettings::Watch, this);
}

void RKwatch::configureEditor () {
	RK_TRACE (APP);
	commands->configure ();
}

void RKwatch::openEditor () {
	RK_TRACE (APP);
	new RKCommandEditorWindow (0);
}

void RKwatch::clearWatch () {
	RK_TRACE (APP);
	watch->setText ("");

	// set a fixed width font
    QFont font ("Courier");
	watch->setCurrentFont (font);
	watch->setWordWrap (QTextEdit::NoWrap);
}
