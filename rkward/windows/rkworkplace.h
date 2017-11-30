/***************************************************************************
                          rkworkplace  -  description
                             -------------------
    begin                : Thu Sep 21 2006
    copyright            : (C) 2006-2016 by Thomas Friedrichsmeier
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

#ifndef RKWORKPLACE_H
#define RKWORKPLACE_H

#include <qlist.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <QSplitter>
#include <QPointer>

#include <QUrl>
#include <kconfigbase.h>

#include "rkmdiwindow.h"
#include "rktoolwindowlist.h"

class RObject;
class RCommandChain;
class RKWorkplaceView;
class RKEditor;
class KActionCollection;
class QAction;
class RKToolWindowBar;
class RKMDIWindowHistoryWidget;
class RKGraphicsDevice;
class KMessageWidget;
class QWindow;

#define TOOL_WINDOW_BAR_COUNT 4

/** Simple class to store the history of recently used RKMDIWindow */
class RKMDIWindowHistory : public QObject {
	Q_OBJECT
public:
	explicit RKMDIWindowHistory (QObject *parent);
	~RKMDIWindowHistory ();

	void removeWindow (RKMDIWindow *window);
/** pops the last window from the list, if it matches the given pointer */
	void popLastWindow (RKMDIWindow *match);
	RKMDIWindow *previousDocumentWindow ();
	void next (QAction *prev_action, QAction *next_action);
	void prev (QAction *prev_action, QAction *next_action);
public slots:
	void windowActivated (RKMDIWindow *window);
private slots:
	void switcherDestroyed ();
private:
	void updateSwitcher ();
	QList<RKMDIWindow *> recent_windows;
	RKMDIWindowHistoryWidget *switcher;
	RKMDIWindowHistoryWidget *getSwitcher (QAction *prev_action, QAction *next_action);
};

/** This class (only one instance will probably be around) keeps track of which windows are opened in the workplace, which are detached, etc. Also it is responsible for creating and manipulating those windows.
It also provides a QWidget (RKWorkplace::view ()), which actually manages the document windows (only those, so far. I.e. this is a half-replacement for KMdi, which will be gone in KDE 4). Currently layout of the document windows is always tabbed. */
class RKWorkplace : public QWidget {
	Q_OBJECT
public:
/** ctor.
@param parent: The parent widget for the workspace view (see view ()) */
	explicit RKWorkplace (QWidget *parent);
	~RKWorkplace ();
	void initActions (KActionCollection *ac);

/** @returns a pointer to the view of the workplace. Since possibly the workplace layout might change, better not rely on this pointer being valid for long */
	RKWorkplaceView *view () { return wview; };

/** convenience typedef: A list of RKMDIWindow s */
	typedef QList<RKMDIWindow *> RKWorkplaceObjectList;
/** Returns a list of all windows in the workplace. */
	RKWorkplaceObjectList getObjectList () { return windows; };
/** Returns a list of all windows with a given type and state */
	RKWorkplaceObjectList getObjectList (int type, int state=RKMDIWindow::AnyWindowState);

/** Attach an already created window. */
	void attachWindow (RKMDIWindow *window);
/** Dettach a window (it is removed from the view (), and placed in a top-level DetachedWindowContainer instead. */
	void detachWindow (RKMDIWindow *window, bool was_attached=true);
/** @returns a pointer to the current window. state specifies, which windows should be considered. */
	RKMDIWindow *activeWindow (RKMDIWindow::State state);

/** Opens the given url in the appropriate way. */
	bool openAnyUrl (const QUrl &url, const QString &known_mimetype = QString (), bool force_external=false);

/** Opens a new script editor
@param url URL to load. Default option is to open an empty document
@param encoding encoding to use. If QString (), the default encoding is used.
@param use_r_highlighting Set R highlighting mode (vs. no highlighting)? Default is yes
@param read_only Open the document read only? Default is false, i.e. Read-write
@param force_caption Usually the caption is determined from the url of the file. If you specify a non-empty string here, that is used instead.
@returns false if a local url could not be opened, true for all remote urls, and on success */
	RKMDIWindow* openScriptEditor (const QUrl &url=QUrl (), const QString& encoding=QString (), bool use_r_highlighting=true, bool read_only=false, const QString &force_caption = QString (), bool delete_on_close=false);
/** Opens a new help window, starting at the given url
@param url URL to open
@param only_once if true, checks whether any help window already shows this URL. If so, raise it, but do not open a new window. Else show the new window */
	RKMDIWindow* openHelpWindow (const QUrl &url=QUrl (), bool only_once=false);
/** Opens a new output window, or raise / refresh the current output window.
@param url currently ignored! */
	RKMDIWindow* openOutputWindow (const QUrl &url=QUrl ());

	void newX11Window (QWindow* window_to_embed, int device_number);
	void newRKWardGraphisWindow (RKGraphicsDevice *dev, int device_number);
	RKMDIWindow* newObjectViewer (RObject *object);

/** @returns true if there is a known editor for this type of object, false otherwise */
	bool canEditObject (RObject *object);
/** Creates a new editor of an appropriate type, and loads the given object into the editor
@param object object to edit
@returns a pointer to the editor */
	RKEditor* editObject (RObject *object);
/** Creates a new data.frame with the given name, and loads it in an editor. @see editObject()
@param name Name of the data.frame to create
@returns a pointer to the editor */
	RKEditor* editNewDataFrame (const QString& name);

/** tell all DataEditorWindow s to synchronize changes to the R backend
// TODO: add RCommandChain parameter */
	void flushAllData ();
/** Close the active (attached) window. Safe to call even if there is no current active window (no effect in that case) */
	void closeActiveWindow ();
/** Close the given window, whether it is attached or detached.
@param window window to close */
	void closeWindow (RKMDIWindow *window, RKMDIWindow::CloseWindowMode ask_save = RKMDIWindow::AutoAskSaveModified);
/** Close the given windows, whether they are attached or detached. TODO: Be smart about asking what to save.
@param windows list windows to close
@returns false if cancelled by user (user was prompted for saving, and chose cancel) */
	bool closeWindows (QList<RKMDIWindow*> windows);
/** Closes all windows of the given type(s). Default call (no arguments) closes all windows
@param type: A bitwise OR of RKWorkplaceObjectType
@param state: A bitwise OR of RKWorkplaceObjectState
@returns false if cancelled by user (user was prompted for saving, and chose cancel) */
	bool closeAll (int type=RKMDIWindow::AnyType, int state=RKMDIWindow::AnyWindowState);

/** Write a description of all current windows to the R backend. This can later be read by restoreWorkplace (). Has no effect, if RKSettingsModuleGeneral::workplaceSaveMode () != RKSettingsModuleGeneral::SaveWorkplaceWithWorkspace
@param chain command chain to place the command in */
	void saveWorkplace (RCommandChain *chain=0);
/** Load a description of windows from the R backend (created by saveWorkplace ()), and (try to) restore all windows accordingly
Has no effect, if RKSettingsModuleGeneral::workplaceSaveMode () != RKSettingsModuleGeneral::SaveWorkplaceWithWorkspace
@param chain command chain to place the command in */
	void restoreWorkplace (RCommandChain *chain=0, bool merge=false);
/** Like the other restoreWorkplace (), but takes the description as a parameter rather than reading from the R workspace. To be used, when RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSeesion
@param description workplace description */
	void restoreWorkplace (const QStringList &description);

	QStringList makeWorkplaceDescription ();
	QString makeItemDescription (RKMDIWindow *) const;
/** Restore a document window given its description. Returns true, if a window was restored, false otherwise (e.g. invalid/unsupported description). */
	bool restoreDocumentWindow (const QString &description, const QString &base=QString ());

/** In the current design there is only ever one workplace. Use this static function to reference it.
@returns a pointer to the workplace */
	static RKWorkplace *mainWorkplace () { return main_workplace; };
	static RKMDIWindowHistory *getHistory () { return main_workplace->history; };
	void placeToolWindows ();

	void setWorkspaceURL (const QUrl &url, bool keep_config=false);
	QUrl workspaceURL () const { return current_url; };
	KConfigBase *workspaceConfig ();
	QString portableUrl (const QUrl &url);
/** Register a named area where to place MDI windows. For directing preview windows to a specific location. */
	void registerNamedWindow (const QString& id, QObject *owner, QWidget* parent, RKMDIWindow *window=0);
/** Return the window in the specified named area (can be 0). */
	RKMDIWindow *getNamedWindow (const QString& id);
/** Make the next window to be created appear in a specific location (can be a named window). 
 *  @note It is the caller's responsibility to clear the override (by calling setWindowPlacementOverride ()) after the window in question has been created. */
	void setWindowPlacementOverrides (const QString& placement=QString (), const QString& name=QString (), const QString& style=QString ());

/** Inserts the given message widget above the central area. While technically, the workplace becomes the parent widget of the message widget, it is the caller's responsibility to
 *  delete the widget, when appropriate. */
	void addMessageWidget (KMessageWidget *message);

/** For window splitting: Copy the given window (or, if that is not possible, create a placeholder window), and attach it to the main view. */
	void splitAndAttachWindow (RKMDIWindow *source);
signals:
/** emitted when the workspace Url has changed */
	void workspaceUrlChanged (const QUrl &url);
public slots:
/** When windows are attached to the workplace, their QObject::destroyed () signal is connected to this slot. Thereby deleted objects are removed from the workplace automatically */
	void removeWindow (QObject *window);
	void saveSettings ();
private slots:
	void namedWindowDestroyed (QObject *);
	void namedWindowOwnerDestroyed (QObject *);
private:
	QUrl current_url;
	KConfig *_workspace_config;

/** current list of windows. @See getObjectList () */ 
	RKWorkplaceObjectList windows;
/** the view. @See view () */ 
	RKWorkplaceView *wview;
/** Internal function to add an existing window to the list of mdi windows */
	void addWindow (RKMDIWindow *window, bool attached=true);
/** static pointer to the workplace. @See mainWorkplace () */
	static RKWorkplace *main_workplace;
/** a window was removed. Try to activate some other window. */
	void windowRemoved ();

	RKMDIWindowHistory *history;

	QSplitter *horiz_splitter;
	QSplitter *vert_splitter;
	QWidget *message_area;

	RKToolWindowBar* tool_window_bars[TOOL_WINDOW_BAR_COUNT];
friend class RKToolWindowBar;
	void placeInToolWindowBar (RKMDIWindow *window, int position);

	/** Control placement of windows from R */
	struct NamedWindow {
		/** "Owner" of this named window. If non-0, and the owner gets destroyed, the named window record gets closed, too. */
		QObject *owner;
		/** Where to place window, if it does not exist, yet. Can be one of 0: detached, RKWardMainWindow::getMain(): attached, any other: docked in a special region */
		QPointer<QWidget> parent;
		/** Pointer to window, if it exists, already */
		RKMDIWindow* window;
		/** Identifier string */
		QString id;
	};
	QList<NamedWindow> named_windows;
	RKMDIWindow::State window_placement_override;
	QString window_name_override;
	QString window_style_override;
};

#endif
