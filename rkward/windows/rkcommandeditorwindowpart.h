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

#ifndef RKCOMMANDEDITORWINDOWPART_H
#define RKCOMMANDEDITORWINDOWPART_H

#include <kparts/part.h>

class RKCommandEditorWindow;
class KAction;

/** This class provides a KPart interface to RKCommandEditorWindow. Basically, it is responsible for creating the menu-entries the RKCommandEditorWindow provides, and keeps the corresponding Actions. The reason to use this, is so the required menus/menu-items can be merged in on the fly.

@author Thomas Friedrichsmeier
*/
class RKCommandEditorWindowPart : public KParts::Part {
	Q_OBJECT
public:
	RKCommandEditorWindowPart (QWidget *parent, RKCommandEditorWindow *editor_widget);

	~RKCommandEditorWindowPart ();
private:
	RKCommandEditorWindow *command_editor;

	void initializeActions ();
// TODO: move these to RKCommandEditorWindow as well, disable runSelection, when there is no selection
	KAction* runAll;
	KAction* runSelection;
	KAction* runLine;

	KAction* helpFunction;
};

#endif
