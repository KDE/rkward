/***************************************************************************
                          rkxmlguipreviewarea  -  description
                             -------------------
    begin                : Wed Feb 03 2016
    copyright            : (C) 2016 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rkxmlguipreviewarea.h"

#include <QMenu>
#include <QToolButton>
#include <QEvent>
#include <QMenuBar>
#include <QWidgetAction>
#include <QLabel>

#include <kxmlguifactory.h>
#include <ktoolbar.h>
#include <kmenubar.h>
#include <klocale.h>

#include "../windows/rkmdiwindow.h"
#include "rkstandardicons.h"

#include "../debug.h"

RKXMLGUIPreviewArea::RKXMLGUIPreviewArea (QWidget* parent) : KXmlGuiWindow (parent) {
	RK_TRACE (PLUGIN);

	menu_button = new QToolButton (this);
	menu_button->setPopupMode (QToolButton::InstantPopup);
	menu_button->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionShowMenu));
	menu_button->setMenu (menu = new QMenu ());
	// KF5 TODO:
	connect (menu, SIGNAL (aboutToShow()), this, SLOT (prepareMenu()));
	current = 0;
	setWindowFlags (Qt::Widget);
	setMenuBar (new QMenuBar (this));
	setHelpMenuEnabled (false);
}

RKXMLGUIPreviewArea::~RKXMLGUIPreviewArea () {
	RK_TRACE (PLUGIN);

	if (current) {
		removeChildClient (current);
		current->setFactory (0);
	}
}

QWidget* RKXMLGUIPreviewArea::menuButton() const {
	return menu_button;
}

void RKXMLGUIPreviewArea::childEvent (QChildEvent *event) {
	RK_TRACE (PLUGIN);

	if (event->type () == QEvent::ChildAdded) {
		RKMDIWindow *child = qobject_cast<RKMDIWindow*> (event->child ());
		if (child) {
			if (current) {
				removeChildClient (current);
				factory ()->removeClient (current);  // _always_ remove before adding, or the previous child will be leaked in the factory
			}
			current = child->getPart ();
			insertChildClient (current);
			setCentralWidget (child);
			createGUI ("rkdummypart.rc");
			menuBar ()->hide ();
			QList<KToolBar*> tbars = toolBars ();
			for (int i = 0; i < tbars.size (); ++i) tbars[i]->hide ();
		}
	}
	QObject::childEvent (event);
}

void RKXMLGUIPreviewArea::prepareMenu () {
	RK_TRACE (PLUGIN);

	// flatten menu, and try to purge irrelevant actions
	menu->clear ();
	QList<QAction*> entries = menuBar ()->actions ();
	for (int i = 0; i < entries.size (); ++i) {
		QMenu *smenu = entries[i]->menu ();
		if (!smenu) continue;    // Don't think it can happen...
		if (entries[i]->objectName () == "settings") continue;  // Skip settings menu, entirely

		QList<QAction*> subentries = smenu->actions ();
		QList<QAction*> entries_to_add;
		bool menu_empty = true;
		for (int j = 0; j < subentries.size (); ++j) {
			QAction *act = subentries[j];
			if (act->isVisible () && act->isEnabled () && act) {
				entries_to_add.append (act);
				if (!act->isSeparator ()) menu_empty = false;  // Copy separators, but purge menus with only separators in them.
			}
		}
		if (menu_empty) continue;

		QWidgetAction *act = new QWidgetAction (this);
		QLabel *lab = new QLabel ("<b>" + entries[i]->text ().replace ('&', "") + "</b>");
		lab->setAlignment (Qt::AlignCenter);
		act->setDefaultWidget (lab);
		menu->addAction (act);

		QMenu *where_to_add = menu;
		if (entries_to_add.size () >= 8) {                     // if there are really many entries in the menu don't flatten it, keep it as a (shortened) submenu
			where_to_add = menu->addMenu (entries[i]->text ());
		}
		for (int j = 0; j < entries_to_add.size (); ++j) {
			where_to_add->addAction (entries_to_add[j]);
		}
	}
}

#include "rkxmlguipreviewarea.moc"
