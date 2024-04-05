/*
rktoolwindowlist - This file is part of the RKWard project. Created: Thu Apr 07 2011
SPDX-FileCopyrightText: 2011-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKTOOLWINDOWLIST_H
#define RKTOOLWINDOWLIST_H

#include <QList>
#include <QString>
#include <QKeyCombination>

class RKMDIWindow;

/** Simple helper functions to keep track of available tool windows. */
namespace RKToolWindowList {
	enum Placement {		//NOTE: enum values must match thus in KMultiTabBarPosition
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
		QKeyCombination default_shortcut;
	};

	void registerToolWindow (RKMDIWindow *window, const QString &id, Placement default_placement, const QKeyCombination& default_shortcut);
	void unregisterToolWindow (RKMDIWindow *window);
	RKMDIWindow* findToolWindowById (const QString &id);
	QString idOfWindow (RKMDIWindow *window);
	QList<ToolWindowRepresentation>& registeredToolWindows ();
};

#endif
