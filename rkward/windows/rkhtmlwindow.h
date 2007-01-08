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

#include <qptrlist.h>

#include "../windows/rkmdiwindow.h"

class KHTMLPart;
class KActionCollection;

/**
	\brief Show html files.

This class wraps a khtml part.

It is used as a base for several purposes: Display R-help (in HTML format), display generic HTML, display RKWard output. Do not use this class directly. Use the derived classes instead.

@author Pierre Ecochard
*/
class RKHTMLWindow : public RKMDIWindow {
	Q_OBJECT
protected:
/** constructor. Protected. Use derived classes instead, or derive your own class.
@param parent parent QWidget, usually RKGlobals::rkApp () or similar */
	RKHTMLWindow (QWidget *parent = 0);
/** destructor */
	virtual ~RKHTMLWindow ();
public:
/** open given URL. Returns false, if the URL is not an existing local file. Loading a non-local URL may succeed, even if this returns false! */
	virtual bool openURL (const KURL &url);
/** Reload current page.*/
	virtual void refresh ();
/** Add common actions to the given action collection (currently only "copy")
@param action_collection A KActionCollection to insert actions in. */
	void addCommonActions (KActionCollection *action_collection);

	QString getDescription ();
	bool isModified ();
	KParts::Part *getPart ();
public slots:
/** this is used for browsing only. Use openURL instead, when calling from outside. */
	void slotOpenURLRequest (const KURL &url, const KParts::URLArgs &);
	void slotPrint ();
	void slotForward ();
	void slotBack ();
private slots:
/** This slot is called when the new page has finished loading. Sets scroll position to scroll_position */
	void loadDone ();
protected:
/** Here we store the position of the scroll bar before refresh. Used to scroll to the same position after a reload */
	int scroll_position;
/** the KHTMLPart doing all the real work */
	KHTMLPart * khtmlpart;
/** update caption according to given URL */
	virtual void updateCaption (const KURL &url);
/** called from openURL. Takes care of updating caption, and updating back/forward actions, if available */
	void changeURL (const KURL &url);
protected:
	QPtrList<KURL> url_history;
	KAction *back;
	KAction *forward;
	KAction *print;
	bool url_change_is_from_history;	// dirty!!!
};

/**
	\brief RKWard output window.

Specialized RKHTMLWindow used for RKWard output.

@author Thomas Friedrichsmeier
*/
class RKOutputWindow : public RKHTMLWindow, public KXMLGUIClient {
	Q_OBJECT
public:
/** constructor.
@param parent parent QWidget, usually RKGlobals::rkApp () or similar */
	RKOutputWindow (QWidget *parent = 0);
/** destructor */
	~RKOutputWindow ();

/** reimplemented to show "output is empty" message, if file could not be opened */
	bool openURL (const KURL &url);
/** reimplemented to scroll to the bottom of the page */
	void refresh ();
/** refresh output window.
@param show Show the window, if not currently shown (this actually means: it is created if not currently existant)
@param raise Raise the window (if currently shown, or show==true) */
	static RKOutputWindow* refreshOutput (bool show, bool raise, bool only_if_modified);

/** return a pointer to the current output. If there is no output window, one will be created (and shown) automatically */
	static RKOutputWindow* getCurrentOutput ();

	static void initialize ();

	QString getDescription ();
public slots:
/** flush current output. */
	void flushOutput ();
/** Slot wrapper around refreshOutput (bool, bool, bool). */
	void refreshOutput ();
protected:
/** reimplemented to never change the caption (it's always "Output") */
	void updateCaption (const KURL &url);
private:
/** print a message "Output is empty" to the output window. Used internally, if loading output fails*/
	void showOutputEmptyMessage ();

	KAction* outputFlush;
	KAction* outputRefresh;

	static RKOutputWindow* current_output;
/** In case the output is empty (i.e. output file does not exist), we need to store, where the output *would* be, if it existed, so we can properly refresh the output */
	KURL output_url;
	static QDateTime last_refresh_time;
};

class QDomElement;

/**
	\brief Show html help files.

This class wraps a khtml part.

Specialized HTML window for displaying R help pages.

@author Pierre Ecochard
*/
class RKHelpWindow : public RKHTMLWindow, public KXMLGUIClient {
public:
/** constructor.
@param parent parent QWidget, usually RKGlobals::rkApp () or similar */
	RKHelpWindow (QWidget *parent = 0);
/** destructor */
	~RKHelpWindow ();
/** reimplemented to handle our special protocols component://, rhelp://, and rkhelp:// in addition to the regular protocols */
	bool openURL (const KURL &url);
private:
	bool renderRKHelp (const KURL &url);
	QString renderHelpFragment (QDomElement &fragment);
	void prepareHelpLink (QDomElement *link_element);
};

#endif
