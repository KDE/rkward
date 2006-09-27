/***************************************************************************
                          rkworkplace  -  description
                             -------------------
    begin                : Thu Sep 21 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#ifndef RKWORKPLACE_H
#define RKWORKPLACE_H

#include <qvaluelist.h>
#include <qstring.h>
#include <qtabwidget.h>

#include <kurl.h>

#include "rkmdiwindow.h"
#include "../rbackend/rcommandreceiver.h"

class RObject;
class RCommandChain;
class RKWorkplaceView;
class RKEditor;

/** This class (only one instance will probably be around) keeps track of which windows are opened in the workplace, which are detached, etc. Also it is responsible for creating and manipulating those windows.
It also provides a QWidget (RKWorkplace::view ()), which actually manages the document windows (only those, so far. I.e. this is a half-replacement for KMdi, which will be gone in KDE 4). Currently layout of the document windows is always tabbed. */
class RKWorkplace : public QObject, public RCommandReceiver {
	Q_OBJECT
public:
/** ctor.
@param parent: The parent widget for the workspace view (see view ()) */
	RKWorkplace (QWidget *parent);
	~RKWorkplace ();

/** @returns a pointer to the view of the workplace. Since possibly the workplace layout might change, better not rely on this pointer being valid for long */
	RKWorkplaceView *view () { return wview; };

/** convenience typedef: A list of RKMDIWindow s */
	typedef QValueList<RKMDIWindow *> RKWorkplaceObjectList;
/** Returns a list of all windows in the workplace. */
	RKWorkplaceObjectList getObjectList () { return windows; };

/** Attach an already created window. */
	void attachWindow (RKMDIWindow *window);
/** Dettach a window (it is removed from the view (), and placed in a top-level DetachedWindowContainer instead. */
	void detachWindow (RKMDIWindow *window);
/** @returns a pointer to the current window. Detached windows are not considered, only those attached to the workplace(view) */
	RKMDIWindow *activeAttachedWindow ();
/** Activate the given window. TODO: is this needed? Does it work? 
@param window the window to activate */
	void activateWindow (RKMDIWindow *window);

/** Opens a new script editor
@param url URL to load. Default option is to open an empty document
@param use_r_highlighting Set R highlighting mode (vs. no highlighting)? Default is yes
@param read_only Open the document read only? Default is false, i.e. Read-write
@param force_caption Usually the caption is determined from the url of the file. If you specify a non-empty string here, that is used instead.
@returns false if a local url could not be opened, true for all remote urls, and on success */
	bool openScriptEditor (const KURL &url=KURL (), bool use_r_highlighting=true, bool read_only=false, const QString &force_caption = QString::null);
/** Opens a new help window, starting at the given url
@param url URL to open */
	void openHelpWindow (const KURL &url=KURL ());
/** Opens a new output window. Currently only a single output window will ever be created. Subsequent calls to the function will not create additional windows right now (but will raise / refresh the output window
@param url currently ignored! */
	void openOutputWindow (const KURL &url=KURL ());

/** @returns true if there is a known editor for this type of object, false otherwise */
	bool canEditObject (RObject *object);
/** Creates a new editor of an appropriate type, and loads the given object into the editor
@param object object to edit
@param initialize_to_empty Create an empty object (no data). Use with care!
@returns a pointer to the editor */
	RKEditor *editObject (RObject *object, bool initialize_to_empty=false);

/** tell all DataEditorWindow s to syncronize changes to the R backend
// TODO: add RCommandChain parameter */
	void flushAllData ();
/** Close the active (attached) window. Safe to call even if there is no current active window (no effect in that case) */
	void closeActiveWindow ();
/** Close the given window, whether it is attached or detached.
@param window window to close */
	void closeWindow (RKMDIWindow *window);
/** Closes all windows of the given type(s). Default call (no arguments) closes all windows
@param type: A bitwise OR of RKWorkplaceObjectType
@param state: A bitwise OR of RKWorkplaceObjectState */
	void closeAll (int type=RKMDIWindow::AnyType, int state=RKMDIWindow::AnyState);

/** Write a description of all current windows to the R backend. This can later be read by restoreWorkplace ()
@param chain command chain to place the command in */
	void saveWorkplace (RCommandChain *chain=0);
/** Load a description of windows from the R backend (created by saveWorkplace ()), and (try to) restore all windows accordingly
@param chain command chain to place the command in */
	void restoreWorkplace (RCommandChain *chain=0);
/** Clear the description as set by saveWorkplace () from the R backend. Simply removes the internal object. Since the description is only needed while the workplace is being saved / restored, this should be called shortly after saveWorkplace () and restoreWorkplace ()
@param cahin command chain to place the command in */
	void clearWorkplaceDescription (RCommandChain *chain=0);

/** In the current design there is only ever one workplace. Use this static function to reference it.
@returns a pointer to the workplace */
	static RKWorkplace *mainWorkplace () { return main_workplace; };
signals:
/** TODO: For future expansion. This signal is neither emitted nor used so far. It could be used to deactivate some options in the "Window" menu. Or maybe it can be removed? */
	void lastWindowClosed ();
public slots:
/** When windows are attached to the workplace, their QObject::destroyed () signal is connected to this slot. Thereby deleted objects are removed from the workplace automatically */
	void windowDestroyed (QObject *window);
protected:
/** handles the result of the command issued in restoreWorkplace. */
	void rCommandDone (RCommand *command);
private:
/** current list of windows. @See getObjectList () */ 
	RKWorkplaceObjectList windows;
/** the view. @See view () */ 
	RKWorkplaceView *wview;
/** Internal function to add an existing window to the list of mdi windows */
	void addWindow (RKMDIWindow *window);
/** static pointer to the workplace. @See mainWorkplace () */
	static RKWorkplace *main_workplace;
};

#endif
