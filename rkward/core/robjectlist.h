/***************************************************************************
                          robjectlist  -  description
                             -------------------
    begin                : Wed Aug 18 2004
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
#ifndef ROBJECTLIST_H
#define ROBJECTLIST_H

#include <qobject.h>

class QTimer;
class RCommand;

/**
This class is responsible for keeping and updating a list of objects in the R-workspace.

@author Thomas Friedrichsmeier
*/
class RObjectList : public QObject
{
  Q_OBJECT
public:
    RObjectList ();

    ~RObjectList ();
	void updateList ();
public slots:
	void timeout ();
	void receivedROutput (RCommand *command);
signals:
/// emitted if the list of objects has changed
	void changed ();
private:
	QTimer *update_timer;
	void updateObject (char *name);

	enum { DataFrame = 1, Matrix=2, Array=4, List=8 };
	
/** this struct is used to represent an object which is currently in the process of being updated. It keeps the bits until all info has been gathered. TODO: will likely be changed in some way to interact more directly with RKVariables. */
	struct UpdatingObject {
		QString name;
		QString *classname;
		int *dimension;
		int num_classes;
		int num_dimensions;
		int type;
	};

/// Maps commands to objects
	typedef QMap<RCommand *, UpdatingObject*> UpdateMap;
	UpdateMap update_map;
};

#endif
