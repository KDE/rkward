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
#ifndef RKCOMMANDEDITOR_H
#define RKCOMMANDEDITOR_H

#include <qwidget.h>
#include <qstring.h>

#include <kate/document.h>
#include <kate/view.h>

#include <kurl.h>

/**
Provides a wrapper around the Kate-part used for syntax-highlighting

TODO: This class seems to be obsolete! Check whether RCommandEditorWindow provides all required functionality, and use that instead.

@author Thomas Friedrichsmeier
*/
class RKCommandEditor : public QWidget {
public:
    RKCommandEditor (QWidget *parent, bool readonly=false);

    ~RKCommandEditor ();

	void setFocus ();

	void setText (const QString &text);
	void insertText (const QString &text);
	QString text ();
	
	void configure ();
	
	bool save (const KURL &url);
	KURL getURL ();
	bool isModified ();
	bool open (const KURL &url);
	void toggleWordWrap ();
	void toggleLineNumbers ();
	QString getSelection ();
	void print ();
private:
	Kate::Document *doc;
	Kate::View *view;
	bool readwrite;
	void setRHighlighting ();
};

#endif
