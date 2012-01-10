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

#include "rktoolwindowlist.h"
#include "rkmdiwindow.h"

#include "../debug.h"

namespace RKToolWindowListPrivate {
	QList<RKToolWindowList::ToolWindowRepresentation> registered_tool_windows;
};

QList<RKToolWindowList::ToolWindowRepresentation>& RKToolWindowList::registeredToolWindows () {
	return RKToolWindowListPrivate::registered_tool_windows;
}

void RKToolWindowList::registerToolWindow (RKMDIWindow *window, const QString &id, Placement default_placement, int default_shortcut) {
	RK_TRACE (APP);

	ToolWindowRepresentation tr;
	tr.window = window;
	tr.id = id;
	tr.default_placement = default_placement;
#ifdef Q_WS_MAC
	// HACK: Workaround for shortcut conflict issue: http://www.mail-archive.com/rkward-devel@lists.sourceforge.net/msg01659.html
	if (default_shortcut & Qt::AltModifier) default_shortcut |= Qt::CtrlModifier;
#endif
	tr.default_shortcut = default_shortcut;

	RKToolWindowListPrivate::registered_tool_windows.append (tr);
}

RKMDIWindow* RKToolWindowList::findToolWindowById (const QString &id) {
	RK_TRACE (APP);

	for (int i = 0; i < RKToolWindowListPrivate::registered_tool_windows.size (); ++i) {
		if (RKToolWindowListPrivate::registered_tool_windows[i].id == id) return RKToolWindowListPrivate::registered_tool_windows[i].window;
	}

	return 0;
}

void RKToolWindowList::unregisterToolWindow (RKMDIWindow *window) {
	RK_TRACE (APP);

	for (int i = 0; i < RKToolWindowListPrivate::registered_tool_windows.size (); ++i) {
		if (RKToolWindowListPrivate::registered_tool_windows[i].window == window) {
			RKToolWindowListPrivate::registered_tool_windows.removeAt (i);
			return;
		}
	}

	RK_ASSERT (false);
}

QString RKToolWindowList::idOfWindow (RKMDIWindow *window) {
	RK_TRACE (APP);

	for (int i = 0; i < RKToolWindowListPrivate::registered_tool_windows.size (); ++i) {
		if (RKToolWindowListPrivate::registered_tool_windows[i].window == window) {
			return RKToolWindowListPrivate::registered_tool_windows[i].id;
		}
	}
	RK_ASSERT (false);
	return QString ();
}
