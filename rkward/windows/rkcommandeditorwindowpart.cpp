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
#include "../misc/rkcommonfunctions.h"
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

	runAll = actionCollection ()->addAction ("run_all", command_editor, SLOT (runAll()));
	runAll->setText (i18n ("Run all"));
	runAll->setIcon (QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/run_all.png"));
	runAll->setShortcut (Qt::Key_F9);

	runSelection = actionCollection ()->addAction ("run_selection", command_editor, SLOT (runSelection()));
	runSelection->setText (i18n ("Run selection"));
	runSelection->setIcon (QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/run_selection.png"));
	runSelection->setShortcut (Qt::Key_F9);

	runLine = actionCollection ()->addAction ("run_line", command_editor, SLOT (runLine()));
	runLine->setText (i18n ("Run current line"));
	runLine->setIcon (QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/run_line.png"));
	runLine->setShortcut (Qt::ControlModifier + Qt::Key_L);

	helpFunction = actionCollection ()->addAction ("function_reference", command_editor, SLOT (showHelp()));
	helpFunction->setText (i18n ("&Function reference"));
	helpFunction->setShortcut (Qt::Key_F2);
}

#include "rkcommandeditorwindowpart.moc"
