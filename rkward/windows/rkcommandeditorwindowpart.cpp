/***************************************************************************
                          rkeditordataframepart  -  description
                             -------------------
    begin                : Wed Sep 14 2005
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

#include "rkcommandeditorwindowpart.h"

#include <kinstance.h>
#include <klocale.h>
#include <kaction.h>

#include "rkcommandeditorwindow.h"
#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../debug.h"

RKCommandEditorWindowPart::RKCommandEditorWindowPart (QWidget *parent, RKCommandEditorWindow *editor_widget) : KParts::Part (parent) {
	RK_TRACE (COMMANDEDITOR);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);
 
	command_editor = editor_widget;

	setXMLFile ("rkcommandeditorwindowpart.rc");

	initializeActions ();
}

RKCommandEditorWindowPart::~RKCommandEditorWindowPart () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCommandEditorWindowPart::initializeActions () {
	RK_TRACE (COMMANDEDITOR);

	runAll = new KAction (i18n ("Run all"), KShortcut ("F9"), this, SLOT (slotRunAll ()), actionCollection (), "run_all");
	runAll->setIcon("player_fwd");
	runSelection = new KAction (i18n ("Run selection"), KShortcut ("F8"), this, SLOT (slotRunSelection ()), actionCollection (), "run_selection");
	runSelection->setIcon("player_play");
	runLine = new KAction (i18n ("Run current line"), KShortcut ("Ctrl+L"), this, SLOT (slotRunLine ()), actionCollection (), "run_line");
	runLine->setIcon("player_play");

	helpFunction = new KAction (i18n ("&Function reference"), KShortcut ("F2"), this, SLOT (slotFunctionReference ()), actionCollection (), "function_reference");
}

void RKCommandEditorWindowPart::slotRunSelection() {
	RK_TRACE (COMMANDEDITOR);

	if (command_editor->getSelection ().isEmpty () || command_editor->getSelection ().isNull ()) return;

	RKGlobals::rInterface ()->issueCommand (new RCommand (command_editor->getSelection (), RCommand::User, QString::null));
}

void RKCommandEditorWindowPart::slotRunLine() {
	RK_TRACE (COMMANDEDITOR);

	if (command_editor->getLine ().isEmpty () || command_editor->getLine().isNull ()) return;

	RKGlobals::rInterface ()->issueCommand (new RCommand (command_editor->getLine (), RCommand::User, QString::null));
}


void RKCommandEditorWindowPart::slotRunAll() {
	RK_TRACE (COMMANDEDITOR);

	if (command_editor->getText ().isEmpty () || command_editor->getText ().isNull ()) return;
		
	RKGlobals::rInterface ()->issueCommand (new RCommand (command_editor->getText (), RCommand::User, QString::null));
}

void RKCommandEditorWindowPart::slotFunctionReference () {
	RK_TRACE (COMMANDEDITOR);

	command_editor->showHelp ();
}

#include "rkcommandeditorwindowpart.moc"

