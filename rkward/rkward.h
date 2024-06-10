/*
rkward.h - This file is part of the RKWard project. Created: Tue Oct 29 2002
SPDX-FileCopyrightText: 2002-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKWARD_H
#define RKWARD_H

#include <QAction>
#include <QUrl>
#include <kparts/mainwindow.h>

class QLabel;
class QCloseEvent;
class KActionMenu;
class KRecentFilesAction;
class RKTopLevelWindowGUI;
class KSqueezedTextLabel;
class QAction;
class KatePluginIntegrationApp;

/**
The main class of rkward. This is where all strings are tied together, controls the initialization, and there are some of the most important slots for user actions. All real work is done elsewhere.
*/
class RKWardMainWindow : public KParts::MainWindow {
	Q_OBJECT
public:
/** constructor */
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

	KatePluginIntegrationApp *katePluginIntegration ();
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
	virtual void closeEvent (QCloseEvent *e) override;
	bool event(QEvent *e) override;
Q_SIGNALS:
	void aboutToQuitRKWard ();
	void tabForToolViewAdded(QWidget*, QWidget*);  // Needed from katepluginintegration
	void backendCreated();
public Q_SLOTS:
	void setWorkspaceUnmodified () { setWorkspaceMightBeModified (false); };
	/** open a workspace. If the current workspace is not empty, ask whether to save first.
    @see setNoAskSave ()
    @see setWorkspaceMightBeModified () */
	void askOpenWorkspace (const QUrl &url=QUrl());
	/** creates a new (empty) data.frame */
	void slotNewDataFrame ();
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
	void slotSetStatusBarText (const QString &text) override;
/** Basically a shortcut to slotSetStatusBarText (QString ()). Needed as a slot without parameters. */
	void slotSetStatusReady () { slotSetStatusBarText (QString ()); };

/** configures RKward-settings */
	void slotConfigure ();

	/** Init-procedures to be done after the exec-loop was started */
	void doPostInit ();

/** open a new command editor (blank file) */
	void slotNewCommandEditor();
/** open a new command editor (load given url, ask for url to load, if empty) */
	void slotOpenCommandEditor (const QUrl &url = QUrl(), const QString& encoding = QString());

/** create and show a new output window */
	void slotNewOutput();
/** load an output Window (ask for file to open) */
	void slotOpenOutput(const QUrl &url=QUrl());

/** close current window (Windows->Close). */
	void slotCloseWindow ();
/** close all windows (Windows->Close All) */
	void slotCloseAllWindows ();
/** detach current window (Windows->Detach) */
	void slotDetachWindow ();

/** reimplemented from KMainWindow, to additionally include the workspace url. Parameters are ignored. Rather we create a caption according to the active view */
	void setCaption (const QString &) override;
/** HACK this is only to make the compiler happy with -Woverloaded-virtual */
	void setCaption (const QString &dummy, bool) override { setCaption (dummy); };
	void openUrlsFromCommandLineOrExternal(bool no_warn_external, QStringList urls);
/** Trigger restart of backend. Does not wait for restart to actually complete.
 *  Return true, if restart was triggered, false if restart has been cancelled */
	bool triggerBackendRestart(bool promptsave=true);
private Q_SLOTS:
	void partChanged (KParts::Part *new_part);
private:
/** Prompt for a local file to open, providing a choice of how to open the file (as R script, text, workspace, auto) */
	void openAnyFile ();

	QLabel* statusbar_r_status;
	KSqueezedTextLabel* statusbar_cwd;
	KParts::PartManager *part_manager;

	// QAction pointers to enable/disable actions
	QAction* fileOpenScript;
	QAction* fileOpenOutput;
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
	QAction* new_output;

	QAction* window_close_all;
	QAction* window_detach;
	
	QAction* configure;
	QAction* restart_r;

	/** used so that if the menu is empty, there is a note in it, explaining that fact */
	QAction* edit_menu_dummy;
	/** used so that if the menu is empty, there is a note in it, explaining that fact */
	QAction* view_menu_dummy;

	KActionMenu* open_any_action;
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

/** set the R status message ("R engine idle/busy") to idle or busy. Note: Status is actually RInterface::RStatus */
	void setRStatus (int status);
/** update the display for the current working directory */
	void updateCWD ();

	RKTopLevelWindowGUI *toplevel_actions;
	bool gui_rebuild_locked;
	bool no_ask_save;
	bool workspace_modified;
	bool merge_loads;

	KatePluginIntegrationApp *katepluginintegration;
	KXMLGUIClient *active_ui_buddy;
friend class RKWardCoreTest;
	bool testmode_suppress_dialogs;
public:
	static bool suppressModalDialogsForTesting() { return rkward_mainwin->testmode_suppress_dialogs; };
};

#endif // RKWARD_H
