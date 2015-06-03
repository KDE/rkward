/***************************************************************************
                          rkward.h  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002-2014 by Thomas Friedrichsmeier 
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
#include <QAction>
#include <kurl.h>
#include <kparts/mainwindow.h>

class QLabel;
class QCloseEvent;
class KActionMenu;
class RKTopLevelWindowGUI;
class KRecentFilesAction;
class KSqueezedTextLabel;
class QAction;

/**
The main class of rkward. This is where all strings are tied together, controls the initialization, and there are some of the most important slots for user actions. All real work is done elsewhere.
*/
class RKWardMainWindow : public KParts::MainWindow {
	Q_OBJECT
public:
/** construtor */
	RKWardMainWindow ();
/** destructor */
	~RKWardMainWindow ();

/** initialize the backend */
	void startR ();

	KParts::PartManager *partManager () { return part_manager; };

	static RKWardMainWindow *getMain () { return rkward_mainwin; };

/** (try to) close all windows, and ask whether it is ok to quit */
	bool doQueryQuit ();
	void lockGUIRebuild (bool lock);
/** Set whether not to ask for saving, although the workspace @em might be modified */
	void setNoAskSave (bool no_ask) { no_ask_save = no_ask; };
/** Set whether workspace is known to be unmodified, or could be modified.
    TODO: Some less guessing would be nice... */
	void setWorkspaceMightBeModified (bool modified) { workspace_modified = modified; };
/** Merge files to be loaded, instead of closing windows / clearing workspace */
	void setMergeLoads (bool merge) { merge_loads = merge; };
protected:
	/** save Options/Settings. Includes general Options like all bar positions and status as well as the geometry and the recent file list */
	void saveOptions();
/** read general Options again and initialize all variables like the recent file list */
	void readOptions();
	/** initializes the KActions of the application */
	void initActions();
	/** sets up the statusbar for the main window by initialzing a statuslabel.
	*/
	void initStatusBar();
	/** sets up the various tool windows, and starts the R engine */
	void initToolViewsAndR ();
	/** reimplemented from KMainWindow to call our doQueryClose (), and then (if quitting was not cancelled), invoke an RKQuitAgent to wait for the R-backend to finish up before actually quitting. */
	virtual void closeEvent (QCloseEvent *e);
signals:
	void aboutToQuitRKWard ();
public slots:
	void setWorkspaceUnmodified () { setWorkspaceMightBeModified (false); };
	/** open a workspace. If the current workspace is not empty, ask whether to save first.
    @see setNoAskSave ()
    @see setWorkspaceMightBeModified () */
	void askOpenWorkspace (const KUrl &url);
	/** creates a new (empty) data.frame */
	void slotNewDataFrame ();
	/** open a file and load it into the document*/
	void slotFileOpenWorkspace();
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
	void slotCancelAllCommands ();
	void configureCarbonCopy ();
	void slotSetStatusBarText (const QString &text);
/** Basically a shortcut to slotSetStatusBarText (QString ()). Needed as a slot without parameters. */
	void slotSetStatusReady () { slotSetStatusBarText (QString ()); };

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
/** Opens a new workspace, without asking or closing anything. */
	void openWorkspace (const KUrl &url);

	QLabel* statusbar_r_status;
	KSqueezedTextLabel* statusbar_cwd;
	QLabel* statusbar_ready;
	KParts::PartManager *part_manager;

	// QAction pointers to enable/disable actions
	QAction* fileOpen;
	KRecentFilesAction* fileOpenRecent;
	
	QAction* fileOpenWorkspace;
	KRecentFilesAction* fileOpenRecentWorkspace;
	QAction* fileSaveWorkspace;
	QAction* fileSaveWorkspaceAs;
	QAction* fileQuit;
	QAction* interrupt_all_commands;
	QAction* close_all_editors;
	QAction* new_data_frame;
	QAction* new_command_editor;

	QAction* window_close_all;
	QAction* window_detach;
	
	QAction* configure;

	/** used so that if the menu is empty, there is a note in it, explaining that fact */
	QAction* edit_menu_dummy;
	/** used so that if the menu is empty, there is a note in it, explaining that fact */
	QAction* view_menu_dummy;

	QAction* proxy_export, *proxy_import;
	KActionMenu* save_any_action;
	QAction* save_actions_plug_point;
	QList<QPointer <QAction> > plugged_save_actions;

	friend class RKSettingsModule;
	friend class RKSettingsModulePlugins;
	friend class RKSettings;
	friend class RKComponentMap;

	/** Finds plugins and inserts them into the menu-structure */
	void initPlugins (const QStringList &automatically_added = QStringList ());

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
	bool gui_rebuild_locked;
	bool no_ask_save;
	bool workspace_modified;
	bool merge_loads;
};

#endif // RKWARD_H
