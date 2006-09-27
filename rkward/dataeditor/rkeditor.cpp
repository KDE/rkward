/***************************************************************************
                          rkeditor  -  description
                             -------------------
    begin                : Fri Aug 20 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rkeditor.h"

RKEditor::RKEditor (QWidget *parent) : RKMDIWindow (parent, RKMDIWindow::DataEditorWindow) {
}

RKEditor::~RKEditor () {
	getObject ()->setObjectOpened (this, false);
}

#include "rkeditor.moc"
