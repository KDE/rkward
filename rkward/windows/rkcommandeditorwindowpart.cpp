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

#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>

#include "rkcommandeditorwindow.h"
#include "../misc/rkstandardactions.h"
#include "../debug.h"

RKCommandEditorWindowPart::RKCommandEditorWindowPart (QWidget *parent, RKCommandEditorWindow *editor_widget) : KParts::Part (parent) {
	RK_TRACE (COMMANDEDITOR);

	setComponentData (KGlobal::mainComponent ());
 
	command_editor = editor_widget;
	setWidget (parent);

	setXMLFile ("rkcommandeditorwindowpart.rc");

	initializeActions ();
}

RKCommandEditorWindowPart::~RKCommandEditorWindowPart () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCommandEditorWindowPart::initializeActions () {
	RK_TRACE (COMMANDEDITOR);

	runAll = RKStandardActions::runAll (actionCollection (), "run_all", command_editor, SLOT (runAll()));
	runSelection = RKStandardActions::runSelection (actionCollection (), "run_selection", command_editor, SLOT (runSelection()));
	runLine = RKStandardActions::runLine (actionCollection (), "run_line", command_editor, SLOT (runLine()));

	helpFunction = RKStandardActions::functionHelp (actionCollection (), "function_reference", command_editor, SLOT (showHelp()));

	QAction* configure = actionCollection ()->addAction ("configure_commandeditor", command_editor, SLOT (configure()));
	configure->setText (i18n ("Configure Script Editor"));
}

#include "rkcommandeditorwindowpart.moc"
