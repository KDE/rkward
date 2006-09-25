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
#include <kparts/part.h>

#include "../rbackend/rcommandreceiver.h"

class RObject;
class RCommandChain;
class RKWorkplaceView;
class KParts::PartManager;
class RKEditor;

/** This class (only one instance will probably be around) keeps track of which windows are opened in the
workplace, which are detached, etc. Will replace RKEditorManager.
It also provides a QWidget (RKWorkplace::view ()), which actually manages the document windows (only those, so far. I.e. this is a half-replacement for KMdi, which will be gone in KDE 4). Currently layout of the document windows is always tabbed.
//TODO: move to windows */
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
		KParts::Part *part;
		RKWorkplaceObjectState state;
	};

	typedef QMap<QWidget *, RKWorkplaceObjectInfo *> RKWorkplaceObjectMap;

	RKWorkplaceView *view () { return wview; };

	RKWorkplaceObjectMap getObjectList () { return windows; };

/** Attach an already created window. */
	void attachWindow (QWidget *window);
	void detachWindow (QWidget *window);
	QWidget *activeAttachedWindow ();
	void activateWindow (QWidget *window);

	bool openScriptEditor (const KURL &url=KURL (), bool use_r_highlighting=true, bool read_only=false, const QString &force_caption = QString::null);
	void openHelpWindow (const KURL &url=KURL ());
	void openOutputWindow (const KURL &url=KURL ());

	bool canEditObject (RObject *object);
	RKEditor *editObject (RObject *object, bool initialize_to_empty=false);

/** tell all DataEditorWindow s to syncronize changes to the R backend
// TODO: add RCommandChain parameter */
	void flushAllData ();
	void closeActiveWindow ();
	void closeWindow (QWidget *window);
/** Closes all windows of the given type(s). Default call (no arguments) closes all windows
@param type: A bitwise OR of RKWorkplaceObjectType
@param state: A bitwise OR of RKWorkplaceObjectState */
	void closeAll (int type=AnyType, int state=AnyState);

	void saveWorkplace (RCommandChain *chain=0);
	void restoreWorkplace (RCommandChain *chain=0);

	static RKWorkplace *mainWorkplace () { return main_workplace; };
signals:
	void lastWindowClosed ();
	void changeGUI (KParts::Part *active_part);
	void changeCaption ();
public slots:
	void windowDestroyed (QObject *window);
	void updateWindowCaption (QWidget *window);
	void activeAttachedChanged (QWidget *window);

	void registerPart (QWidget *window, KParts::Part *part);
protected:
	void rCommandDone (RCommand *command);
private:
	RKWorkplaceObjectMap windows;
	RKWorkplaceView *wview;
	void addWindow (QWidget *window, RKWorkplaceObjectType type);
	static RKWorkplace *main_workplace;
friend class RKwardApp;
	KParts::PartManager *part_manager;
};

/** This is a separate class mostly for future extension. right now, it's just a QTabWidget */
class RKWorkplaceView : public QTabWidget {
public:
	RKWorkplaceView (QWidget *parent) : QTabWidget (parent) {};
	~RKWorkplaceView () {};
};

#endif
