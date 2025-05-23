/*
rkcodenavigation - This file is part of the RKWard project. Created: Fri May 23 2025
SPDX-FileCopyrightText: 2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKCODENAVIGATION_H
#define RKCODENAVIGATION_H

#include <KTextEditor/View>

class QMenu;

namespace RKCodeNavigation {
	void doNavigation(KTextEditor::View *view, QWidget *parent);
/** creates a menu code navigation actions.
 *  The first action in the menu brings up code navigation mode (may e.g. be plugged into a toolbar). */
	QMenu *actionMenu(KTextEditor::View *view, QWidget *parent);
};

#endif
