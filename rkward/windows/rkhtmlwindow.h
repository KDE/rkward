/***************************************************************************
                          rkhtmlwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
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

#ifndef RKHTMLWINDOW_H
#define RKHTMLWINDOW_H

#include <kurl.h>
#include <kparts/browserextension.h>
#include <kmdichildview.h>
#include <kxmlguiclient.h>

class KHTMLPart;

/**
	\brief Show html files.

This class wraps a khtml part.

It is used as a base for several purposes: Display R-help (in HTML format), display generic HTML, display RKWard output. Do not use this class directly. Use the derived classes instead.

@author Pierre Ecochard
*/
class RKHTMLWindow : public KMdiChildView {
	Q_OBJECT
protected:
	RKHTMLWindow (QWidget *parent = 0);

	virtual ~RKHTMLWindow ();
public:
	virtual bool openURL (const KURL &url);
/** Reload current page.*/
	virtual void refresh ();
public slots:
/** this is used for browsing only. Use openURL instead, when calling from outside. */
	void slotOpenURLRequest (const KURL &url, const KParts::URLArgs &);
private slots:
/** This slot is called when the new page has finished loading. Sets scroll position to scroll_position */
	void loadDone ();
protected:
/** Here we store the position of the scroll bar before refresh. Used to scroll to the same position after a reload */
    int scroll_position;
	KHTMLPart * khtmlpart;
/** update caption according to given URL */
	virtual void updateCaption (const KURL &url);
};

/**
	\brief RKWard output window.

Used to display RKWard output.

@author Thomas Friedrichsmeier
*/
class RKOutputWindow : public RKHTMLWindow, public KXMLGUIClient {
	Q_OBJECT
public:
	RKOutputWindow (QWidget *parent = 0);

	~RKOutputWindow ();

/** reimplemented to show "output is empty" message, if file could not be opened */
	bool openURL (const KURL &url);
/** reimplemented to scroll to the bottom of the page */
	void refresh ();
	static void refreshOutput (bool show, bool raise);

	static RKOutputWindow* getCurrentOutput ();
public slots:
	void flushOutput ();
	void refreshOutput ();
protected:
/** reimplemented to never change the caption (it's always "Output") */
	void updateCaption (const KURL &url);
private:
	void showOutputEmptyMessage ();

	KAction* outputFlush;
	KAction* outputRefresh;

	static RKOutputWindow* current_output;
};

/**
	\brief Show html help files.

This class wraps a khtml part.

Specialized HTML window for displaying R help pages

@author Pierre Ecochard
*/
class RKHelpWindow : public RKHTMLWindow, public KXMLGUIClient {
public:
	RKHelpWindow (QWidget *parent = 0);

	~RKHelpWindow ();
};

#endif
