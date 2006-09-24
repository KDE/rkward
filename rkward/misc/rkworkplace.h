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

#include "../rbackend/rcommandreceiver.h"

class RObject;
class RCommandChain;
class RKWorkplaceView;

/** This class (only one instance will probably be around) keeps track of which windows are opened in the
workplace, which are detached, etc. Will replace RKEditorManager.
It also provides a QWidget (RKWorkplace::view ()), which actually manages the document windows (only those, so far. I.e. this is a half-replacement for KMdi, which will be gone in KDE 4). Currently layout of the document windows is always tabbed. */
class RKWorkplace : public QObject, public RCommandReceiver {
	Q_OBJECT
public:
/** ctor.
@param parent: The parent widget for the workspace view (see view ()) */
	RKWorkplace (QWidget *parent);
	~RKWorkplace ();

	enum RKWorkplaceObjectType {
		DataEditorWindow=1,
		CommandEditorWindow=2,
		OutputWindow=4,
		HelpWindow=8,
		AnyType=DataEditorWindow | CommandEditorWindow | OutputWindow | HelpWindow
	};

	enum RKWorkplaceObjectState {
		Attached=1,
		Detached=2,
		AnyState=Attached | Detached
	};

	struct RKWorkplaceObjectInfo {
		RKWorkplaceObjectType type;
		QString location_or_name;		// do we need this?
		bool detached;
	};

	typedef QMap<QWidget *, RKWorkplaceObjectInfo *> RKWorkplaceObjectMap;

	RKWorkplaceView *view ();

	RKWorkplaceObjectMap getObjectList () { return windows; };

	void detachWindow (QWidget *window);
/** Attach an already created window. */
	void attachWindow (QWidget *window);

	void openScriptEditor (const KURL &url=KURL ());
	void openHelpWindow (const KURL &url=KURL ());
	void openOutputWindow (const KURL &url=KURL ());

	bool canEditObject (RObject *object);
	void editObject (RObject *object, bool initialize_to_empty=false);

/** tell all DataEditorWindow s to syncronize changes to the R backend
// TODO: add RCommandChain parameter */
	void flushAllData ();
/** Closes all windows of the given type(s). Default call (no arguments) closes all windows
@param type: A bitwise OR of RKWorkplaceObjectType
@param state: A bitwise OR of RKWorkplaceObjectState */
	void closeAll (int type=AnyType, int state=AnyState);

	void saveWorkplace (RCommandChain *chain=0);
	void restoreWorkplace (RCommandChain *chain=0);
signals:
	void lastWindowClosed ();
public slots:
	void windowDestroyed (QWidget *window);

	void updateWindowCaption (QWidget *window);
protected:
	void rCommandDone (RCommand *command);
private:
	RKWorkplaceObjectMap windows;
};

class RKWorkplaceView : public QTabWidget {
	RKWorkplaceView (QWidget *parent);
	~RKWorkplaceView ();
};

#endif
