/***************************************************************************
			rkward.h  -  description
			-------------------
begin                : Tue Oct 29 20:06:08 CET 2002 
copyright            : (C) 2002, 2005, 2006, 2007, 2008, 2009, 2010 by Thomas Friedrichsmeier 
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

#include <kapplication.h>
#include <kaction.h>
#include <kurl.h>
#include <kparts/mainwindow.h>

class QLabel;
class QCloseEvent;
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
class RKTopLevelWindowGUI;
class KRecentFilesAction;
class KSqueezedTextLabel;

struct RKWardStartupOptions {
	KUrl initial_url;	/**< The workspace file to load on startup. If empty, show a dialog asking what to do. **/
	QString evaluate;	/**< R code to run after startup */
};

/**
The main class of rkward. This is where all strings are tied togther, controlls the initialization, and there are some of the most important slots for user actions. All real work is done elsewhere.
*/
class RKWardMainWindow : public KParts::MainWindow {
	Q_OBJECT
public:
/** construtor
@param options Options from command line. RKWardMainWindow will take ownership of this pointer, and delete it, once not longer needed. */
	RKWardMainWindow (RKWardStartupOptions *options = 0);
/** destructor */
	~RKWardMainWindow ();

/** initialize the backend */
	void startR ();

/** open a workspace. Do not ask whether to save the old one. The old workspace is deleted! */
	void fileOpenNoSave (const KUrl &url);
/** open a workspace. If the current workspace is not empty, ask wether to save first. */
	void fileOpenAskSave (const KUrl &url);
/** opens the given url, assuming it is an HTML-help page. */
	void openHTML (const KUrl &url);

	KParts::PartManager *partManager () { return part_manager; };

	static RKWardMainWindow *getMain () { return rkward_mainwin; };

/** (try to) close all windows, and ask whether it is ok to quit */
	bool doQueryQuit ();
protected:
	void openWorkspace (const KUrl &url);
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
signals:
	void aboutToQuitRKWard ();
public slots:
	/** creates a new (empty) data.frame */
	void slotNewDataFrame ();
	/** open a file and load it into the document*/
	void slotFileOpenWorkspace();
	/** opens a file from the recent files menu */
	void slotFileOpenRecentWorkspace(const KUrl& url);
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

/** Add the given url to the list of recent scripts */
	void addScriptUrl (const KUrl& url);
/** Add the given url to the list of recent workspaces */
	void addWorkspaceUrl (const KUrl& url);

	/** Init-procedures to be done after the exec-loop was started */
	void doPostInit ();

/** open a new command editor (blank file) */
	void slotNewCommandEditor();
/** open a new command editor (ask for file to open) */
	void slotOpenCommandEditor ();
/** open a new command editor (load given url) */
	void slotOpenCommandEditor (const KUrl &url, const QString& encoding = QString ());

/** close current window (Windows->Close). */
	void slotCloseWindow ();
/** close all windows (Windows->Close All) */
	void slotCloseAllWindows ();
/** detach current window (Windows->Detach) */
	void slotDetachWindow ();

/** reimplemented from KMainWindow, to additionally include the workspace url. Parameters are ignored. Rather we create a caption according to the active view */
	void setCaption (const QString &);
/** HACK this is only to make the compiler happy with -Woverloaded-virtual */
	void setCaption (const QString &dummy, bool) { setCaption (dummy); };
private slots:
	void partChanged (KParts::Part *new_part);
private:
	QLabel* statusbar_r_status;
	KSqueezedTextLabel* statusbar_cwd;
	QLabel* statusbar_ready;
	KParts::PartManager *part_manager;

	// KAction pointers to enable/disable actions
	KAction* fileOpen;
	KRecentFilesAction* fileOpenRecent;
	
	KAction* fileOpenWorkspace;
	KRecentFilesAction* fileOpenRecentWorkspace;
	KAction* fileSaveWorkspace;
	KAction* fileSaveWorkspaceAs;
	KAction* fileQuit;
	KAction* close_all_editors;
	KAction* new_data_frame;
	KAction* new_command_editor;
	
	KAction* editUndo;
	KAction* editRedo;

	KAction* window_close_all;
	KAction* window_detach;
	
	KAction* configure;

	/** used so that if the menu is empty, there is a note in it, explaining that fact */
	KAction* edit_menu_dummy;
	/** used so that if the menu is empty, there is a note in it, explaining that fact */
	KAction* view_menu_dummy;
	/** used so that if the menu is empty, there is a note in it, explaining that fact */
	KAction* run_menu_dummy;

	friend class RKSettingsModule;
	friend class RKSettingsModulePlugins;
	friend class RKSettings;

	/** Finds plugins and inserts them into the menu-structure */
	void initPlugins ();

	RKWardStartupOptions *startup_options;

	static RKWardMainWindow *rkward_mainwin;

	friend class RInterface;
	enum RStatus {
		Busy,
		Idle,
		Starting
	};
/** set the R status message ("R engine idle/busy") to idle or busy */
	void setRStatus (RStatus status);
/** update the display for the current working directory */
	void updateCWD ();

	RKTopLevelWindowGUI *toplevel_actions;
};

#endif // RKWARD_H
