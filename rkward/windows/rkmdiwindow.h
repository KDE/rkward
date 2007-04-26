/***************************************************************************
                          rkmdiwindow  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006, 2007 by Thomas Friedrichsmeier
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

#include <kparts/part.h>
#include <kmdimainfrm.h>

class RKWorkplace;

/** Base class for rkward document mdi windows */
class RKMDIWindow : public QFrame {
	Q_OBJECT
public:
	enum Type {
		DataEditorWindow=1,
		CommandEditorWindow=2,
		OutputWindow=4,
		HelpWindow=8,
		X11Window=16,
		ConsoleWindow=32,
		CommandLogWindow=64,
		WorkspaceBrowserWindow=128,
		SearchHelpWindow=256,
		PendingJobsWindow=512,
		FileBrowserWindow=1024,

		DocumentWindow=2048,
		ToolWindow=4096,
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
	RKMDIWindow (QWidget *parent, int type, bool tool_window=false, char *name=0);
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
	bool isAttached () { return (state == Attached); };
/** Is this a tool window? */
	bool isToolWindow () { return (type & ToolWindow); };
/** Activate (raise) this window, regardless of whether it is attached or detached
@param with_focus Should the window also get keyboard focus? */
	virtual void activate (bool with_focus=true);
/** If your mdi window should perform any adjustments before being attached, reimplement this function. Default implementation does nothing, but raises an assert, if this is a tool window */
	virtual void prepareToBeAttached ();
/** If your mdi window should perform any adjustments before being detached, reimplement this function. Default implementation does nothing, but raises an assert, if this is a tool window */
	virtual void prepareToBeDetached ();
/** Tool windows will only hide themselves, and ignore the also_delete flag */
	virtual bool close (bool also_delete);
/** For tool windows only (and perhaps for KDE3 only): set the wrapper widget that should be shown/raised on activation */
	void setToolWrapper (KMdiToolViewAccessor *wrapper_widget);
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
friend class RKWorkplace;
/** type of this window */
	int type;
private:
/** state of this window (attached / detached). This is usually set from the RKWorkplace */
	KParts::Part *part;
	State state;
	KMdiToolViewAccessor *wrapper;
	bool active;
};

#endif
