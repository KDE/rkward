/*
robjectbrowser - This file is part of RKWard (https://rkward.kde.org). Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "robjectbrowser.h"

#include <QPushButton>
#include <QFocusEvent>
#include <QVBoxLayout>
#include <QMenu>
#include <QInputDialog>
#include <QApplication>
#include <QMimeData>
#include <QClipboard>

#include <KLocalizedString>
#include <kmessagebox.h>

#include "../rkward.h"
#include "rkhelpsearchwindow.h"
#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../core/rkmodificationtracker.h"
#include "../rbackend/rkrinterface.h"
#include "../misc/rkobjectlistview.h"
#include "../misc/rkdummypart.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkstandardactions.h"
#include "../misc/rkspecialactions.h"
#include "rkworkplace.h"
#include "../dataeditor/rkeditor.h"

#include "../debug.h"

// static
RObjectBrowser* RObjectBrowser::object_browser = nullptr;

RObjectBrowser::RObjectBrowser (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, WorkspaceBrowserWindow, tool_window, name) {
	RK_TRACE (APP);

	internal = nullptr;
	locked = true;

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	layout_widget = new QWidget (this);
	layout->addWidget (layout_widget);
	layout_widget->setFocusPolicy (Qt::StrongFocus);

	RKDummyPart *part = new RKDummyPart (this, layout_widget);
	setPart (part);
	setMetaInfo (i18n ("R workspace browser"), QUrl ("rkward://page/rkward_workspace_browser"), RKSettings::PageObjectBrowser);
	initializeActivationSignals ();

	setCaption (i18n ("R Workspace"));
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

	RK_DEBUG (APP, DL_INFO, "creating workspace browser");

	internal = new RObjectBrowserInternal (layout_widget, this);
	QVBoxLayout *l = new QVBoxLayout (layout_widget);
	l->setContentsMargins (0, 0, 0, 0);
	l->addWidget (internal);

	setFocusProxy (internal);
	setMinimumSize (internal->minimumSize ());
}


///////////////////////// RObjectBrowserInternal /////////////////////////////
RObjectBrowserInternal::RObjectBrowserInternal (QWidget *parent, RObjectBrowser *browser) : QWidget (parent) {
	RK_TRACE (APP);
	setFocusPolicy (Qt::ClickFocus);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	list_view = new RKObjectListView (true, this);
	vbox->addWidget (list_view->getSettings ()->filterWidget (this));
	vbox->addWidget (list_view);

	update_button = new QPushButton (i18n ("Update"), this);
	vbox->addWidget (update_button);

	actions.insert (Help, RKStandardActions::functionHelp (browser, this));
	actions.insert (SearchOnline, RKStandardActions::onlineHelp (browser, this));
	actions.insert (Edit, new QAction (i18n ("Edit"), this));
	connect (actions[Edit], &QAction::triggered, this, &RObjectBrowserInternal::popupEdit);
	actions.insert (View, new QAction (i18n ("View"), this));
	connect (actions[View], &QAction::triggered, this, &RObjectBrowserInternal::popupView);
	actions.insert (Rename, new QAction (i18n ("Rename"), this));
	connect (actions[Rename], &QAction::triggered, this, &RObjectBrowserInternal::popupRename);
	actions.insert (Copy, new QAction (i18n ("Copy to new symbol"), this));
	connect (actions[Copy], &QAction::triggered, this, &RObjectBrowserInternal::popupCopy);
	actions.insert (CopyToGlobalEnv, new QAction (i18n ("Copy to .GlobalEnv"), this));
	connect (actions[CopyToGlobalEnv], &QAction::triggered, this, &RObjectBrowserInternal::popupCopyToGlobalEnv);
	actions.insert (Delete, new QAction (i18n ("Delete"), this));
	connect (actions[Delete], &QAction::triggered, this, &RObjectBrowserInternal::popupDelete);
	actions.insert(NewFromClipboard, new QAction(QIcon::fromTheme("edit-paste"), i18n("New object from clipboard"), this));
	connect (actions[NewFromClipboard], &QAction::triggered, this, []() { RKPasteSpecialDialog dia(RKWardMainWindow::getMain(), true); dia.exec(); });
	actions.insert (Unload, new QAction (i18n ("Unload Package"), this));
	connect (actions[Unload], &QAction::triggered, this, &RObjectBrowserInternal::popupUnload);
	actions.insert (LoadUnloadPackages, new QAction (i18n ("Load / Unload Packages"), this));
	connect (actions[LoadUnloadPackages], &QAction::triggered, RKWardMainWindow::getMain(), &RKWardMainWindow::slotFileLoadLibs);

	QAction* sep = list_view->contextMenu ()->insertSeparator (list_view->contextMenu ()->actions ().value (0));
	list_view->contextMenu ()->insertActions (sep, actions);

	connect (list_view, &RKObjectListView::aboutToShowContextMenu, this, &RObjectBrowserInternal::contextMenuCallback);
	
	connect (list_view, &QAbstractItemView::doubleClicked, this, &RObjectBrowserInternal::doubleClicked);
	
	resize (minimumSizeHint ().expandedTo (QSize (400, 480)));

	list_view->initialize ();
	connect (update_button, &QPushButton::clicked, this, &RObjectBrowserInternal::updateButtonClicked);
}

RObjectBrowserInternal::~RObjectBrowserInternal () {
	RK_TRACE (APP);
}

void RObjectBrowserInternal::focusInEvent (QFocusEvent *e) {
	RK_TRACE (APP);

	list_view->getSettings ()->filterWidget (this)->setFocus ();
	if (e->reason () != Qt::MouseFocusReason) {
		list_view->setObjectCurrent (RObjectList::getGlobalEnv (), true);
	}
}

void RObjectBrowserInternal::updateButtonClicked () {
	RK_TRACE (APP);
	RObjectList::getObjectList()->updateFromR(nullptr);
}

void RObjectBrowserInternal::currentHelpContext (QString* symbol, QString* package) {
	RK_TRACE (APP);

	RObject *object = list_view->menuObject ();
	if (!object) return;
	*symbol = object->getShortName ();
	*package = object->isInGlobalEnv () ? QString () : object->toplevelEnvironment ()->packageName ();
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
	QString name = QInputDialog::getText (this, i18n ("Copy object"), i18n ("Enter the name to copy to"), QLineEdit::Normal, suggested_name, &ok);

	if (ok) {
		QString valid = RObjectList::getGlobalEnv ()->validizeName (name);
		if (valid != name) KMessageBox::error(this, i18n("The name you specified was already in use or not valid. Renamed to %1", valid), i18n("Invalid Name"));
		RInterface::issueCommand (RObject::rQuote (valid) + " <- " + object->getFullName (), RCommand::App | RCommand::ObjectListUpdate);
	}
}

void RObjectBrowserInternal::popupCopyToGlobalEnv () {
	RK_TRACE (APP);

	const RObject *object = list_view->menuObject ();
	QString name = object->getShortName ();

	QString valid = RObjectList::getGlobalEnv ()->validizeName (name);
	if (valid != name) KMessageBox::error(this, i18n("An object named '%1' already exists in the GlobalEnv. Created the copy as '%2' instead.", name, valid), i18n("Name already in use"));
	RInterface::issueCommand (RObject::rQuote (valid) + " <- " + object->getFullName (), RCommand::App | RCommand::ObjectListUpdate);
}

void RObjectBrowserInternal::popupView () {
	RK_TRACE (APP);
	RKWorkplace::mainWorkplace ()->flushAllData ();
	RKWorkplace::mainWorkplace ()->newObjectViewer (list_view->menuObject ());
}

void RObjectBrowserInternal::popupDelete () {
	RK_TRACE (APP);
	RKModificationTracker::instance()->removeObject (list_view->menuObject ());
}

void RObjectBrowserInternal::popupUnload () {
	RK_TRACE (APP);

	RObject *object = list_view->menuObject ();
	RK_ASSERT (object);
	RK_ASSERT (object->isType (RObject::PackageEnv));

	QStringList messages = RObjectList::getObjectList ()->detachPackages (QStringList (object->getShortName ()));

	if (!messages.isEmpty()) KMessageBox::error(this, messages.join("\n"));
}

void RObjectBrowserInternal::popupRename () {
	RK_TRACE (APP);
	bool ok;
	QString name = QInputDialog::getText (this, i18n ("Rename object"), i18n ("Enter the new name"), QLineEdit::Normal, list_view->menuObject ()->getShortName (), &ok);
	
	if (ok) {
		QString valid = static_cast<RContainerObject*> (list_view->menuObject ()->parentObject ())->validizeName (name);
		if (valid != name) KMessageBox::error(this, i18n("The name you specified was already in use or not valid. Renamed to %1", valid), i18n("Invalid Name"));
		RKModificationTracker::instance()->renameObject (list_view->menuObject (), valid);
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
	actions[Edit]->setText (object->canWrite () ? i18n ("Edit") : i18n ("View in editor (read-only)"));
	actions[Edit]->setVisible (RKWorkplace::mainWorkplace ()->canEditObject (object));
	actions[View]->setVisible (object->canRead ());
	actions[Rename]->setVisible (object->canRename ());
	actions[Copy]->setVisible (object->canRead () && (!object->isType (RObject::ToplevelEnv)));
	actions[CopyToGlobalEnv]->setVisible (object->canRead () && (!object->isInGlobalEnv()) && (!object->isType (RObject::ToplevelEnv)));
	actions[Delete]->setVisible (object->canRemove ());
	{
		const QClipboard *clipboard = QApplication::clipboard();
		const QMimeData *mime_data = clipboard->mimeData();
		actions[NewFromClipboard]->setEnabled(mime_data->hasText());
	}
	actions[Unload]->setVisible (object->isType (RObject::PackageEnv));
	actions[LoadUnloadPackages]->setVisible (object == RObjectList::getObjectList ());
}

void RObjectBrowserInternal::doubleClicked (const QModelIndex& index) {
	RK_TRACE (APP);

	RObject *object = list_view->objectAtIndex (index);
	if (!object) return;
	if (object == RObjectList::getObjectList ()) return;

	if (object->isInGlobalEnv ()) {
		if (RKWorkplace::mainWorkplace ()->canEditObject (object)) {
			RKWorkplace::mainWorkplace ()->editObject (object);
		} else {
			RKWorkplace::mainWorkplace ()->newObjectViewer (object);
		}
	} else {
		RKHelpSearchWindow::mainHelpSearch ()->getFunctionHelp (object->getShortName (), object->toplevelEnvironment ()->packageName ());
	}
}
