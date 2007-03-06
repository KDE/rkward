/***************************************************************************
			rkward.h  -  description
			-------------------
begin                : Tue Oct 29 20:06:08 CET 2002 
copyright            : (C) 2002, 2005, 2006, 2007 by Thomas Friedrichsmeier 
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

#ifndef RKWARD_H
#define RKWARD_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dcopobject.h>

/** This base provides the DCOP-Interface for RKWardMainWindow */
class RKWardDCOPInterface : virtual public DCOPObject {
	K_DCOP
	k_dcop:

	virtual void openHTMLHelp (const QString &url) = 0;
};


// include files for Qt

// include files for KDE 
#include <kapp.h>
#include <kmdimainfrm.h>
#include <kaccel.h>
#include <kaction.h>
#include <kurl.h>
#include <kparts/partmanager.h>

class QLabel;
// forward declaration of the RKward classes
class RSettings;
class RKMenu;
class RObjectBrowser;
class RKSettingsModule;
class RKSettings;
class RInterface;
class RKEditorManager;
class RKMenuList;
class RKCommandEditorWindow;
class KMdiToolViewAccessor;
class RKMDIWindow;

/**
The main class of rkward. This is where all strings are tied togther, controlls the initialization, and there are some of the most important slots for user actions. All real work is done elsewhere.
*/


class RKWardMainWindow : public KMdiMainFrm, virtual public KParts::PartBase, virtual public RKWardDCOPInterface {
	Q_OBJECT
public:
/** construtor
@param load_url The workspace file to load on startup. If 0, show a dialog asking what to do. */
	RKWardMainWindow (KURL *load_url=0);
/** destructor */
	~RKWardMainWindow ();

/** initialize the backend */
	void startR ();

/** open a workspace. Do not ask whether to save the old one. The old workspace is deleted! */
	void fileOpenNoSave (const KURL &url);
/** open a workspace. If the current workspace is not empty, ask wether to save first. */
	void fileOpenAskSave (const KURL &url);
/** opens the given url, assuming it is an HTML-help page. Like openHTML (), but with a QString parameter instead for DCOP. Generally you should use openHTML () instead. */
	void openHTMLHelp (const QString &url);
/** opens the given url, assuming it is an HTML-help page. */
	void openHTML (const KURL &url);

	KParts::PartManager *partManager () { return part_manager; };

	static RKWardMainWindow *getMain () { return rkward_mainwin; };

	void makeRKWardHelpMenu (QWidget *for_window, KActionCollection *ac);

/** (try to) close all windows, and ask whether it is ok to quit */
	bool doQueryQuit ();
protected:
	void openWorkspace (const KURL &url);
	/** save Options/Settings. Includes general Options like all bar positions and status as well as the geometry and the recent file list */
	void saveOptions();
/** read general Options again and initialize all variables like the recent file list */
	void readOptions();
	/** initializes the KActions of the application */
	void initActions();
	/** sets up the statusbar for the main window by initialzing a statuslabel.
	*/
	void initStatusBar();
	/** reimplemented from KMainWindow to call our doQueryClose (), and then (if quitting was not cancelled), invoke an RKQuitAgent to wait for the R-backend to finish up before actually quitting. */
	virtual void closeEvent (QCloseEvent *e);
	/** saves the window properties for each open window during session end to the session config file, including saving the currently
	* opened file by a temporary filename provided by KApplication.
	* @see KTMainWindow#saveProperties

	// TODO: defunct!! Find out what this is really for.
	*/
	virtual void saveProperties(KConfig *_cfg);
	/** reads the session config file and restores the application's state including the last opened files and documents by reading the
	* temporary files saved by saveProperties()
	* @see KTMainWindow#readProperties

	// TODO: defunct!! Find out what this is really for.
	*/
	virtual void readProperties(KConfig *_cfg);
signals:
/** no idea, why we have to declare this explicitly, but somehow we do. */
	void childWindowCloseRequest (KMdiChildView *);
	void aboutToQuitRKWard ();
public slots:
	/** Raise the help search window */
	void showHelpSearch ();
	/** Toggle the help search window */
	void toggleHelpSearch ();
	/** Toggle the console window */
	void toggleConsole ();
	/** Toggle the command log window */
	void toggleCommandLog ();
	/** Toggle the pending jobs window */
	void togglePendingJobs ();
	/** Toggle the workspace browser window */
	void toggleWorkspace ();
	/** Activate the current (non tools) window in the workspace */
	void activateDocumentView ();

	/** Show the starting page of RKWard help */
	void showRKWardHelp ();
	/** Invokes R help (help.start ()) */
	void invokeRHelp ();
	/** show instructions on reporting bugs in rkward */
	void reportRKWardBug ();
	/** not quite sure, why I have to reimplement this from KMainWindow */
	void showAboutApplication ();
	/** creates a new (empty) data.frame */
	void slotNewDataFrame ();
	/** open a file and load it into the document*/
	void slotFileOpenWorkspace();
	/** opens a file from the recent files menu */
	void slotFileOpenRecentWorkspace(const KURL& url);
	/** save a document */
	void slotFileSaveWorkspace();
	/** save a document by a new filename*/
	void slotFileSaveWorkspaceAs();
	/** shows the dialog to install/load/unload packages */
	void slotFileLoadLibs ();
	/** shows the dialog to import data */
	void importData ();
	/** close all editor windows */
	void slotCloseAllEditors ();
	/** Reimplemented from KParts::MainWindow to be more pretty
	* @param text the text that is displayed in the statusbar
	*/
	void slotSetStatusBarText (const QString &text);
/** Basically a shortcut to slotSetStatusBarText (QString::null). Needed as a slot without parameters. */
	void slotSetStatusReady () { slotSetStatusBarText (QString::null); };

/** configures RKward-settings */
	void slotConfigure ();

	/** Init-procedures to be done after the exec-loop was started */
	void doPostInit ();

/** open a new command editor (blank file) */
	void slotNewCommandEditor();
/** open a new command editor (ask for file to open) */
	void slotOpenCommandEditor ();
/** open a new command editor (load given url) */
	void slotOpenCommandEditor (const KURL &url);

/** a child window has received a close event. Check, whether data needs to be saved. Ask if necessary. Close only if safe. */
	void slotChildWindowCloseRequest (KMdiChildView * window);

/** close current window (Windows->Close). Note: the only reason we need to implement this, is so we can set a default shortcut (Ctrl+W). Usually, KMdiMainFrm would provide an action like this by itselt */
	void slotCloseWindow ();
/** close all windows (Windows->Close All) */
	void slotCloseAllWindows ();
/** detach current window (Windows->Detach) */
	void slotDetachWindow ();

/** ensure output window is shown. */
	void slotOutputShow ();
/** reimplemented from KMainWindow, to additionally include the workspace url. Actually, we also ignore the caption-parameter, as it sometimes is not the one we want. Rather we create one according to the active view */
	void setCaption (const QString &caption);
/** a view has been activated or deactivated. We should make sure to update the main caption to fix strange quirks */
	void viewChanged (KMdiChildView *) { setCaption (QString::null); };

/** connected to m_manager->partAdded (). Connects statusbar notifications */
	void partAdded (KParts::Part *part);
/** connected to m_manager->partAdded (). Disconnects statusbar notifications */
	void partRemoved (KParts::Part *part);
private:
	void toggleToolView (RKMDIWindow *tool_window);

	QLabel* r_status_label;
	KParts::PartManager *part_manager;

	// KAction pointers to enable/disable actions
	KAction* fileOpen;
	KRecentFilesAction* fileOpenRecent;
	
	KAction* fileOpenWorkspace;
	KRecentFilesAction* fileOpenRecentWorkspace;
	KAction* fileSaveWorkspace;
	KAction* fileSaveWorkspaceAs;
	KAction* fileQuit;
	KAction* file_load_libs;
	KAction* close_all_editors;
	KAction* new_data_frame;
	KAction* new_command_editor;
	
	KAction* editUndo;
	KAction* editRedo;

	KAction* outputShow;

	KAction* window_close;
	KAction* window_close_all;
	KAction* window_detach;
	
	KAction* configure;
	
	friend class RKSettingsModule;
	friend class RKSettingsModulePlugins;
	friend class RKSettings;

	/** Finds plugins and inserts them into the menu-structure */
	void initPlugins ();

	KURL *initial_url;

	static RKWardMainWindow *rkward_mainwin;

	friend class RInterface;
/** set the R status message ("R engine idel/busy") to idle or busy */
	void setRStatus (bool busy);
};

#endif // RKWARD_H
