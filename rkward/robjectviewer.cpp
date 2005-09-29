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

#include <klocale.h>

#include "rbackend/rinterface.h"
#include "rkglobals.h"
#include "core/robject.h"
#include "core/rcontainerobject.h"

RObjectViewer::RObjectViewer (QWidget *parent, RObject *object) : QWidget (parent) {
	QGridLayout *grid = new QGridLayout (this, 1, 1);
	
	view_area = new QTextEdit (this);
	view_area->setTextFormat (PlainText);
	view_area->setReadOnly (true);
	view_area->setText (QString::null);
	QFont font ("Courier");
	view_area->setCurrentFont (font);
	view_area->setWordWrap (QTextEdit::NoWrap);
	grid->addWidget (view_area, 0, 0);
	
	view_area->append (i18n("Object name: ") + object->getShortName ());
	view_area->append (i18n("\nFull location: ") + object->getFullName ());
	if (object->isContainer ()) {
		RContainerObject *cobj = static_cast<RContainerObject*> (object);		// for convenience only
		view_area->append (i18n("\nClass: ") + cobj->makeClassString (", "));
		if (cobj->numDimensions ()) {
			QString dummy = i18n("\nDimensions: ") + QString ().setNum (cobj->getDimension (0));
			for (int i=1; i < cobj->numDimensions (); ++i) {
				dummy.append (", " + QString ().setNum (cobj->getDimension (i)));
			}
			view_area->append (dummy);
		}
	}
	view_area->append (i18n("\nResult of 'print (") + object->getFullName () + i18n(")':\n"));
	
	RCommand *command = new RCommand ("print (" + object->getFullName () + ")", RCommand::App, QString::null, this);
	RKGlobals::rInterface ()->issueCommand (command, 0);
	waiting = true;
	destruct = false;
	
	caption = i18n("Object Viewer: ") + object->getShortName ();
	setCaption (caption + i18n(" - Waiting for results from R..."));
	resize (minimumSizeHint ().expandedTo (QSize (640, 480)));
	show ();
}

RObjectViewer::~RObjectViewer () {
}

void RObjectViewer::rCommandDone (RCommand *command) {
	setCaption (caption);
	
	view_area->append (command->output ());
	if (command->hasError ()) {
		view_area->append (i18n("\nSome errors occured: ") + command->error ());
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
