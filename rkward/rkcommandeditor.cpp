/***************************************************************************
                          rkcommandeditor  -  description
                             -------------------
    begin                : Mon Aug 9 2004
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
#include "rkcommandeditor.h"

#include <qlayout.h>

#include "debug.h"

RKCommandEditor::RKCommandEditor (QWidget *parent, bool readonly) : QWidget (parent) {
	RK_TRACE (COMMANDEDITOR);
	QVBoxLayout *layout = new QVBoxLayout (this);

	// create a Kate-part as command-editor
	doc = Kate::document (KTextEditor::createDocument ("libkatepart", this, "Kate::Document"));
	view = Kate::view (doc->createView (this));
	layout->addWidget (view);
	doc->setText ("");
	
	// set syntax-highlighting for R
	int modes_count = doc->hlModeCount ();
	bool found_mode = false;
	int i;
	RK_DO (qDebug ("%s", "Looking for syntax highlighting definition"), COMMANDEDITOR, DL_INFO);
	for (i = 0; i < modes_count; ++i) {
		RK_DO (qDebug ("%s", doc->hlModeName(i).lower().latin1 ()), COMMANDEDITOR, DL_DEBUG);
		if (doc->hlModeName(i).lower() == "r script") {
			found_mode = true;
			break;
		}
	}
	if (found_mode) {
		doc->setHlMode(i);
	} else {
		RK_DO (qDebug ("%s", doc->hlModeName(i).lower().latin1 ()), COMMANDEDITOR, DL_WARNING);
	}
	
	view->setDynWordWrap (false);
	doc->setReadWrite ((readwrite = (!readonly)));
}


RKCommandEditor::~RKCommandEditor () {
	RK_TRACE (COMMANDEDITOR);
	doc->writeConfig ();
	delete view;
	delete doc;
}

void RKCommandEditor::setText (const QString &text) {
	RK_TRACE (COMMANDEDITOR);
	doc->setReadWrite (true);
	doc->setText (text);
	doc->setReadWrite (readwrite);
}

void RKCommandEditor::insertText (const QString &text) {
	RK_TRACE (COMMANDEDITOR);
// TODO: make sane!
	setText (doc->text () + text);
}

QString RKCommandEditor::text () {
	RK_TRACE (COMMANDEDITOR);
	return doc->text ();
}

void RKCommandEditor::configure () {
	doc->configDialog ();
}

