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

#include <kate/document.h>
#include <kate/view.h>

/**
Provides a wrapper around the Kate-part used for syntax-highlighting

@author Thomas Friedrichsmeier
*/
class RKCommandEditor : public QWidget {
public:
    RKCommandEditor (QWidget *parent, bool readonly=false);

    ~RKCommandEditor ();
	
	void setText (const QString &text);
	void insertText (const QString &text);
	QString text ();
	
	void configure ();
private:
	Kate::Document *doc;
	Kate::View *view;
	bool readwrite;
};

#endif
