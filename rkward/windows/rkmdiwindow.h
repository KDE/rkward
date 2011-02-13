/***************************************************************************
                          rkmdiwindow  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006, 2007, 2008, 2009, 2010 by Thomas Friedrichsmeier
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

#ifndef RKMDIWINDOW_H
#define RKMDIWINDOW_H

#include <QFrame>
#include <QMap>

#include <kparts/part.h>

#include "../settings/rksettings.h"

class QEvent;
class QPaintEvent;
class RKWorkplace;
class RKToolWindowBar;

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
		ConsoleWindow=1 << 10,
		CommandLogWindow=1 << 11,
		WorkspaceBrowserWindow=1 << 12,
		SearchHelpWindow=1 << 13,
		PendingJobsWindow=1 << 14,
		FileBrowserWindow=1 << 15,

		DocumentWindow=1 << 20,
		ToolWindow=1 << 21,
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
	RKMDIWindow (QWidget *parent, int type, bool tool_window=false, const char *name=0);
	virtual ~RKMDIWindow ();
public slots:
/** Reimplemented from QWidget::setCaption () to emit the signal captionChanged () when the caption is changed. */
	void setCaption (const QString &caption);
public:
/** @returns true, if the window's document was modified (and would need to be saved) */
	virtual bool isModified () { return false; };
/** @returns A long / complete caption. Default implementation simply calls shortCaption () */
	virtual QString fullCaption ();
/** @returns A short caption (e.g. only the filename without the path). Default implementation simply calls QWidget::caption () */
	virtual QString shortCaption ();
/** @returns The corresponding KPart for this window */
	KParts::Part *getPart () { return part; };
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
/** Tool windows will only hide themselves, and ignore the also_delete flag */
	virtual bool close (bool also_delete);

	bool eventFilter (QObject *watched, QEvent *e);
	bool acceptsEventsFor (QObject *object);
/** Whether the window is active. This seems to be more reliable than hasFocus () */
	bool isActive ();
/** Returns a pointer to an action collection suitable to place RKStandardAction in. This collection (and the corresponding KXMLGUIClient) is created on the fly. */
	KActionCollection *standardActionCollection ();
/** plugin-accessible properties of this object in the global context. Currently used only by RKEditorDataFrame to give information on the currently active data.frame. NOTE: ATM, you cannot set arbitrary properties. Only those supported in RKStandardComponent will have an effect. */
	QString globalContextProperty (const QString& property) { return global_context_properties.value (property); };
signals:
/** This signal is emitted, whenever the window caption was changed.
@param RKMDIWindow* a pointer to this window */
	void captionChanged (RKMDIWindow *);
/** This signal is emitted, when the window was activated *with* focus */
	void windowActivated (RKMDIWindow *);
protected slots:
	void showWindowHelp ();
	void showWindowSettings ();
protected:
	void setPart (KParts::Part *p) { part = p; };
	void setMetaInfo (const QString& generic_window_name, const QString& help_url, RKSettings::SettingsPage settings_page=RKSettings::NoPage);
	void initializeActivationSignals ();
	void paintEvent (QPaintEvent *e);
	void windowActivationChange (bool);

/** reimplemented from QWidget to emulate focus-follows-mouse behavior */
	void enterEvent (QEvent *event);
/** @see globalContextProperty() */
	void setGlobalContextProperty (const QString& property, const QString& value) { global_context_properties.insert (property, value); };
friend class RKWorkplace;
/** type of this window */
	int type;
private:
friend class RKToolWindowBar;
/** state of this window (attached / detached). This is usually set from the RKWorkplace */
	KParts::Part *part;
	State state;
	RKToolWindowBar *tool_window_bar;
	bool active;
	RKMDIStandardActionClient *standard_client;
/** @see globalContextProperty() */
	QMap<QString, QString> global_context_properties;
	QString generic_window_name;
	QString help_url;
	RKSettings::SettingsPage settings_page;
};

#endif
