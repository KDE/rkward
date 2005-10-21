/***************************************************************************
                          rkeditormanager  -  description
                             -------------------
    begin                : Fri Aug 20 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef RKEDITORMANAGER_H
#define RKEDITORMANAGER_H

#include <qwidget.h>
#include <qmap.h>
#include <qvaluelist.h>

#include "rbackend/rcommandreceiver.h"

class RKEditor;
class RObject;
class RCommandChain;
class RCommand;
class RKEditorDataFrame;

/**
This class is used to manage open editor windows. It is used to do some generic functions, like notifying the backend, when an editor window has been closed. Also (and perhaps most importantly), it contains a function editObject (), which will open any editable RObject with the editor capable to do so.

@author Thomas Friedrichsmeier
*/
class RKEditorManager : public QObject, public RCommandReceiver {
Q_OBJECT
public:
	RKEditorManager();

	~RKEditorManager();

	RKEditor *editObject (RObject *object, bool initialize_to_empty=false);
/** tries to open the editors/objects that were last opened (in the previous session) */
	void restoreEditors ();
	
	void closedEditor (RKEditor *editor);

	void flushAll ();
	void closeAll ();

/** is an editor availble for the type of object given? */
	bool canEditObject (RObject *object);

/** set the name (caption) of an editor */
	void setEditorName (RKEditor *editor, const QString &new_name);
protected:
	void rCommandDone (RCommand *command);
private:
	RCommandChain *restore_chain;
	RKEditorDataFrame *newRKEditorDataFrame ();
	QValueList<RKEditor*> editors;
};

#endif
