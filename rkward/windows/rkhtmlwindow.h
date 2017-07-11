/***************************************************************************
                          rkhtmlwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
    copyright            : (C) 2005-2017 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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

#include <QUrl>
#include <kparts/part.h>
#include <kio/jobclasses.h>
#include <kwebpage.h>

#include <QDomElement>

#include "../windows/rkmdiwindow.h"

class KActionCollection;
class KRecentFilesAction;
class QAction;
class RKComponentHandle;
class XMLHelper;
class RKHTMLWindowPart;
class KWebView;
class QTemporaryFile;
class RKHTMLWindow;
class RKFindBar;
class RCommandChain;

class RKWebPage : public KWebPage {
	Q_OBJECT
public:
	explicit RKWebPage (RKHTMLWindow* window);
	void load (const QUrl& url);
signals:
	void pageInternalNavigation (const QUrl& url);
protected:
/** reimplemented to always emit linkClicked() for pages that need special handling (importantly, rkward://-urls). */
	bool acceptNavigationRequest (QWebFrame* frame, const QNetworkRequest& request, NavigationType type) override;
/** reimplemented to schedule new window creation for the next page to load */
	QWebPage* createWindow (WebWindowType type) override;
private:
	RKHTMLWindow *window;
	bool new_window;
	bool direct_load;
};

/**
	\brief Show html files.

Provide a window for viewing HTML pages.

It is used as a base for several purposes: Display R-help (in HTML format), display RKWard help pages, display generic HTML, display RKWard output.

@author Pierre Ecochard
*/
class RKHTMLWindow : public RKMDIWindow {
	Q_OBJECT
public:
	enum WindowMode {
		Undefined,
		HTMLHelpWindow,
		HTMLOutputWindow
	};

/** constructor. 
@param parent parent QWidget, usually RKGlobals::rkApp () or similar */
	explicit RKHTMLWindow (QWidget *parent, WindowMode mode=HTMLHelpWindow);
/** destructor */
	~RKHTMLWindow ();
/** open given URL. Returns false, if the URL is not an existing local file. Loading a non-local URL may succeed, even if this returns false! */
	bool openURL (const QUrl &url);
/** takes care of special handling, if the url is an rkward://-url. Does nothing and returns false, otherwise.
 *  If window is not 0, and the url is a help window, open it, there (otherwise in a new window).
 *  TODO: move to RKWorkplace? As this can really open a bunch of different things, although generally _from_ an html window.
 */
	static bool handleRKWardURL (const QUrl &url, RKHTMLWindow *window=0);
	void openRKHPage (const QUrl &url);

	bool isModified () override;
/** Return current url */
	QUrl url ();
/** Return current url in a restorable way, i.e. for help pages, abstract the session specific part of the path */
	QUrl restorableUrl ();

	WindowMode mode () { return window_mode; };
public slots:
	void slotPrint ();
	void slotSave ();
	void saveRequested (const QNetworkRequest& request);
	void slotForward ();
	void slotBack ();
	void selectionChanged ();
	void runSelection ();
/** flush current output. */
	void flushOutput ();
/** Reload current page.*/
	void refresh ();
	void zoomIn ();
	void zoomOut ();
	void setTextEncoding (QTextCodec* encoding);
private slots:
	void scrollToBottom ();
	void mimeTypeDetermined (KIO::Job*, const QString& type);
	void mimeTypeJobFail (KJob*);
	void mimeTypeJobFail2 (KJob*);
	void internalNavigation (const QUrl& new_url);
	void makeContextMenu (const QPoint& pos);
	void findRequest (const QString& text, bool backwards, const RKFindBar *findbar, bool* found);
private:
friend class RKHTMLWindowPart;
	KWebView* view;
	RKWebPage* page;
	RKFindBar* findbar;
	bool have_highlight;
/** In case the part is a khtmlpart: A ready-cast pointer to that. 0 otherwise (if a webkit part is in use) */
	RKHTMLWindowPart *part;
/** update caption according to given URL */
	virtual void updateCaption (const QUrl &url);
/** called from openURL. Takes care of updating caption, and updating back/forward actions, if available */
	void changeURL (const QUrl &url);

	struct VisitedLocation {
		QUrl url;
		QPoint scroll_position;
	};
	QList<VisitedLocation> url_history;
	void openLocationFromHistory (VisitedLocation &loc);
	int current_history_position;
	bool url_change_is_from_history;	// dirty!!!

	QUrl current_url;
	void startNewCacheFile ();
	QTemporaryFile *current_cache_file;

	WindowMode window_mode;
	void useMode (WindowMode);

	void fileDoesNotExistMessage ();

	void saveBrowserState (VisitedLocation *state);
	void restoreBrowserState (VisitedLocation *state);
};

class RKHTMLWindowPart : public KParts::Part {
	Q_OBJECT
public:
	explicit RKHTMLWindowPart (RKHTMLWindow *window);
	~RKHTMLWindowPart () {};

	void setOutputWindowSkin ();
	void setHelpWindowSkin ();
	void initActions ();
private:
friend class RKHTMLWindow;
	RKHTMLWindow *window;

	// general actions
	QAction *run_selection;
	QAction* print;
	// actions in output window mode
	QAction* outputFlush;
	QAction* outputRefresh;
	// actions in help window mode
	QAction *back;
	QAction *forward;
	QAction* save_page;
};

/**
	\brief Renders RKWard help pages.

@author Thomas Friedrichsmeier
*/
class RKHelpRenderer {
public:
/** ctor */
	explicit RKHelpRenderer (QIODevice *_device) { device = _device; help_xml = 0; component_xml = 0; };
/** destructor */
	~RKHelpRenderer () {};

	XMLHelper *help_xml;
	XMLHelper *component_xml;
	QDomElement help_doc_element;
	QDomElement component_doc_element;

	// for dealing with rkward://[page|component]-pages
	bool renderRKHelp (const QUrl &url);
	QString renderHelpFragment (QDomElement &fragment);
	QString resolveLabel (const QString &id) const;
	QString prepareHelpLink (const QString &href, const QString &text);
	QString componentPathToId (QString path);
	RKComponentHandle *componentPathToHandle (QString path);
	QString startSection (const QString &name, const QString &title, const QString &shorttitle, QStringList *anchors, QStringList *anchor_names);

	QIODevice *device;
	void writeHTML (const QString &string);
};

#include <QMultiHash>

#include <kdirwatch.h>

/** Takes care of showing / refreshing output windows as needed. */
class RKOutputWindowManager : public QObject {
Q_OBJECT
public:
	static RKOutputWindowManager *self ();

	void registerWindow (RKHTMLWindow *window);
/** R may produce output while no output window is active. This allows to set the file that should be monitored for such changes (called from within rk.set.html.output.file()). */
	void setCurrentOutputPath (const QString &path);
/** returns a list (possibly empty) of pointers to existing output windows for the given path (for the current output path, if no path given). */
	QList<RKHTMLWindow*> existingOutputWindows (const QString &path = QString ()) const;
/** Create (and show) a new output window (for the current output path, unless path is specified), and @return the pointer */
	RKHTMLWindow* newOutputWindow (const QString& path = QString ());
/** Import an existing output directory. @Returns error message, if any, and empty string in case of success */
	QString importOutputDirectory (const QString& dir, const QString& index_file=QString (), bool ask_revert = true, RCommandChain* chain = 0);
/** Save the given output directory to the locaiton it was last saved to / imported from. If the output directory has not been saved / imported, yet, prompt the user for a destination.
    @param index_path output directory to save
    @returns error message, if any, an empty string in case of success */
	QString saveOutputDirectory (const QString& dir, RCommandChain* chain = 0);
/** Save the given output directory. @see saveOutputDirectory ().
    @param index_path the output directory to save
    @param dest destination directory. May be left empty, in which case the user will be prompted for a destination.
    @returns error message, if any, an empty string in case of success */
	QString saveOutputDirectoryAs (const QString& dir, const QString& dest = QString (), bool ask_overwrite = true, RCommandChain* chain = 0);
/** Create a new empty output directory.
    @returns path of the new directory */
	QString createOutputDirectory (RCommandChain* chain = 0);
/** Drop the given output directory. If it was the active directory, activate another output file. Return error message if any, an empty string in case of success. */
	QString dropOutputDirectory (const QString& dir, bool ask=true, RCommandChain* chain = 0);

/** Return a list of all current output directories that have been modified. Used for asking for save during shutdown. */
	QStringList modifiedOutputDirectories () const;
/** Use with case! Purges all current output directories, saved or not. You should query modifiedOutputDirectories (), and make sure to prompt for saving, before calling this. For use during shutdown. */
	void purgeAllOututputDirectories ();
private:
	RKOutputWindowManager ();
	~RKOutputWindowManager ();
	static RKOutputWindowManager *_self;

	QString current_default_path;
	KDirWatch *file_watcher;
	QMultiHash<QString, RKHTMLWindow *> windows;

	struct OutputDirectory {
		QString index_file;
		QString saved_hash;
		QDateTime save_timestamp;
		QString save_dir;
	};
	/** map of outputs. Key is the working directory of the output */
	QMap<QString, OutputDirectory> outputs;
	void backendActivateOutputDirectory (const QString& dir, RCommandChain* chain);
	QString createOutputDirectoryInternal ();
	bool isRKWwardOutputDirectory (const QString &dir);
	QString dropOutputDirectoryInternal (const QString& dir);
private slots:
	void fileChanged (const QString &path);
	void windowDestroyed (QObject *window);
	void rewatchOutput ();

	void updateOutputSavedHash (RCommand *command);
};

#endif
