/*
rkmdiwindow - This file is part of the RKWard project. Created: Tue Sep 26 2006
SPDX-FileCopyrightText: 2006-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKMDIWINDOW_H
#define RKMDIWINDOW_H

#include <QFrame>
#include <QMap>
#include <QUrl>
#include <QTimer>

#include <KParts/Part>

#include "../settings/rksettings.h"

class QEvent;
class QPaintEvent;
class RKWorkplace;
class RKToolWindowBar;
class KMessageWidget;
class RCommand;

class RKMDIStandardActionClient : public KXMLGUIClient {
public:
	RKMDIStandardActionClient ();
	~RKMDIStandardActionClient ();
};

/** Base class for rkward document mdi windows */
class RKMDIWindow : public QFrame {
	Q_OBJECT
public:
	enum Type {
		DataEditorWindow=1 << 0,
		CommandEditorWindow=1 << 1,
		OutputWindow=1 << 2,
		HelpWindow=1 << 3,
		X11Window=1 << 4,
		ObjectWindow=1 << 5,
		PDFWindow=1 << 6,
		ConsoleWindow=1 << 10,
		CommandLogWindow=1 << 11,
		WorkspaceBrowserWindow=1 << 12,
		SearchHelpWindow=1 << 13,
		PendingJobsWindow=1 << 14,
		FileBrowserWindow=1 << 15,
		DebugConsoleWindow=1 << 16,
		CallstackViewerWindow=1 << 17,
		DebugMessageWindow=1 << 18,
		KatePluginWindow=1 << 19,

		DocumentWindow=1 << 29,
		ToolWindow=1 << 30,
		AnyType=DocumentWindow | ToolWindow
	};

	enum State {
		Attached=1,
		Detached=2,
		AnyWindowState=Attached | Detached
	};
protected:
/** constructor
@param parent parent widget
@param type Type of window (see RKMDIWindow::Type).*/
	RKMDIWindow (QWidget *parent, int type, bool tool_window=false, const char *name = nullptr);
	virtual ~RKMDIWindow ();
public Q_SLOTS:
/** Reimplemented from QWidget::setCaption () to Q_EMIT the signal captionChanged () when the caption is changed. */
	void setCaption (const QString &caption);
public:
/** @returns true, if the window's document was modified (and would need to be saved) */
	virtual bool isModified () const { return false; };
/** Ask the window's document to save itself.
@returns true on success, _or_ if the document cannot be saved at all (in which case isModified() should return false, too. False, if saving failed / was cancelled */
	virtual bool save () { return true; };
/** @returns A long / complete caption. Default implementation simply calls shortCaption () */
	virtual QString fullCaption ();
/** @returns A short caption (e.g. only the filename without the path). Default implementation simply calls QWidget::caption () */
	virtual QString shortCaption ();
/** @returns The corresponding KPart for this window */
	KParts::Part *getPart() const { return part; };
/** Is this window attached (or detached)?
@returns true if attached, false if detached */
	bool isAttached () const { return (state == Attached); };
/** Is this a tool window? */
	bool isToolWindow () const { return (type & ToolWindow); };
/** Returns the type of this window */
	bool isType (Type t) const { return (type & t); };
/** Activate (raise) this window, regardless of whether it is attached or detached
@param with_focus Should the window also get keyboard focus? */
	virtual void activate (bool with_focus=true);
/** If your mdi window should perform any adjustments before being attached, reimplement this function. Default implementation does nothing, but raises an assert, if this is a tool window */
	virtual void prepareToBeAttached ();
/** If your mdi window should perform any adjustments before being detached, reimplement this function. Default implementation does nothing, but raises an assert, if this is a tool window */
	virtual void prepareToBeDetached ();
	enum CloseWindowMode {
		AutoAskSaveModified,
		NoAskSaveModified
	};
/** Closes the window. Most windows will be autodestruct (unless user choses to cancel closing while asked whether to save modifications), tool windows will only hide themselves (and never ask for saving).
@returns true, if the window was closed, false otherwise. */
	virtual bool close (CloseWindowMode ask_save);
/** Set a status message to be shown in a popup inside the window. The message persists until the given R command has finished, or until this function is called with an empty string.
This should be used, when the information shown is currently out-of-date (e.g. when refreshing a preview / loading a plot from history), _not_ when the window
is simply busy (e.g. when saving the current plot to history). */
	void setStatusMessage (const QString& message, RCommand* command = nullptr);
/** Set a style hint for the window. So far the only interpreted style hint is "preview", and not all windows implement it. Base implements hiding of "active" indicator border for "preview"s. */
	virtual void setWindowStyleHint (const QString& hint);

	bool eventFilter (QObject *watched, QEvent *e) override;
	bool acceptsEventsFor (QObject *object);
/** Whether the window is active. This seems to be more reliable than hasFocus () */
	bool isActive ();
/** Like isActive (), but also returns true, if this window _would_ be the active one, if the parent topLevelWindow() _was_ the active Window. */
	bool isActiveInsideToplevelWindow ();
/** Returns a pointer to an action collection suitable to place RKStandardAction in. This collection (and the corresponding KXMLGUIClient) is created on the fly. */
	KActionCollection *standardActionCollection ();
/** plugin-accessible properties of this object in the global context. Currently used only by RKEditorDataFrame to give information on the currently active data.frame. NOTE: ATM, you cannot set arbitrary properties. Only those supported in RKStandardComponent will have an effect. */
	QString globalContextProperty (const QString& property) { return global_context_properties.value (property); };
/** @returns the save action applicable for this window (if any). Will be plugged into the save dropdown */
	QAction* fileSaveAction () { return file_save_action; };
/** @returns the save as action applicable for this window (if any). Will be plugged into the save dropdown */
	QAction* fileSaveAsAction () { return file_save_as_action; };

/** Add an xml client that should be active, whenever this window is active. Noteably the KatePluginIntegrationWindow for kate related windows.
 *  For the time being, only a single buddy is allowed, and it must outlive all mdi windows. */
	void addUiBuddy(KXMLGUIClient* buddy);
	KXMLGUIClient* uiBuddy() const { return ui_buddy; };
Q_SIGNALS:
/** This signal is emitted, whenever the window caption was changed.
@param RKMDIWindow* a pointer to this window */
	void captionChanged (RKMDIWindow *);
/** This signal is emitted, when the window was activated *with* focus */
	void windowActivated (RKMDIWindow *);
protected Q_SLOTS:
	void showWindowHelp ();
	void showWindowSettings ();
	void clearStatusMessage ();
protected:
	void setPart (KParts::Part *p) { part = p; };
	void setMetaInfo (const QString& generic_window_name, const QUrl& help_url, RKSettings::SettingsPage settings_page=RKSettings::NoPage);
	void initializeActivationSignals ();
	void paintEvent (QPaintEvent *e) override;
	void changeEvent (QEvent *event) override;
	void removeUiBuddy(QObject* buddy);

/** reimplemented from QWidget to emulate focus-follows-mouse behavior */
	void enterEvent (QEnterEvent *event) override;
/** @see globalContextProperty() */
	void setGlobalContextProperty (const QString& property, const QString& value) { global_context_properties.insert (property, value); };

	KMessageWidget* status_popup;
	QWidget* status_popup_container;
	void resizeEvent (QResizeEvent *ev) override;

friend class RKWorkplace;
/** type of this window */
	int type;
private Q_SLOTS:
	void slotActivateForFocusFollowsMouse ();
protected:
	QAction* file_save_as_action;
	QAction* file_save_action;
private:
friend class RKToolWindowBar;
/** state of this window (attached / detached). This is usually set from the RKWorkplace */
	KParts::Part *part;
	State state;
	RKToolWindowBar *tool_window_bar;
	bool active;
	bool no_border_when_active;
	RKMDIStandardActionClient *standard_client;
/** @see globalContextProperty() */
	QMap<QString, QString> global_context_properties;
	QString generic_window_name;
	QUrl help_url;
	RKSettings::SettingsPage settings_page;
	KXMLGUIClient* ui_buddy;
	void showStatusMessageNow();
	QTimer status_message_timer;
	QString status_message;
};

#endif
