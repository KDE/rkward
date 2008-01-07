/***************************************************************************
                          rkmdiwindow  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006, 2007, 2008 by Thomas Friedrichsmeier
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

#include <qwidget.h>
#include <QFrame>

#include <kparts/part.h>

class QEvent;
class QPaintEvent;
class RKWorkplace;
class RKToolWindowBar;

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
public:
/** @returns true, if the window's document was modified (and would need to be saved) */
	virtual bool isModified () { return false; };
/** @returns A long / complete caption. Default implementation simply calls shortCaption () */
	virtual QString fullCaption ();
/** @returns A short caption (e.g. only the filename without the path). Default implementation simply calls QWidget::caption () */
	virtual QString shortCaption ();
/** @returns The corresponding KPart for this window */
	KParts::Part *getPart () { return part; };
/** This is used in RKWorkplace::saveWorkplace () to save the info about the workplace. Make sure to add corresponding code to RKWorkplace::restoreWorkplace (), so your window(s) get restored when loading a Workspace
@returns An internal descriptive string. */
	virtual QString getDescription () { return QString (); };
/** Reimplemented from QWidget::setCaption () to emit the signal captionChanged () when the caption is changed. */
	void setCaption (const QString &caption);
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
signals:
/** This signal is emitted, whenever the window caption was changed.
@param RKMDIWindow* a pointer to this window */
	void captionChanged (RKMDIWindow *);
/** This signal is emitted, when the window was activated *with* focus */
	void windowActivated (RKMDIWindow *);
protected:
	void setPart (KParts::Part *p) { part = p; };
	void initializeActivationSignals ();
	void paintEvent (QPaintEvent *e);
	void windowActivationChange (bool);

/** reimplemented from QWidget to emulate focus-follows-mouse behavior */
	void enterEvent (QEvent *event);
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
};

#endif
