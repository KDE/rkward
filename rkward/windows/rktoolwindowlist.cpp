/*
rktoolwindowlist - This file is part of RKWard (https://rkward.kde.org). Created: Thu Apr 07 2011
SPDX-FileCopyrightText: 2011-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rktoolwindowlist.h"
#include "rkmdiwindow.h"

#include "../debug.h"

namespace RKToolWindowListPrivate {
	QList<RKToolWindowList::ToolWindowRepresentation> registered_tool_windows;
};

QList<RKToolWindowList::ToolWindowRepresentation>& RKToolWindowList::registeredToolWindows () {
	return RKToolWindowListPrivate::registered_tool_windows;
}

void RKToolWindowList::registerToolWindow (RKMDIWindow *window, const QString &id, Placement default_placement, const QKeyCombination& default_shortcut) {
	RK_TRACE (APP);

	ToolWindowRepresentation tr;
	tr.window = window;
	tr.id = id;
	tr.default_placement = default_placement;
	tr.default_shortcut = default_shortcut;
#ifdef Q_OS_MACOS
	// HACK: Workaround for shortcut conflict issue: https://mail.kde.org/pipermail/rkward-devel/2011-December/003153.html
	if (default_shortcut.keyboardModifiers() & Qt::AltModifier) tr.default_shortcut = default_shortcut.keyboardModifiers() | Qt::ControlModifier | default_shortcut.key();
#endif

	RKToolWindowListPrivate::registered_tool_windows.append (tr);
}

RKMDIWindow* RKToolWindowList::findToolWindowById (const QString &id) {
	RK_TRACE (APP);

	for (int i = 0; i < RKToolWindowListPrivate::registered_tool_windows.size (); ++i) {
		if (RKToolWindowListPrivate::registered_tool_windows[i].id == id) return RKToolWindowListPrivate::registered_tool_windows[i].window;
	}

	return nullptr;
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
