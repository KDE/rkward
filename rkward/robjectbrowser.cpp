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
#include "core/renvironmentobject.h"
#include "core/rkmodificationtracker.h"
#include "rbackend/rinterface.h"
#include "misc/rkobjectlistview.h"
#include "windows/rkworkplace.h"
#include "dataeditor/rkeditor.h"
#include "robjectviewer.h"

#include "debug.h"

RObjectBrowser::RObjectBrowser () : QWidget () {
	RK_TRACE (APP);

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
	list_view->contextMenu ()->insertItem (i18n ("Copy to new symbol"), this, SLOT (popupCopy ()), 0, Copy, 3);
	list_view->contextMenu ()->insertItem (i18n ("Copy to .GlobalEnv"), this, SLOT (popupCopyToGlobalEnv ()), 0, CopyToGlobalEnv, 4);
	list_view->contextMenu ()->insertItem (i18n ("Delete"), this, SLOT (popupDelete ()), 0, Delete, 5);
	list_view->contextMenu ()->insertSeparator (6);
	connect (list_view, SIGNAL (aboutToShowContextMenu (RKListViewItem*, bool*)), this, SLOT (contextMenuCallback (RKListViewItem*, bool*)));
	
	connect (list_view, SIGNAL (doubleClicked (QListViewItem *, const QPoint &, int )), this, SLOT (slotListDoubleClicked (QListViewItem *, const QPoint &, int)));
	
	resize (minimumSizeHint ().expandedTo (QSize (400, 480)));
}

RObjectBrowser::~RObjectBrowser () {
	RK_TRACE (APP);
}

void RObjectBrowser::initialize () {
	RK_TRACE (APP);

	list_view->initialize (true);
	
	connect (update_button, SIGNAL (clicked ()), this, SLOT (updateButtonClicked ()));
}

void RObjectBrowser::updateButtonClicked () {
	RK_TRACE (APP);
	RObjectList::getObjectList ()->updateFromR ();
}

void RObjectBrowser::popupEdit () {
	RK_TRACE (APP);
	if (list_view->menuObject ()) RKWorkplace::mainWorkplace ()->editObject (list_view->menuObject ());
}

void RObjectBrowser::popupCopy () {
	RK_TRACE (APP);

	bool ok;
	RObject *object = list_view->menuObject ();
	QString suggested_name = RObjectList::getGlobalEnv ()->validizeName (object->getShortName ());
	QString name = KInputDialog::getText (i18n ("Copy object"), i18n ("Enter the name to copy to"), suggested_name, &ok, this);

	if (ok) {
		QString valid = RObjectList::getGlobalEnv ()->validizeName (name);
		if (valid != name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1").arg (valid), i18n ("Invalid Name"));
		RObject *copy = RObjectList::getGlobalEnv ()->createNewChild (valid, 0, true, true);
		RKGlobals::rInterface ()->issueCommand (RObject::rQuote (valid) + " <- " + object->getFullName (), RCommand::App);
		copy->updateFromR ();
	}
}

void RObjectBrowser::popupCopyToGlobalEnv () {
	RK_TRACE (APP);

	RObject *object = list_view->menuObject ();
	QString name = object->getShortName ();

	QString valid = RObjectList::getGlobalEnv ()->validizeName (name);
	if (valid != name) KMessageBox::sorry (this, i18n ("An object named '%1' already exists in the GlobalEnv. Created the copy as '%2' instead.").arg (name).arg (valid), i18n ("Name already in use"));
	RObject *copy = RObjectList::getGlobalEnv ()->createNewChild (valid, 0, true, true);
	RKGlobals::rInterface ()->issueCommand (RObject::rQuote (valid) + " <- " + object->getFullName (), RCommand::App);
	copy->updateFromR ();
}

void RObjectBrowser::popupView () {
	RK_TRACE (APP);
	RKWorkplace::mainWorkplace ()->flushAllData ();
	new RObjectViewer (0, list_view->menuObject ());
}

void RObjectBrowser::popupDelete () {
	RK_TRACE (APP);
	RKGlobals::tracker ()->removeObject (list_view->menuObject ());
}

void RObjectBrowser::popupRename () {
	RK_TRACE (APP);
	bool ok;
	QString name = KInputDialog::getText (i18n ("Rename object"), i18n ("Enter the new name"), list_view->menuObject ()->getShortName (), &ok, this);
	
	if (ok) {
		QString valid = list_view->menuObject ()->getContainer ()->validizeName (name);
		if (valid != name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1").arg (valid), i18n ("Invalid Name"));
		RKGlobals::tracker ()->renameObject (list_view->menuObject (), valid);
	}
}

void RObjectBrowser::contextMenuCallback (RKListViewItem *, bool *) {
	RK_TRACE (APP);
	RObject *object = list_view->menuObject ();
	QPopupMenu *menu = list_view->contextMenu ();

	if (!object) {
		menu->setItemVisible (Edit, false);
		menu->setItemVisible (View, false);
		menu->setItemVisible (Rename, false);
		menu->setItemVisible (Copy, false);
		menu->setItemVisible (CopyToGlobalEnv, false);
		menu->setItemVisible (Delete, false);

		return;
	}

	menu->setItemVisible (Edit, object->canEdit () && RKWorkplace::mainWorkplace ()->canEditObject (object));
	menu->setItemVisible (View, object->canRead ());
	menu->setItemVisible (Rename, object->canRename ());
	menu->setItemVisible (Copy, object->canRead () && (!object->isType (RObject::ToplevelEnv)));
	menu->setItemVisible (CopyToGlobalEnv, object->canRead () && (!object->isInGlobalEnv()) && (!object->isType (RObject::ToplevelEnv)));
	menu->setItemVisible (Delete, object->canRemove ());
}

void RObjectBrowser::slotListDoubleClicked (QListViewItem *item, const QPoint &, int) {
	RK_TRACE (APP);
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
