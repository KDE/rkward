/***************************************************************************
                          rkstandardactions  -  description
                             -------------------
    begin                : Sun Nov 18 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include "rkstandardactions.h"

#include <klocale.h>
#include <kactioncollection.h>
#include <kaction.h>

#include "rkstandardicons.h"

#include "../debug.h"

KAction* RKStandardActions::runLine (KActionCollection* action_collection, const QString &name, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	KAction* ret = action_collection->addAction (name, receiver, member);
	ret->setText (i18n ("Run current line"));
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunLine));
	ret->setShortcut (Qt::ShiftModifier + Qt::Key_F7);
	return ret;
}

KAction* RKStandardActions::runSelection (KActionCollection* action_collection, const QString &name, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	KAction* ret = action_collection->addAction (name, receiver, member);
	ret->setText (i18n ("Run selection"));
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunSelection));
	ret->setShortcut (Qt::ShiftModifier + Qt::Key_F8);
	return ret;
}

KAction* RKStandardActions::runAll (KActionCollection* action_collection, const QString &name, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	KAction* ret = action_collection->addAction (name, receiver, member);
	ret->setText (i18n ("Run all"));
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunAll));
	ret->setShortcut (Qt::ShiftModifier + Qt::Key_F9);
	return ret;
}

KAction* RKStandardActions::functionHelp (KActionCollection* action_collection, const QString &name, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	KAction* ret = action_collection->addAction (name, receiver, member);
	ret->setText (i18n ("&Function reference"));
	ret->setShortcut (Qt::Key_F2);
	return ret;
}
