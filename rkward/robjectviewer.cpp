/***************************************************************************
                          robjectviewer  -  description
                             -------------------
    begin                : Tue Aug 24 2004
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
#include "robjectviewer.h"

#include <qlayout.h>
#include <qtextedit.h>
#include <qfont.h>

#include "rbackend/rinterface.h"
#include "rkglobals.h"
#include "core/robject.h"
#include "core/rcontainerobject.h"

RObjectViewer::RObjectViewer (QWidget *parent, RObject *object) : QWidget (parent) {
	QGridLayout *grid = new QGridLayout (this, 1, 1);
	
	view_area = new QTextEdit (this);
	view_area->setTextFormat (PlainText);
	view_area->setReadOnly (true);
	view_area->setText ("");
	QFont font ("Courier");
	view_area->setCurrentFont (font);
	view_area->setWordWrap (QTextEdit::NoWrap);
	grid->addWidget (view_area, 0, 0);
	
	view_area->append ("Object name: " + object->getShortName ());
	view_area->append ("\nFull location: " + object->getFullName ());
	if (object->isContainer ()) {
		RContainerObject *cobj = static_cast<RContainerObject*> (object);		// for convenience only
		view_area->append ("\nClasses: " + cobj->makeClassString (", "));
		if (cobj->numDimensions ()) {
			QString dummy = "\nDimensions: " + QString ().setNum (cobj->getDimension (0));
			for (int i=1; i < cobj->numDimensions (); ++i) {
				dummy.append (", " + QString ().setNum (cobj->getDimension (i)));
			}
			view_area->append (dummy);
		}
	}
	view_area->append ("\nResult of 'print (" + object->getFullName () + ")':\n");
	
	RCommand *command = new RCommand ("print (" + object->getFullName () + ")", RCommand::App, "", this);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	waiting = true;
	destruct = false;
	
	caption = "Object-Viewer for object " + object->getShortName ();
	setCaption (caption + " - Waiting for results from R...");
	show ();
}

RObjectViewer::~RObjectViewer () {
}

void RObjectViewer::rCommandDone (RCommand *command) {
	setCaption (caption);
	
	view_area->append (command->output ());
	if (command->hasError ()) {
		view_area->append ("\nSome error(s) occured: " + command->error ());
	}
	
	if (destruct) {
		delete this;
		return;
	}
}

void RObjectViewer::closeEvent (QCloseEvent *e) {
	if (!waiting) {
		e->accept ();
		delete this;
		return;
	} else {
		destruct = true;
		e->accept ();
		hide ();
	}
}

#include "robjectviewer.moc"
