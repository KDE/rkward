/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006, 2007, 2008, 2009 by Thomas Friedrichsmeier
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
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <QHBoxLayout>
#include <QFocusEvent>
#include <QVBoxLayout>
#include <QMenu>
#include <QButtonGroup>

#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kvbox.h>

#include "../rkward.h"
#include "rkhelpsearchwindow.h"
#include "../rkglobals.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rkmodificationtracker.h"
#include "../rbackend/rinterface.h"
#include "../misc/rkobjectlistview.h"
#include "../misc/rkdummypart.h"
#include "rkworkplace.h"
#include "../dataeditor/rkeditor.h"

#include "../debug.h"

// static
RObjectBrowser* RObjectBrowser::object_browser = 0;

RObjectBrowser::RObjectBrowser (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, WorkspaceBrowserWindow, tool_window, name) {
	RK_TRACE (APP);

	internal = 0;
	locked = true;

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	layout_widget = new KVBox (this);
	layout->addWidget (layout_widget);
	layout_widget->setFocusPolicy (Qt::StrongFocus);

	RKDummyPart *part = new RKDummyPart (this, layout_widget);
	setPart (part);
	initializeActivationSignals ();

	setCaption (i18n ("Objects in the R workspace"));
}

RObjectBrowser::~RObjectBrowser () {
	RK_TRACE (APP);
}

void RObjectBrowser::unlock () {
	RK_TRACE (APP);

	locked = false;
	if (!isHidden ()) {
		initialize ();
	}
}

void RObjectBrowser::showEvent (QShowEvent *e) {
	RK_TRACE (APP);

	initialize ();
	RKMDIWindow::showEvent (e);
}

void RObjectBrowser::initialize () {
	RK_TRACE (APP);

	if (internal) return;
	if (locked) return;

	RK_DO (qDebug ("creating workspace browser"), APP, DL_INFO);

	internal = new RObjectBrowserInternal (layout_widget);
	setFocusProxy (internal);
	setMinimumSize (internal->minimumSize ());
}


///////////////////////// RObjectBrowserInternal /////////////////////////////
RObjectBrowserInternal::RObjectBrowserInternal (QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);
	setFocusPolicy (Qt::ClickFocus);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	list_view = new RKObjectListView (this);
	vbox->addWidget (new RKObjectListViewSettingsWidget (list_view->getSettings (), this));
	vbox->addWidget (list_view);

	update_button = new QPushButton (i18n ("Update"), this);
	vbox->addWidget (update_button);

	actions.insert (Help, new QAction (i18n ("Search Help"), this));
	connect (actions[Help], SIGNAL(triggered(bool)), this, SLOT(popupHelp()));
	actions.insert (Edit, new QAction (i18n ("Edit"), this));
	connect (actions[Edit], SIGNAL(triggered(bool)), this, SLOT(popupEdit()));
	actions.insert (View, new QAction (i18n ("View"), this));
	connect (actions[View], SIGNAL(triggered(bool)), this, SLOT(popupView()));
	actions.insert (Rename, new QAction (i18n ("Rename"), this));
	connect (actions[Rename], SIGNAL(triggered(bool)), this, SLOT(popupRename()));
	actions.insert (Copy, new QAction (i18n ("Copy to new symbol"), this));
	connect (actions[Copy], SIGNAL(triggered(bool)), this, SLOT(popupCopy()));
	actions.insert (CopyToGlobalEnv, new QAction (i18n ("Copy to .GlobalEnv"), this));
	connect (actions[CopyToGlobalEnv], SIGNAL(triggered(bool)), this, SLOT(popupCopyToGlobalEnv()));
	actions.insert (Delete, new QAction (i18n ("Delete"), this));
	connect (actions[Delete], SIGNAL(triggered(bool)), this, SLOT(popupDelete()));
	actions.insert (Unload, new QAction (i18n ("Unload Package"), this));
	connect (actions[Unload], SIGNAL(triggered(bool)), this, SLOT(popupUnload()));
	actions.insert (LoadUnloadPackages, new QAction (i18n ("Load / Unload Packages"), this));
	connect (actions[LoadUnloadPackages], SIGNAL(triggered(bool)), RKWardMainWindow::getMain(), SLOT(slotFileLoadLibs()));

	QAction* sep = list_view->contextMenu ()->insertSeparator (list_view->contextMenu ()->actions ().value (0));
	list_view->contextMenu ()->insertActions (sep, actions);

	connect (list_view, SIGNAL (aboutToShowContextMenu (RObject *, bool*)), this, SLOT (contextMenuCallback (RObject*, bool*)));
	
	connect (list_view, SIGNAL (doubleClicked(const QModelIndex&)), this, SLOT (doubleClicked(const QModelIndex&)));
	
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
	if (e->reason () != Qt::MouseFocusReason) {
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
		if (valid != name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1", valid), i18n ("Invalid Name"));
		RKGlobals::rInterface ()->issueCommand (RObject::rQuote (valid) + " <- " + object->getFullName (), RCommand::App | RCommand::ObjectListUpdate);
	}
}

void RObjectBrowserInternal::popupCopyToGlobalEnv () {
	RK_TRACE (APP);

	RObject *object = list_view->menuObject ();
	QString name = object->getShortName ();

	QString valid = RObjectList::getGlobalEnv ()->validizeName (name);
	if (valid != name) KMessageBox::sorry (this, i18n ("An object named '%1' already exists in the GlobalEnv. Created the copy as '%2' instead.", name, valid), i18n ("Name already in use"));
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

void RObjectBrowserInternal::popupUnload () {
	RK_TRACE (APP);

	RObject *object = list_view->menuObject ();
	RK_ASSERT (object);
	RK_ASSERT (object->isType (RObject::PackageEnv));

	QStringList messages = RObjectList::getObjectList ()->detachPackages (QStringList (object->getShortName ()));

	if (!messages.isEmpty ()) KMessageBox::sorry (this, messages.join ("\n"));
}

void RObjectBrowserInternal::popupRename () {
	RK_TRACE (APP);
	bool ok;
	QString name = KInputDialog::getText (i18n ("Rename object"), i18n ("Enter the new name"), list_view->menuObject ()->getShortName (), &ok, this);
	
	if (ok) {
		QString valid = list_view->menuObject ()->getContainer ()->validizeName (name);
		if (valid != name) KMessageBox::sorry (this, i18n ("The name you specified was already in use or not valid. Renamed to %1", valid), i18n ("Invalid Name"));
		RKGlobals::tracker ()->renameObject (list_view->menuObject (), valid);
	}
}

void RObjectBrowserInternal::contextMenuCallback (RObject *, bool *) {
	RK_TRACE (APP);
	RObject *object = list_view->menuObject ();

	if (!object) {
		RK_ASSERT (actions.size () == ActionCount);
		for (int i = 0; i < ActionCount; ++i) {
			actions[i]->setVisible (false);
		}
		actions[LoadUnloadPackages]->setVisible (true);
		return;
	}

	actions[Help]->setVisible (!(object->isType (RObject::ToplevelEnv) || object->isInGlobalEnv ()));
	actions[Edit]->setVisible (object->canEdit () && RKWorkplace::mainWorkplace ()->canEditObject (object));
	actions[View]->setVisible (object->canRead ());
	actions[Rename]->setVisible (object->canRename ());
	actions[Copy]->setVisible (object->canRead () && (!object->isType (RObject::ToplevelEnv)));
	actions[CopyToGlobalEnv]->setVisible (object->canRead () && (!object->isInGlobalEnv()) && (!object->isType (RObject::ToplevelEnv)));
	actions[Delete]->setVisible (object->canRemove ());
	actions[Unload]->setVisible (object->isType (RObject::PackageEnv));
	actions[LoadUnloadPackages]->setVisible (object == RObjectList::getObjectList ());
}

void RObjectBrowserInternal::doubleClicked (const QModelIndex& index) {
	RK_TRACE (APP);

	RObject *object = list_view->objectAtIndex (index);
	if (!object) return;
	if (object == RObjectList::getObjectList ()) return;

	if (object->canEdit () && RKWorkplace::mainWorkplace ()->canEditObject (object)) {
		RKWorkplace::mainWorkplace ()->editObject (object);
	} else {
		RKWorkplace::mainWorkplace ()->flushAllData ();
		RKWorkplace::mainWorkplace ()->newObjectViewer (object);
	}
}


//////////////////// RKObjectListViewSettingsWidget //////////////////////////
RKObjectListViewSettingsWidget::RKObjectListViewSettingsWidget (RKObjectListViewSettings *settings, QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);

	RKObjectListViewSettingsWidget::settings = settings;
	connect (settings, SIGNAL (settingsChanged ()), this, SLOT (settingsChanged ()));

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);

	QButtonGroup *group = new QButtonGroup (this);
	QHBoxLayout *hbox = new QHBoxLayout ();
	hbox->setContentsMargins (0, 0, 0, 0);
	group->addButton (all = new QRadioButton (i18n ("All"), this));
	group->addButton (nonfunctions = new QRadioButton (i18n ("Non-Functions"), this));
	group->addButton (functions = new QRadioButton (i18n ("Functions"), this));
	hbox->addWidget (all);
	hbox->addWidget (nonfunctions);
	hbox->addWidget (functions);
	connect (all, SIGNAL(clicked(bool)), this, SLOT(modeChanged(bool)));
	connect (nonfunctions, SIGNAL(clicked(bool)), this, SLOT(modeChanged(bool)));
	connect (functions, SIGNAL(clicked(bool)), this, SLOT(modeChanged(bool)));
	layout->addLayout (hbox);

	all_envirs = new QCheckBox (i18n ("Show All Environments"), this);
	connect (all_envirs, SIGNAL (clicked(bool)), this, SLOT (boxChanged(bool)));
	layout->addWidget (all_envirs);

	hidden_objects = new QCheckBox (i18n ("Show Hidden Objects"), this);
	connect (hidden_objects, SIGNAL (clicked(bool)), this, SLOT (boxChanged(bool)));
	layout->addWidget (hidden_objects);

	settingsChanged ();
}

RKObjectListViewSettingsWidget::~RKObjectListViewSettingsWidget () {
	RK_TRACE (APP);
}

void RKObjectListViewSettingsWidget::settingsChanged () {
	RK_TRACE (APP);

	all_envirs->setChecked (settings->getSetting (RKObjectListViewSettings::ShowObjectsAllEnvironments));
	hidden_objects->setChecked (settings->getSetting (RKObjectListViewSettings::ShowObjectsHidden));

	bool functions_shown = settings->getSetting (RKObjectListViewSettings::ShowObjectsFunction);
	bool vars_shown = settings->getSetting (RKObjectListViewSettings::ShowObjectsVariable);
	bool containers_shown = settings->getSetting (RKObjectListViewSettings::ShowObjectsContainer);

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

void RKObjectListViewSettingsWidget::modeChanged (bool) {
	RK_TRACE (APP);

	if (all->isChecked ()) {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsFunction, true);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsVariable, true);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsContainer, true);
	} else if (functions->isChecked ()) {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsFunction, true);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsVariable, false);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsContainer, false);
	} else if (nonfunctions->isChecked ()) {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsFunction, false);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsVariable, true);
		settings->setSetting (RKObjectListViewSettings::ShowObjectsContainer, true);
	} else {
		RK_ASSERT (false);
	}
}

void RKObjectListViewSettingsWidget::boxChanged (bool) {
	RK_TRACE (APP);

	if (sender () == all_envirs) {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsAllEnvironments, all_envirs->isChecked ());
	} else {
		settings->setSetting (RKObjectListViewSettings::ShowObjectsHidden, hidden_objects->isChecked ());
	}
}

#include "robjectbrowser.moc"
