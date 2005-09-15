/***************************************************************************
                          rkhtmlwindowpart  -  description
                             -------------------
    begin                : Thu Sep 15 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

#ifndef RKHTMLWINDOWPART_H
#define RKHTMLWINDOWPART_H

#include <kparts/part.h>

class KMdiChildView;
class RKHelpWindow;
class KAction;

/**
@author Thomas Friedrichsmeier
*/
class RKHTMLWindowPart: public KParts::Part {
	Q_OBJECT
public:
	RKHTMLWindowPart (bool is_output=false);

	~RKHTMLWindowPart ();

	KMdiChildView* getWidget ();

	static void openHTML (const KURL &url, bool is_output=false);

	static void refreshOutput (bool show, bool raise);

	static RKHTMLWindowPart* getCurrentOutput ();
public slots:
	void flushOutput ();
	void refreshOutput ();
private:
	KAction* outputFlush;
	KAction* outputRefresh;

	RKHelpWindow* widget;

	static RKHTMLWindowPart* current_output;
};

#endif
