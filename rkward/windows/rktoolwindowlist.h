/***************************************************************************
                          rktoolwindowlist  -  description
                             -------------------
    begin                : Thu Apr 07 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#ifndef RKTOOLWINDOWLIST_H
#define RKTOOLWINDOWLIST_H

#include <QList>
#include <QString>

class RKMDIWindow;

/** Simple helper functions to keep track of available tool windows. */
namespace RKToolWindowList {
	enum Placement {
		Left=0,
		Right=1,
		Top=2,
		Bottom=3,
		Nowhere=4
	};

	struct ToolWindowRepresentation {
		RKMDIWindow *window;
		QString id;
		Placement default_placement;
	};

	void registerToolWindow (RKMDIWindow *window, const QString &id, Placement default_placement);
	void unregisterToolWindow (RKMDIWindow *window);
	RKMDIWindow* findToolWindowById (const QString &id);
	QString idOfWindow (RKMDIWindow *window);
	QList<ToolWindowRepresentation>& registeredToolWindows ();
};

#endif
