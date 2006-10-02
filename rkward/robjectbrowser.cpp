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

#include "rkward.h"
#include "windows/rkcommandeditorwindow.h"
#include "rkglobals.h"
#include "core/robjectlist.h"
#include "core/rkmodificationtracker.h"
#include "misc/rkobjectlistview.h"
#include "windows/rkworkplace.h"
#include "dataeditor/rkeditor.h"
#include "robjectviewer.h"

#include "debug.h"

RObjectBrowser::RObjectBrowser () : QWidget () {
	QGridLayout *grid = new QGridLayout (this, 1, 1);
	QVBoxLayout *vbox = new QVBoxLayout ();
	grid->addLayout (vbox, 0, 0);
	
	list_view = new RKObjectListView (this);
	vbox->addWidget (list_view);

	update_button = new QPushButton (i18n ("Update"), this);
	vbox->addWidget (update_button);
	
	setCaption (i18n ("Objects in the R workspace"));

	list_view->contextMenu ()->insertItem (i18n ("Edit"), this, SLOT (popupEdit ()), 0, Edit, 0);
	list_view->contextMenu ()->insertItem (i18n ("View"), this, SLOT (popupView ()), 0, View, 1);
	list_view->contextMenu ()->insertItem (i18n ("Rename"), this, SLOT (popupRename ()), 0, Rename, 2);
	list_view->contextMenu ()->insertItem (i18n ("Delete"), this, SLOT (popupDelete ()), 0, Delete, 3);
	list_view->contextMenu ()->insertSeparator (4);
	connect (list_view, SIGNAL (aboutToShowContextMenu (RKListViewItem*, bool*)), this, SLOT (contextMenuCallback (RKListViewItem*, bool*)));
	
	connect (list_view, SIGNAL (doubleClicked (QListViewItem *, const QPoint &, int )), this, SLOT (slotListDoubleClicked (QListViewItem *, const QPoint &, int)));
	
	resize (minimumSizeHint ().expandedTo (QSize (400, 480)));
}

RObjectBrowser::~RObjectBrowser () {
}

void RObjectBrowser::initialize () {
	list_view->initialize (true);
	
	connect (update_button, SIGNAL (clicked ()), this, SLOT (updateButtonClicked ()));
}

void RObjectBrowser::updateButtonClicked () {
	RObjectList::getObjectList ()->updateFromR ();
}

void RObjectBrowser::popupEdit () {
	if (list_view->menuObject ()) RKWorkplace::mainWorkplace ()->editObject (list_view->menuObject ());
}

void RObjectBrowser::popupView () {
	RKWorkplace::mainWorkplace ()->flushAllData ();
	new RObjectViewer (0, list_view->menuObject ());
}

void RObjectBrowser::popupDelete () {
	RKGlobals::tracker ()->removeObject (list_view->menuObject ());
}

void RObjectBrowser::popupRename () {
	bool ok;
	QString name = KInputDialog::getText (i18n ("Rename object"), i18n ("Enter the new name"), list_view->menuObject ()->getShortName (), &ok, this);
	
	if (ok) {
		QString valid = list_view->menuObject ()->getContainer ()->validizeName (name);
		if (valid != name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1").arg (valid), i18n ("Invalid Name"));
		RKGlobals::tracker ()->renameObject (list_view->menuObject (), valid);
	}
}

void RObjectBrowser::contextMenuCallback (RKListViewItem *, bool *) {
	RObject *object = list_view->menuObject ();
	QPopupMenu *menu = list_view->contextMenu ();

	if ((!object) || (object == RObjectList::getObjectList ())) {
		menu->setItemVisible (Edit, false);
		menu->setItemVisible (View, false);
		menu->setItemVisible (Rename, false);
		menu->setItemVisible (Delete, false);

		return;
	}

	menu->setItemVisible (Edit, true);
	menu->setItemEnabled (Edit, RKWorkplace::mainWorkplace ()->canEditObject (object));
	menu->setItemVisible (View, true);
	menu->setItemVisible (Rename, true);
	menu->setItemVisible (Delete, true);
}

void RObjectBrowser::slotListDoubleClicked (QListViewItem *item, const QPoint &, int) {
	RObject *object = list_view->findItemObject (static_cast<RKListViewItem*> (item));
	
	if (!object) return;
	if (object == RObjectList::getObjectList ()) return;
	QWidget *w = RKWorkplace::mainWorkplace ()->activeAttachedWindow ();
	if (!w) return;
	
	if (w->inherits ("RKCommandEditorWindow")) {
		static_cast<RKCommandEditorWindow*> (w)->insertText (object->getFullName ());
	}
}

#include "robjectbrowser.moc"

