/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qvbox.h>

#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>

#include "rkward.h"
#include "windows/rkhelpsearchwindow.h"
#include "windows/rkcommandeditorwindow.h"
#include "rkglobals.h"
#include "core/robjectlist.h"
#include "core/renvironmentobject.h"
#include "core/rkmodificationtracker.h"
#include "rbackend/rinterface.h"
#include "misc/rkobjectlistview.h"
#include "misc/rkdummypart.h"
#include "windows/rkworkplace.h"
#include "dataeditor/rkeditor.h"

#include "debug.h"

// static
RObjectBrowser* RObjectBrowser::object_browser = 0;

RObjectBrowser::RObjectBrowser (QWidget *parent, bool tool_window, char *name) : RKMDIWindow (parent, WorkspaceBrowserWindow, tool_window, name) {
	RK_TRACE (APP);

	internal = 0;
	locked = true;

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout_widget = new QVBox (this);
	layout->addWidget (layout_widget);
	layout_widget->setFocusPolicy (QWidget::StrongFocus);

	RKDummyPart *part = new RKDummyPart (this, layout_widget);
	setPart (part);
	initializeActivationSignals ();
}

RObjectBrowser::~RObjectBrowser () {
	RK_TRACE (APP);
}

void RObjectBrowser::unlock () {
	RK_TRACE (APP);

	locked = false;
	if (isShown ()) {
		initialize ();
	}
}

void RObjectBrowser::show () {
	RK_TRACE (APP);

	initialize ();
	RKMDIWindow::show ();
}

void RObjectBrowser::initialize () {
	RK_TRACE (APP);

	if (internal) return;
	if (locked) return;

	RK_DO (qDebug ("creating workspace browser"), APP, DL_INFO);

	internal = new RObjectBrowserInternal (layout_widget);
	layout_widget->setFocusProxy (internal);
	setMinimumSize (internal->minimumSize ());
}


///////////////////////// RObjectBrowserInternal /////////////////////////////
RObjectBrowserInternal::RObjectBrowserInternal (QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);
	setFocusPolicy (QWidget::ClickFocus);

	QVBoxLayout *vbox = new QVBoxLayout (this);

	list_view = new RKObjectListView (this);
	vbox->addWidget (new RKObjectListViewSettingsWidget (list_view->getSettings (), this));
	vbox->addWidget (list_view);

	update_button = new QPushButton (i18n ("Update"), this);
	vbox->addWidget (update_button);
	
	setCaption (i18n ("Objects in the R workspace"));

	list_view->contextMenu ()->insertItem (i18n ("Search Help"), this, SLOT (popupHelp ()), 0, Help, 0);
	list_view->contextMenu ()->insertItem (i18n ("Edit"), this, SLOT (popupEdit ()), 0, Edit, 1);
	list_view->contextMenu ()->insertItem (i18n ("View"), this, SLOT (popupView ()), 0, View, 2);
	list_view->contextMenu ()->insertItem (i18n ("Rename"), this, SLOT (popupRename ()), 0, Rename, 3);
	list_view->contextMenu ()->insertItem (i18n ("Copy to new symbol"), this, SLOT (popupCopy ()), 0, Copy, 4);
	list_view->contextMenu ()->insertItem (i18n ("Copy to .GlobalEnv"), this, SLOT (popupCopyToGlobalEnv ()), 0, CopyToGlobalEnv, 5);
	list_view->contextMenu ()->insertItem (i18n ("Delete"), this, SLOT (popupDelete ()), 0, Delete, 6);
	list_view->contextMenu ()->insertSeparator (7);
	connect (list_view, SIGNAL (aboutToShowContextMenu (RKListViewItem*, bool*)), this, SLOT (contextMenuCallback (RKListViewItem*, bool*)));
	
	connect (list_view, SIGNAL (doubleClicked (QListViewItem *, const QPoint &, int )), this, SLOT (slotListDoubleClicked (QListViewItem *, const QPoint &, int)));
	
	resize (minimumSizeHint ().expandedTo (QSize (400, 480)));

	list_view->initialize ();
	connect (update_button, SIGNAL (clicked ()), this, SLOT (updateButtonClicked ()));
}

RObjectBrowserInternal::~RObjectBrowserInternal () {
	RK_TRACE (APP);
}

void RObjectBrowserInternal::focusInEvent (QFocusEvent *e) {
	RK_TRACE (APP);

	list_view->setFocus ();
	if (e->reason () != QFocusEvent::Mouse) {
		list_view->setObjectCurrent (RObjectList::getGlobalEnv (), true);
	}
}

void RObjectBrowserInternal::updateButtonClicked () {
	RK_TRACE (APP);
	RObjectList::getObjectList ()->updateFromR (0);
}

void RObjectBrowserInternal::popupHelp () {
	RK_TRACE (APP);

	if (list_view->menuObject ()) RKHelpSearchWindow::mainHelpSearch ()->getFunctionHelp (list_view->menuObject ()->getShortName ());
}

void RObjectBrowserInternal::popupEdit () {
	RK_TRACE (APP);
	if (list_view->menuObject ()) RKWorkplace::mainWorkplace ()->editObject (list_view->menuObject ());
}

void RObjectBrowserInternal::popupCopy () {
	RK_TRACE (APP);

	bool ok;
	RObject *object = list_view->menuObject ();
	QString suggested_name = RObjectList::getGlobalEnv ()->validizeName (object->getShortName ());
	QString name = KInputDialog::getText (i18n ("Copy object"), i18n ("Enter the name to copy to"), suggested_name, &ok, this);

	if (ok) {
		QString valid = RObjectList::getGlobalEnv ()->validizeName (name);
		if (valid != name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1").arg (valid), i18n ("Invalid Name"));
		RKGlobals::rInterface ()->issueCommand (RObject::rQuote (valid) + " <- " + object->getFullName (), RCommand::App | RCommand::ObjectListUpdate);
	}
}

void RObjectBrowserInternal::popupCopyToGlobalEnv () {
	RK_TRACE (APP);

	RObject *object = list_view->menuObject ();
	QString name = object->getShortName ();

	QString valid = RObjectList::getGlobalEnv ()->validizeName (name);
	if (valid != name) KMessageBox::sorry (this, i18n ("An object named '%1' already exists in the GlobalEnv. Created the copy as '%2' instead.").arg (name).arg (valid), i18n ("Name already in use"));
	RKGlobals::rInterface ()->issueCommand (RObject::rQuote (valid) + " <- " + object->getFullName (), RCommand::App | RCommand::ObjectListUpdate);
}

void RObjectBrowserInternal::popupView () {
	RK_TRACE (APP);
	RKWorkplace::mainWorkplace ()->flushAllData ();
	RKWorkplace::mainWorkplace ()->newObjectViewer (list_view->menuObject ());
}

void RObjectBrowserInternal::popupDelete () {
	RK_TRACE (APP);
	RKGlobals::tracker ()->removeObject (list_view->menuObject ());
}

void RObjectBrowserInternal::popupRename () {
	RK_TRACE (APP);
	bool ok;
	QString name = KInputDialog::getText (i18n ("Rename object"), i18n ("Enter the new name"), list_view->menuObject ()->getShortName (), &ok, this);
	
	if (ok) {
		QString valid = list_view->menuObject ()->getContainer ()->validizeName (name);
		if (valid != name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1").arg (valid), i18n ("Invalid Name"));
		RKGlobals::tracker ()->renameObject (list_view->menuObject (), valid);
	}
}

void RObjectBrowserInternal::contextMenuCallback (RKListViewItem *, bool *) {
	RK_TRACE (APP);
	RObject *object = list_view->menuObject ();
	QPopupMenu *menu = list_view->contextMenu ();

	if (!object) {
		menu->setItemVisible (Help, false);
		menu->setItemVisible (Edit, false);
		menu->setItemVisible (View, false);
		menu->setItemVisible (Rename, false);
		menu->setItemVisible (Copy, false);
		menu->setItemVisible (CopyToGlobalEnv, false);
		menu->setItemVisible (Delete, false);

		return;
	}

	menu->setItemVisible (Help, !(object->isType (RObject::ToplevelEnv) || object->isInGlobalEnv ()));
	menu->setItemVisible (Edit, object->canEdit () && RKWorkplace::mainWorkplace ()->canEditObject (object));
	menu->setItemVisible (View, object->canRead ());
	menu->setItemVisible (Rename, object->canRename ());
	menu->setItemVisible (Copy, object->canRead () && (!object->isType (RObject::ToplevelEnv)));
	menu->setItemVisible (CopyToGlobalEnv, object->canRead () && (!object->isInGlobalEnv()) && (!object->isType (RObject::ToplevelEnv)));
	menu->setItemVisible (Delete, object->canRemove ());
}

void RObjectBrowserInternal::slotListDoubleClicked (QListViewItem *item, const QPoint &, int) {
	RK_TRACE (APP);
	RObject *object = list_view->findItemObject (static_cast<RKListViewItem*> (item));
	
	if (!object) return;
	if (object == RObjectList::getObjectList ()) return;
	QWidget *w = RKWorkplace::mainWorkplace ()->activeWindow (RKMDIWindow::Attached);
	if (!w) return;
	
	if (w->inherits ("RKCommandEditorWindow")) {
		static_cast<RKCommandEditorWindow*> (w)->insertText (object->getFullName ());
	}
}


//////////////////// RKObjectListViewSettingsWidget //////////////////////////
RKObjectListViewSettingsWidget::RKObjectListViewSettingsWidget (RKObjectListViewSettings *settings, QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);

	RKObjectListViewSettingsWidget::settings = settings;
	connect (settings, SIGNAL (settingsChanged ()), this, SLOT (settingsChanged ()));

	QVBoxLayout *layout = new QVBoxLayout (this);
	group = new QButtonGroup (this);
	QHBoxLayout *grouplayout = new QHBoxLayout (group);
	all = new QRadioButton (i18n ("All"), group);
	nonfunctions = new QRadioButton (i18n ("Non-Functions"), group);
	functions = new QRadioButton (i18n ("Functions"), group);
	grouplayout->addWidget (all);
	grouplayout->addWidget (nonfunctions);
	grouplayout->addWidget (functions);
	connect (group, SIGNAL (clicked (int)), this, SLOT (modeActivated (int)));
	layout->addWidget (group);

	all_envirs = new QCheckBox (i18n ("Show All Environments"), this);
	connect (all_envirs, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	layout->addWidget (all_envirs);

	hidden_objects = new QCheckBox (i18n ("Show Hidden Objects"), this);
	connect (hidden_objects, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	layout->addWidget (hidden_objects);

	settingsChanged ();
}

RKObjectListViewSettingsWidget::~RKObjectListViewSettingsWidget () {
	RK_TRACE (APP);
}

void RKObjectListViewSettingsWidget::settingsChanged () {
	RK_TRACE (APP);

	all_envirs->setChecked (settings->settingActive (RKObjectListViewSettings::ShowObjectsAllEnvironments));
	all_envirs->setEnabled (settings->optionConfigurable (RKObjectListViewSettings::ShowObjectsAllEnvironments));

	hidden_objects->setChecked (settings->settingActive (RKObjectListViewSettings::ShowObjectsHidden));
	hidden_objects->setEnabled (settings->optionConfigurable (RKObjectListViewSettings::ShowObjectsHidden));

	bool functions_shown = settings->settingActive (RKObjectListViewSettings::ShowObjectsFunction);
	bool functions_configurable = settings->optionConfigurable (RKObjectListViewSettings::ShowObjectsFunction);

	bool vars_shown = settings->settingActive (RKObjectListViewSettings::ShowObjectsVariable);
	bool vars_configurable = settings->optionConfigurable (RKObjectListViewSettings::ShowObjectsVariable);

	bool containers_shown = settings->settingActive (RKObjectListViewSettings::ShowObjectsContainer);
	bool containers_configurable = settings->optionConfigurable (RKObjectListViewSettings::ShowObjectsContainer);

	if (functions_configurable && (vars_configurable || containers_configurable)) group->show ();
	else group->hide ();

	if (functions_shown && vars_shown && containers_shown) {
		all->setChecked (true);
	} else if (vars_shown && containers_shown) {
		nonfunctions->setChecked (true);
	} else if (functions_shown && (!(vars_shown || containers_shown))) {
		functions->setChecked (true);
	} else {
		all->setChecked (false);
		nonfunctions->setChecked (false);
		functions->setChecked (false);
	}
}

void RKObjectListViewSettingsWidget::modeActivated (int mode) {
	RK_TRACE (APP);

	if (mode == All) {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsFunction, RKObjectListViewSettings::Yes);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsVariable, RKObjectListViewSettings::Yes);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsContainer, RKObjectListViewSettings::Yes);
	} else if (mode == Functions) {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsFunction, RKObjectListViewSettings::Yes);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsVariable, RKObjectListViewSettings::No);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsContainer, RKObjectListViewSettings::No);
	} else if (mode == NonFunctions) {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsFunction, RKObjectListViewSettings::No);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsVariable, RKObjectListViewSettings::Yes);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsContainer, RKObjectListViewSettings::Yes);
	} else {
		RK_ASSERT (false);
	}
}

void RKObjectListViewSettingsWidget::boxChanged (int) {
	RK_TRACE (APP);

	if (sender () == all_envirs) {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsAllEnvironments, all_envirs->isChecked () ? RKObjectListViewSettings::Yes : RKObjectListViewSettings::No);
	} else {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsHidden, hidden_objects->isChecked () ? RKObjectListViewSettings::Yes : RKObjectListViewSettings::No);
	}
}

#include "robjectbrowser.moc"
