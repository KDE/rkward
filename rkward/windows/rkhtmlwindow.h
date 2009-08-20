/***************************************************************************
                          rkhtmlwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
    copyright            : (C) 2005, 2006, 2007, 2009 by Thomas Friedrichsmeier
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
#include <kxmlguiclient.h>

#include "../windows/rkmdiwindow.h"

class KHTMLPart;
class KActionCollection;
class KRecentFilesAction;
class QAction;
class QDomElement;
class RKComponentHandle;

/**
	\brief Show html files.

This class wraps a khtml part.

It is used as a base for several purposes: Display R-help (in HTML format), display generic HTML, display RKWard output. Do not use this class directly. Use the derived classes instead.

@author Pierre Ecochard
*/
class RKHTMLWindow : public RKMDIWindow, public KXMLGUIClient {
	Q_OBJECT
public:
	enum WindowMode {
		Undefined,
		HTMLHelpWindow,
		HTMLOutputWindow
	};

/** constructor. 
@param parent parent QWidget, usually RKGlobals::rkApp () or similar */
	RKHTMLWindow (QWidget *parent, WindowMode mode=HTMLHelpWindow);
/** destructor */
	~RKHTMLWindow ();
/** open given URL. Returns false, if the URL is not an existing local file. Loading a non-local URL may succeed, even if this returns false! */
	bool openURL (const KUrl &url);
/** takes care of special handling, if the url is an rkward://-url. Does nothing and returns false, otherwise. */
	bool handleRKWardURL (const KUrl &url);
/** initialize all actions */
	void initActions ();

	QString getDescription ();
	bool isModified ();
/** Return current url */
	KUrl url ();
	void doGotoAnchor (const QString &anchor_name);

/** return a pointer to the current output. If there is no output window, one will be created (and shown) automatically */
	static RKHTMLWindow* getCurrentOutput ();
/** refresh output window.
@param show Show the window, if not currently shown (this actually means: it is created if not currently existant)
@param raise Raise the window (if currently shown, or show==true) */
	static RKHTMLWindow* refreshOutput (bool show, bool raise);
public slots:
/** this is used for browsing only. Use openURL instead, when calling from outside. */
	void slotOpenUrl (const KUrl & url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &);
	void slotPrint ();
	void slotForward ();
	void slotBack ();
	void selectionChanged ();
	void runSelection ();
/** flush current output. */
	void flushOutput ();
/** Reload current page.*/
	void refresh ();
/** apply our customizations to the khtmlpart GUI */
	void fixupPartGUI ();
private slots:
/** This slot is called when the new page has finished loading. Sets scroll position to scroll_position */
	void loadDone ();
	void doGotoAnchorNow ();
protected:
/** Here we store the position of the scroll bar before refresh. Used to scroll to the same position after a reload */
	int scroll_position;
/** the KHTMLPart doing all the real work */
	KHTMLPart * khtmlpart;
/** update caption according to given URL */
	virtual void updateCaption (const KUrl &url);
/** called from openURL. Takes care of updating caption, and updating back/forward actions, if available */
	void changeURL (const KUrl &url);
private:
	QList<KUrl> url_history;
	int current_history_position;
	bool url_change_is_from_history;	// dirty!!!

	// general actions
	QAction *run_selection;
	QAction* print;
	// actions in output window mode
	QAction* outputFlush;
	QAction* outputRefresh;
	// actions in help window mode
	QAction *back;
	QAction *forward;

	QString goto_anchor_name;
	KUrl current_url;

	WindowMode window_mode;
	void useMode (WindowMode);

	void fileDoesNotExistMessage ();

	static RKHTMLWindow *current_output;

	// for dealing with rkward://[page|component]-pages
	bool renderRKHelp (const KUrl &url);
	QString renderHelpFragment (QDomElement &fragment);
	void prepareHelpLink (QDomElement *link_element);
	QString componentPathToId (QString path);
	RKComponentHandle *componentPathToHandle (QString path);
	QString startSection (const QString &name, const QString &title, const QString &shorttitle, QStringList *anchors, QStringList *anchor_names);

};

/**
	\brief Renders RKWard help pages.

@author Thomas Friedrichsmeier
*/
class RKHelpRenderer {
public:
/** ctor */
	RKHelpRenderer () {};
/** destructor */
	~RKHelpRenderer () {};
};


#endif
