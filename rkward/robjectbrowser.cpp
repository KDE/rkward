/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
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
#include "robjectbrowser.h"

#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>

#include "rkglobals.h"
#include "rkeditormanager.h"
#include "core/robjectlist.h"
#include "core/rkmodificationtracker.h"
#include "misc/rkobjectlistview.h"
#include "dataeditor/rkeditor.h"
#include "robjectviewer.h"

#include "debug.h"

RObjectBrowser::RObjectBrowser () : RKToggleWidget () {
	QGridLayout *grid = new QGridLayout (this, 1, 1);
	QVBoxLayout *vbox = new QVBoxLayout ();
	grid->addLayout (vbox, 0, 0);
	
	list_view = new RKObjectListView (this);
	vbox->addWidget (list_view);

	update_button = new QPushButton (i18n ("Update"), this);
	vbox->addWidget (update_button);
	
	setCaption (i18n ("Objects in the R workspace"));

	menu = new QPopupMenu (this);
	menu->insertItem (i18n ("Edit"), this, SLOT (popupEdit ()), 0, Edit);
	menu->insertItem (i18n ("View"), this, SLOT (popupView ()), 0, View);
	menu->insertItem (i18n ("Rename"), this, SLOT (popupRename ()), 0, Rename);
	menu->insertItem (i18n ("Delete"), this, SLOT (popupDelete ()), 0, Delete);
}

RObjectBrowser::~RObjectBrowser () {
}

void RObjectBrowser::initialize () {
	list_view->initialize (false);
	
	connect (update_button, SIGNAL (clicked ()), this, SLOT (updateButtonClicked ()));
	connect (list_view, SIGNAL (contextMenuRequested (QListViewItem*, const QPoint &, int)), this, SLOT (requestedContextMenu (QListViewItem*, const QPoint &, int)));
}

void RObjectBrowser::updateButtonClicked () {
	RKGlobals::rObjectList ()->updateFromR ();
}

void RObjectBrowser::popupEdit () {
	if (menu_object) RKGlobals::editorManager ()->editObject (menu_object);
}

void RObjectBrowser::popupView () {
	RKGlobals::editorManager ()->flushAll ();
	new RObjectViewer (0, menu_object);
}

void RObjectBrowser::popupDelete () {
	RKGlobals::tracker ()->removeObject (menu_object);
}

void RObjectBrowser::popupRename () {
	bool ok;
	QString name = KInputDialog::getText (i18n ("Rename object"), i18n ("Enter the new name (make it a valid one - no checks so far)"), menu_object->getShortName (), &ok, this);

	if (ok) {
		RKGlobals::tracker ()->renameObject (menu_object, name);
	}
}

void RObjectBrowser::requestedContextMenu (QListViewItem *item, const QPoint &pos, int) {
	RObject *object = list_view->findItemObject (item);
	
	if (!object) return;
	if (object == RKGlobals::rObjectList ()) return;
	
	menu->setItemEnabled (Edit, RKGlobals::editorManager ()->canEditObject (object));
	menu_object = object;
	menu->popup (pos);
}

#include "robjectbrowser.moc"
