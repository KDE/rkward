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

#include <kurl.h>

#include "../rbackend/rcommandreceiver.h"

class KMdiChildView;
class RObject;
class RCommandChain;

/** This class (only one instance will probably be around) keeps track of which windows are opened in the
workplace, which are detached, etc. Will replace RKEditorManager. TODO: maybe this class can also keep track of the active view more reliably than KMDI-framework does? */
class RKWorkPlace : public QObject, public RCommandReceiver {
	Q_OBJECT
public:
	RKWorkPlace (QObject *parent);
	~RKWorkPlace ();

	enum RKWorkPlaceObjectType {
		EditorWindow=1,
		CommandEditorWindow=2,
		OutputWindow=3,
		HelpWindo=4
	};

	struct RKWorkPlaceObject {
		KMdiChildView *view;
		RKWorkPlaceObjectType type;
		QString location_or_name;
		bool detached;
	};

	typedef QValueList<RKWorkPlaceObject *> RKWorkPlaceObjectList;

	RKWorkPlaceObjectList getObjectList ();

	void detachView (KMdiChildView *view);

	void openScriptEditor (const KURL &url=KURL ());
	void openHelpWindow (const KURL &url=KURL ());
	void openOutputWindow (const KURL &url=KURL ());

	bool canEditObject (RObject *object);
	void editObject (RObject *object, bool initialize_to_empty=false);

	void flushAllData ();
	void closeAll ();
	void closeAllData ();

	void saveWorkplace (RCommandChain *chain=0);
	void restoreWorkplace (RCommandChain *chain=0);
signals:
	void lastWindowClosed ();
public slots:
	// TODO: eventually, this class should do all the work, not just receive the signals
	void viewDetached (KMdiChildView *view);
	void viewAttached (KMdiChildView *view);

	void viewDestroyed (KMdiChildView *view);

	void updateViewCaption (KMdiChildView *view);
protected:
	void rCommandDone (RCommand *command);
private:
	RKWorkPlaceObjectList attached_windows;
	RKWorkPlaceObjectList detached_windows;
};

#endif
