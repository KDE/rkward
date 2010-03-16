/***************************************************************************
                          rkstandardactions  -  description
                             -------------------
    begin                : Sun Nov 18 2007
    copyright            : (C) 2007, 2009, 2010 by Thomas Friedrichsmeier
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
#include "rkspecialactions.h"
#include "../windows/rkmdiwindow.h"

#include "../debug.h"

KAction* RKStandardActions::pasteSpecial (RKMDIWindow *window, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	KAction* ret = new RKPasteSpecialAction (window->standardActionCollection ());
	window->standardActionCollection ()->addAction ("paste_special", ret);
	ret->connect (ret, SIGNAL (pasteText (const QString&)), receiver, member);
	ret->setShortcut (Qt::ShiftModifier + Qt::ControlModifier + Qt::Key_V);
	return ret;
}

KAction* RKStandardActions::runLine (RKMDIWindow *window, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	KAction* ret = window->standardActionCollection ()->addAction ("run_line", receiver, member);
	ret->setText (i18n ("Run current line"));
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunLine));
	ret->setShortcut (Qt::ShiftModifier + Qt::Key_F7);
	return ret;
}

KAction* RKStandardActions::runSelection (RKMDIWindow *window, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	KAction* ret = window->standardActionCollection ()->addAction ("run_selection", receiver, member);
	ret->setText (i18n ("Run selection"));
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunSelection));
	ret->setShortcut (Qt::ShiftModifier + Qt::Key_F8);
	return ret;
}

KAction* RKStandardActions::runAll (RKMDIWindow *window, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	KAction* ret = window->standardActionCollection ()->addAction ("run_all", receiver, member);
	ret->setText (i18n ("Run all"));
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunAll));
	ret->setShortcut (Qt::ShiftModifier + Qt::Key_F9);
	return ret;
}

KAction* RKStandardActions::functionHelp (RKMDIWindow *window, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	KAction* ret = window->standardActionCollection ()->addAction ("function_reference", receiver, member);
	ret->setText (i18n ("&Function reference"));
	ret->setShortcut (Qt::Key_F2);
	return ret;
}
