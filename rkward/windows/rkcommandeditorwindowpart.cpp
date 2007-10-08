/***************************************************************************
                          rkeditordataframepart  -  description
                             -------------------
    begin                : Wed Sep 14 2005
    copyright            : (C) 2005, 2007 by Thomas Friedrichsmeier
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
#include <kxmlguifactory.h>

#include <q3popupmenu.h>

#include "rkcommandeditorwindow.h"
#include "../misc/rkcommonfunctions.h"
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

	runAll = new KAction (i18n ("Run all"), QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/run_all.png"), KShortcut ("F9"), command_editor, SLOT (runAll()), actionCollection (), "run_all");
	runSelection = new KAction (i18n ("Run selection"), QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/run_selection.png"), KShortcut ("F8"), command_editor, SLOT (runSelection()), actionCollection (), "run_selection");
	runLine = new KAction (i18n ("Run current line"), QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/run_line.png"), KShortcut ("Ctrl+L"), command_editor, SLOT (runLine()), actionCollection (), "run_line");

	helpFunction = new KAction (i18n ("&Function reference"), KShortcut ("F2"), command_editor, SLOT (showHelp()), actionCollection (), "function_reference");
}

#include "rkcommandeditorwindowpart.moc"
