/***************************************************************************
                          rkworkplace  -  description
                             -------------------
    begin                : Thu Sep 21 2006
    copyright            : (C) 2006, 2007, 2009, 2010, 2011 by Thomas Friedrichsmeier
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

#include "rkworkplace.h"

#include <kparts/partmanager.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <khbox.h>
#include <kvbox.h>
#include <kglobalsettings.h>
#include <kactioncollection.h>
#include <krun.h>
#include <kmimetype.h>

#include <QFileInfo>

#include "detachedwindowcontainer.h"
#include "rkcommandeditorwindow.h"
#include "rkhtmlwindow.h"
#include "rkworkplaceview.h"
#include "rktoolwindowbar.h"
#include "../core/robject.h"
#include "../core/rcontainerobject.h"
#include "../core/robjectlist.h"
#include "../dataeditor/rkeditor.h"
#include "../dataeditor/rkeditordataframe.h"
#include "../robjectviewer.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../rbackend/rinterface.h"
#include "../windows/rkwindowcatcher.h"
#include "../rbackend/rcommand.h"
#include "../misc/rkcommonfunctions.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

// static
RKWorkplace *RKWorkplace::main_workplace = 0;

RKWorkplace::RKWorkplace (QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);
	RK_ASSERT (main_workplace == 0);

	main_workplace = this;

	/* Splitter setup contains heavy copying from Kate's katemdi! */
	KVBox *vbox = new KVBox (this);

	tool_window_bars[KMultiTabBar::Top] = new RKToolWindowBar (KMultiTabBar::Top, vbox);
	vert_splitter = new QSplitter (Qt::Vertical, vbox);
	vert_splitter->setOpaqueResize (KGlobalSettings::opaqueResize ());
	tool_window_bars[KMultiTabBar::Top]->setSplitter (vert_splitter);

	KHBox *hbox = new KHBox (vert_splitter);
	vert_splitter->setCollapsible (vert_splitter->indexOf (hbox), false);
	vert_splitter->setStretchFactor (vert_splitter->indexOf (hbox), 1);

	tool_window_bars[KMultiTabBar::Left] = new RKToolWindowBar (KMultiTabBar::Left, hbox);
	horiz_splitter = new QSplitter (Qt::Horizontal, hbox);
	horiz_splitter->setOpaqueResize (KGlobalSettings::opaqueResize ());
	tool_window_bars[KMultiTabBar::Left]->setSplitter (horiz_splitter);

	wview = new RKWorkplaceView (horiz_splitter);
	horiz_splitter->setCollapsible (horiz_splitter->indexOf (wview), false);
	horiz_splitter->setStretchFactor(horiz_splitter->indexOf (wview), 1);

	tool_window_bars[KMultiTabBar::Bottom] = new RKToolWindowBar (KMultiTabBar::Bottom, vbox);
	tool_window_bars[KMultiTabBar::Bottom]->setSplitter (vert_splitter);

	tool_window_bars[KMultiTabBar::Right] = new RKToolWindowBar (KMultiTabBar::Right, hbox);
	tool_window_bars[KMultiTabBar::Right]->setSplitter (horiz_splitter);

	KConfigGroup toolbar_config = KGlobal::config ()->group ("ToolwindowBars");
	tool_window_bars[KMultiTabBar::Top]->restoreSize (toolbar_config);
	tool_window_bars[KMultiTabBar::Left]->restoreSize (toolbar_config);
	tool_window_bars[KMultiTabBar::Bottom]->restoreSize (toolbar_config);
	tool_window_bars[KMultiTabBar::Right]->restoreSize (toolbar_config);

	// now add it all to this widget
	QVBoxLayout *box = new QVBoxLayout (this);
	box->setContentsMargins (0, 0, 0, 0);
	box->addWidget (vbox);

	history = new RKMDIWindowHistory (this);

	connect (RKWardMainWindow::getMain (), SIGNAL (aboutToQuitRKWard()), this, SLOT (saveSettings()));
}

RKWorkplace::~RKWorkplace () {
	RK_TRACE (APP);

//	closeAll ();	// not needed, as the windows will autodelete themselves using QObject mechanism. Of course, closeAll () should be called *before* quitting.
}

void RKWorkplace::saveSettings () {
	RK_TRACE (APP);

	KConfigGroup toolbar_config = KGlobal::config ()->group ("ToolwindowBars");
	tool_window_bars[KMultiTabBar::Top]->saveSize (toolbar_config);
	tool_window_bars[KMultiTabBar::Left]->saveSize (toolbar_config);
	tool_window_bars[KMultiTabBar::Bottom]->saveSize (toolbar_config);
	tool_window_bars[KMultiTabBar::Right]->saveSize (toolbar_config);
}

void RKWorkplace::initActions (KActionCollection *ac, const char *prev_id, const char *next_id, const char *left_id, const char *right_id) {
	RK_TRACE (APP);

	history->initActions (ac, prev_id, next_id);
	wview->initActions (ac, left_id, right_id);
}

void RKWorkplace::attachWindow (RKMDIWindow *window) {
	RK_TRACE (APP);
	RK_ASSERT (windows.contains (window));		// This should not happen for now.

	if (!window->isAttached ()) {
		QWidget *old_parent = window->parentWidget ();
		window->prepareToBeAttached ();
		if (old_parent && qobject_cast<DetachedWindowContainer*> (old_parent)) {
			old_parent->deleteLater ();
		}
	}

	// all the rest is done, even if the window was previously "Attached", as this may also mean it was freshly created
	window->state = RKMDIWindow::Attached;
	if (window->isToolWindow ()) {
		window->tool_window_bar->reclaimDetached (window);
	} else {
		view ()->addWindow (window);
		view ()->topLevelWidget ()->raise ();
		view ()->topLevelWidget ()->activateWindow ();
	}

	RK_ASSERT (window->getPart ());
	RKWardMainWindow::getMain ()->partManager ()->addPart (window->getPart ());
}

void RKWorkplace::detachWindow (RKMDIWindow *window, bool was_attached) {
	RK_TRACE (APP);
	if (!window) return;
	RK_ASSERT (windows.contains (window));		// Can't detach a window that is not registered

	window->prepareToBeDetached ();
	window->state = RKMDIWindow::Detached;

	RK_ASSERT (window->getPart ());
	if (was_attached) {
		RKWardMainWindow::getMain ()->partManager ()->removePart (window->getPart ());
		if (!window->isToolWindow ()) view ()->removeWindow (window);
	}

	DetachedWindowContainer *detached = new DetachedWindowContainer (window);
	detached->show ();
}

void RKWorkplace::addWindow (RKMDIWindow *window, bool attached) {
	RK_TRACE (APP);

	windows.append (window);
	connect (window, SIGNAL (destroyed (QObject *)), this, SLOT (windowDestroyed (QObject *)));
	connect (window, SIGNAL (windowActivated(RKMDIWindow*)), history, SLOT (windowActivated(RKMDIWindow*)));
	if (attached) attachWindow (window);
	else detachWindow (window, false);
}

void RKWorkplace::placeInToolWindowBar (RKMDIWindow *window, KMultiTabBar::KMultiTabBarPosition position) {
	RK_TRACE (APP);

	RK_ASSERT (window->isToolWindow ());
	tool_window_bars[position]->addWidget (window);
	if (!windows.contains (window)) {	// must be new
		addWindow (window, true);
		RKWardMainWindow::getMain ()->partManager ()->addPart (window->getPart ());
	}
}

bool RKWorkplace::openAnyUrl (const KUrl &url) {
	RK_TRACE (APP);

#warning TODO support rkward:\/\/-protocol, here, too
	KMimeType::Ptr mimetype = KMimeType::findByUrl (url);
	if (mimetype->is ("text/html")) {
		openHelpWindow (url, true);
		return true;	// TODO
	}
	if (url.fileName ().toLower ().endsWith (".rdata")) {
		RKWardMainWindow::getMain ()->fileOpenAskSave (url);
		return true;	// TODO
	}
	if (mimetype->name ().startsWith ("text")) {
		return (openScriptEditor (url, QString (), false));
	}

	if (KMessageBox::questionYesNo (this, i18n ("The url you are trying to open ('%1') is not a local file or the filetype is not supported by RKWard. Do you want to open the url in the default application?", url.prettyUrl ()), i18n ("Open in default application?")) != KMessageBox::Yes) {
		return false;
	}
	KRun *runner = new KRun (url, topLevelWidget());		// according to KRun-documentation, KRun will self-destruct when done.
	runner->setRunExecutables (false);
	return false;
}

RKMDIWindow* RKWorkplace::openScriptEditor (const KUrl &url, const QString& encoding, bool use_r_highlighting, bool read_only, const QString &force_caption, bool delete_on_close) {
	RK_TRACE (APP);

// is this url already opened?
	if (!url.isEmpty ()) {
	  	RKWorkplaceObjectList script_windows = getObjectList (RKMDIWindow::CommandEditorWindow, RKMDIWindow::AnyWindowState);
		for (RKWorkplaceObjectList::const_iterator it = script_windows.constBegin (); it != script_windows.constEnd (); ++it) {
			  KUrl ourl = static_cast<RKCommandEditorWindow *> (*it)->url ();
			  if (url == ourl) {
				  (*it)->activate ();
				  return (*it);
			  }
		}
	}

	RKCommandEditorWindow *editor = new RKCommandEditorWindow (view (), use_r_highlighting);

	if (!url.isEmpty ()) {
		if (!editor->openURL (url, encoding, use_r_highlighting, read_only, delete_on_close)) {
			delete editor;
			KMessageBox::messageBox (view (), KMessageBox::Error, i18n ("Unable to open \"%1\"", url.prettyUrl ()), i18n ("Could not open command file"));
			return 0;
		}
	}

	if (!force_caption.isEmpty ()) editor->setCaption (force_caption);
	addWindow (editor);
	return (editor);
}

RKMDIWindow* RKWorkplace::openHelpWindow (const KUrl &url, bool only_once) {
	RK_TRACE (APP);

	if (url.isEmpty ()) {
		RK_ASSERT (false);
		return 0;
	}

	if (only_once) {
		RKWorkplaceObjectList help_windows = getObjectList (RKMDIWindow::HelpWindow, RKMDIWindow::AnyWindowState);
		for (RKWorkplaceObjectList::const_iterator it = help_windows.constBegin (); it != help_windows.constEnd (); ++it) {
			if (static_cast<RKHTMLWindow *> (*it)->url ().equals (url, KUrl::CompareWithoutTrailingSlash | KUrl::CompareWithoutFragment)) {
				(*it)->activate ();
				return (*it);
			}
		}
	}

	RKHTMLWindow *hw = new RKHTMLWindow (view (), RKHTMLWindow::HTMLHelpWindow);
	hw->openURL (url);
	addWindow (hw);
	return (hw);
}

RKMDIWindow* RKWorkplace::openOutputWindow (const KUrl &url) {
	RK_TRACE (APP);

	RKHTMLWindow *w = RKOutputWindowManager::self ()->getCurrentOutputWindow ();
	if (!windows.contains (w)) {
		addWindow (w);
	} else {
		w->activate ();
	}
	return (w);
}

void RKWorkplace::newX11Window (WId window_to_embed, int device_number) {
	RK_TRACE (APP);

	RKCaughtX11Window *window = new RKCaughtX11Window (window_to_embed, device_number);
	window->state = RKMDIWindow::Detached;
	addWindow (window, false);
}

void RKWorkplace::newObjectViewer (RObject *object) {
	RK_TRACE (APP);
	RK_ASSERT (object);

	RKWorkplaceObjectList object_windows = getObjectList (RKMDIWindow::ObjectWindow, RKMDIWindow::AnyWindowState);
	for (RKWorkplaceObjectList::const_iterator it = object_windows.constBegin (); it != object_windows.constEnd (); ++it) {
		if (static_cast<RObjectViewer *> (*it)->object () == object) {
			(*it)->activate ();
			return;
		}
	}

	RObjectViewer *ov = new RObjectViewer (view (), object);
	addWindow (ov);
}

bool RKWorkplace::canEditObject (RObject *object) {
	RK_TRACE (APP);
	
	if (object->isDataFrame ()) {
		return true;
	} else if (object->isVariable () && object->parentObject ()->isDataFrame ()) {
		return true;
	}
	return false;
}

RKEditor* RKWorkplace::editNewDataFrame (const QString &name) {
	RK_TRACE (APP);

	RKEditorDataFrame* ed = new RKEditorDataFrame (name, 0);
	addWindow (ed);
	ed->activate ();

	return ed;
}

RKEditor *RKWorkplace::editObject (RObject *object) {
	RK_TRACE (APP);
	RK_ASSERT (object);

	RObject *iobj = object;
	RKEditor *ed = 0;
	RKEditor *existing_editor = object->editor ();
	if (!existing_editor) {
		if (!iobj->isDataFrame ()) {
			if (iobj->isVariable () && iobj->parentObject ()->isDataFrame ()) {
				iobj = iobj->parentObject ();
			} else {
				return 0;
			}
		}

		unsigned long size = 1;
		foreach (int dim, iobj->getDimensions ()) {
			size *= dim;
		}
		if ((RKSettingsModuleGeneral::warnLargeObjectThreshold () != 0) && (size > RKSettingsModuleGeneral::warnLargeObjectThreshold ())) {
			if (KMessageBox::warningContinueCancel (view (), i18n ("You are about to edit object \"%1\", which is very large (%2 fields). RKWard is not optimized to handle very large objects in the built in data editor. This will use a lot of memory, and - depending on your system - might be very slow. For large objects it is generally recommended to edit using command line means or to split into smaller chunks before editing. On the other hand, if you have enough memory, or the data is simple enough (numeric data is easier to handle, than factor), editing may not be a problem at all. You can configure this warning (or turn it off entirely) under Settings->Configure RKWard->General.\nReally edit object?", iobj->getFullName (), size), i18n ("About to edit very large object")) != KMessageBox::Continue) {
				return 0;
			}
		}

		ed = new RKEditorDataFrame (static_cast<RContainerObject*> (iobj), 0);
		addWindow (ed);
	} else {
		ed = existing_editor;
	}

	ed->activate ();
	return ed;
}

void RKWorkplace::flushAllData () {
	RK_TRACE (APP);

	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if ((*it)->type == RKMDIWindow::DataEditorWindow) {
			static_cast<RKEditor *> (*it)->flushChanges ();
		}
	}
}

void RKWorkplace::closeWindow (RKMDIWindow *window) {
	RK_TRACE (APP);
	RK_ASSERT (windows.contains (window));

	bool tool_window = window->isToolWindow ();
	window->close (true);		// all the rest should happen in windowDestroyed ()
	
	if (tool_window) windowRemoved ();	// for regular windows, this happens in windowDestroyed(), already
}

void RKWorkplace::closeActiveWindow () {
	RK_TRACE (APP);

	RKMDIWindow *w = activeWindow (RKMDIWindow::Attached);
	if (w) closeWindow (w);
}

RKWorkplace::RKWorkplaceObjectList RKWorkplace::getObjectList (int type, int state) {
	RK_TRACE (APP);

	RKWorkplaceObjectList ret;
	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if (((*it)->type & type) && ((*it)->state & state)) {
			ret.append ((*it));
		}
	}
	return ret;
}

void RKWorkplace::closeAll (int type, int state) {
	RK_TRACE (APP);

	RKWorkplaceObjectList list_to_close = getObjectList (type, state);
	for (RKWorkplaceObjectList::const_iterator it = list_to_close.constBegin (); it != list_to_close.constEnd (); ++it) {
		closeWindow (*it);
	}
}

void RKWorkplace::windowDestroyed (QObject *object) {
	RK_TRACE (APP);
	RKMDIWindow *window = static_cast<RKMDIWindow *> (object);

	// remove from history first (otherwise, we might find it there, when trying to activate a new window)
	history->windowDestroyed (window);

	// WARNING: the window is dead. Don't call any functions on it.

	RK_ASSERT (windows.contains (window));
	windows.removeAll (window);		// do this first! view()->removeWindow will call activePage() indirectly from setCaption, causing us to iterate over all known windows!
	if (view ()->hasWindow (window)) view ()->removeWindow (window, true);

	windowRemoved ();
}

void RKWorkplace::windowRemoved () {
	RK_TRACE (APP);

	if (activeWindow (RKMDIWindow::AnyWindowState) != 0) return;	// something already active

	// try to activate an attached document window, first
	RKMDIWindow *window = view ()->activePage ();
	if (window) {
		window->activate (true);
		return;
	}

	// some document window in the history? Try that.
	if (history->haveNext ()) {
		history->next ();
		return;
	} else if (history->havePrev ()) {
		history->prev ();
		return;
	}

	// now try to activate an attached (tool) window, if one is visible
	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if ((*it)->isAttached ()) {
			if ((*it)->isVisible ()) {
				(*it)->activate (true);
				return;
			}
		}
	}

	// nothing, yet? Try *any* visible window
	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if ((*it)->isVisible ()) {
			(*it)->activate (true);
			return;
		}
	}

	// give up
}

RKMDIWindow *RKWorkplace::activeWindow (RKMDIWindow::State state) {
	RK_TRACE (APP);

	RKMDIWindow *ret = 0;
	for (RKWorkplaceObjectList::const_iterator it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if (!(state & ((*it)->state))) continue;

		if ((*it)->isActive ()) {
			ret = *it;
			break;
		}
	}

	return (ret);
}

QStringList RKWorkplace::makeWorkplaceDescription () {
	RK_TRACE (APP);

	QStringList workplace_description;

	// first, save the base directory of the workplace. This allows us to cope better with moved workspaces while restoring.
	KUrl base_url = RObjectList::getObjectList ()->getWorkspaceURL ();
	base_url.setPath (base_url.directory ());
	if (base_url.isLocalFile () && base_url.hasPath ()) workplace_description.append ("base::::" + base_url.url ());

	// window order in the workplace view may have changed with respect to our list. Thus we first generate a properly sorted list
	RKWorkplaceObjectList list = getObjectList (RKMDIWindow::DocumentWindow, RKMDIWindow::Detached);
	for (int i=0; i < wview->count (); ++i) {
		list.append (static_cast<RKMDIWindow*> (wview->widget (i)));
	}
	foreach (RKMDIWindow *win, list) {
		QString type, specification;
		QStringList params;
		if (win->isType (RKMDIWindow::DataEditorWindow)) {
			type = "data";
			specification = static_cast<RKEditor*> (win)->getObject ()->getFullName ();
		} else if (win->isType (RKMDIWindow::CommandEditorWindow)) {
			type = "script";
			specification = static_cast<RKCommandEditorWindow*> (win)->url ().url ();
		} else if (win->isType (RKMDIWindow::OutputWindow)) {
			type = "output";
			specification = static_cast<RKHTMLWindow*> (win)->url ().url ();
		} else if (win->isType (RKMDIWindow::HelpWindow)) {
			type = "help";
			specification = static_cast<RKHTMLWindow*> (win)->restorableUrl ().url ();
		}
		if (!type.isEmpty ()) {
			if (!win->isAttached ()) {
				params.append (QString ("detached,") + QString::number (win->x ()) + ',' + QString::number (win->y ()) + ',' + QString::number (win->width ()) + ',' + QString::number (win->height ()));
			}
			workplace_description.append (type + "::" + params.join (":") + "::" + specification);
		}
	}
	return workplace_description;
}

void RKWorkplace::saveWorkplace (RCommandChain *chain) {
	RK_TRACE (APP);
	if (RKSettingsModuleGeneral::workplaceSaveMode () != RKSettingsModuleGeneral::SaveWorkplaceWithWorkspace) return;

	RKGlobals::rInterface ()->issueCommand ("rk.save.workplace()", RCommand::App, i18n ("Save Workplace layout"), 0, 0, chain);
}

void RKWorkplace::restoreWorkplace (RCommandChain *chain) {
	RK_TRACE (APP);
	if (RKSettingsModuleGeneral::workplaceSaveMode () != RKSettingsModuleGeneral::SaveWorkplaceWithWorkspace) return;

	RKGlobals::rInterface ()->issueCommand ("rk.restore.workplace()", RCommand::App, i18n ("Restore Workplace layout"), 0, 0, chain);
}

KUrl checkAdjustRestoredUrl (const QString &_url, const QString old_base) {
	KUrl url (_url);

	if (old_base.isEmpty ()) return (url);
	KUrl new_base_url = RObjectList::getObjectList ()->getWorkspaceURL ();
	new_base_url.setPath (new_base_url.directory ());
	if (new_base_url.isEmpty ()) return (url);
	KUrl old_base_url (old_base);
	if (old_base_url == new_base_url) return (url);

	// TODO: Should we also care about non-local files? In theory: yes, but stat'ing remote files for existence can take a long time.
	if (!(old_base_url.isLocalFile () && new_base_url.isLocalFile () && url.isLocalFile ())) return (url);

	// if the file exists, unadjusted, return it.
	if (QFileInfo (url.toLocalFile ()).exists ()) return (url);

	// check whether a file exists for the adjusted url
	KUrl relative = KUrl::fromLocalFile (new_base_url.path () + '/' + KUrl::relativePath (old_base_url.path (), url.path ()));
	relative.cleanPath ();
	if (QFileInfo (relative.toLocalFile ()).exists ()) return (relative);
	return (url);
}

void RKWorkplace::restoreWorkplace (const QStringList &description) {
	RK_TRACE (APP);

	QString base;
	for (int i = 0; i < description.size (); ++i) {
		// Item format for rkward <= 0.5.4: "type:specification"
		// Item format for rkward <= 0.5.5: "type::[optional_params1[:optional_params2[:...]]]::specification"
		int typeend = description[i].indexOf (':');
		if ((typeend < 0) || (typeend >= (description[i].size () - 1))) {
			RK_ASSERT (false);
			continue;
		}
		QString type, specification;
		QStringList params;
		type = description[i].left (typeend);
		if (description[i].at (typeend + 1) == ':') {	// rkward 0.5.5 or later
			int specstart = description[i].indexOf ("::", typeend + 2);
			if (specstart < typeend) {
				RK_ASSERT (false);
				continue;
			}
			params = description[i].mid (typeend + 2, specstart - typeend).split (':');
			specification = description[i].mid (specstart + 2);
		} else {
			specification = description[i].mid (typeend +1 );
		}

		RKMDIWindow *win = 0;
		if (type == "base") {
			RK_ASSERT (i == 0);
			base = specification;
		} else if (type == "data") {
			RObject *object = RObjectList::getObjectList ()->findObject (specification);
			if (object) win = editObject (object);
		} else if (type == "script") {
			win = openScriptEditor (checkAdjustRestoredUrl (specification, base));
		} else if (type == "output") {
			win = openOutputWindow (checkAdjustRestoredUrl (specification, base));
		} else if (type == "help") {
			win = openHelpWindow (checkAdjustRestoredUrl (specification, base), true);
		} else {
			RK_ASSERT (false);
		}

		// apply generic window parameters
		if (win) {
			for (int p = 0; p < params.size (); ++p) {
				if (params[p].startsWith ("detached")) {
					QStringList geom = params[p].split (',');
					win->hide ();
					win->setGeometry (geom.value (1).toInt (), geom.value (2).toInt (), geom.value (3).toInt (), geom.value (4).toInt ());
					detachWindow (win);
				} else {
					RK_ASSERT (false);
				}
			}
		}
	}
}

///////////////////////// END RKWorkplace ////////////////////////////
///////////////////// BEGIN RKMDIWindowHistory ///////////////////////

RKMDIWindowHistory::RKMDIWindowHistory (QObject *parent) : QObject (parent) {
	RK_TRACE (APP);

	current = 0;
	prev_action = next_action = 0;
}

RKMDIWindowHistory::~RKMDIWindowHistory () {
	RK_TRACE (APP);

	RK_DO (qDebug ("Remaining windows in history: forward: %d, backward: %d", forward_list.count (), back_list.count ()), APP, DL_DEBUG);
}

void RKMDIWindowHistory::initActions (KActionCollection *ac, const char *prev_id, const char *next_id) {
	RK_TRACE (APP);

	prev_action = (KAction*) ac->addAction (prev_id, this, SLOT (prev()));
	prev_action->setText (i18n ("Previous Window"));
	prev_action->setIcon (QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/window_back.png"));
	prev_action->setShortcut (KShortcut (Qt::AltModifier + Qt::Key_Less, Qt::AltModifier + Qt::Key_Comma));

	next_action = (KAction*) ac->addAction (next_id, this, SLOT (next()));
	next_action->setText (i18n ("Next Window"));
	next_action->setIcon (QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/window_forward.png"));
	next_action->setShortcut (KShortcut (Qt::AltModifier + Qt::Key_Greater, Qt::AltModifier + Qt::Key_Period));

	updateActions ();
}

void RKMDIWindowHistory::windowActivated (RKMDIWindow *window) {
	RK_TRACE (APP);

	if (!window) return;
	if (window == current) return;
	if (window->isToolWindow ()) return;		// exclude tool windows for now. Make configurable?

	// update lists
	back_list.removeAll (window);		// remove dupes
	forward_list.clear ();
	if (current) back_list.append (current);
	current = window;

	updateActions ();
}

void RKMDIWindowHistory::next () {
	RK_TRACE (APP);

	if (!haveNext ()) return;
	if (current) back_list.append (current);
	current = forward_list.first ();
	forward_list.pop_front ();

	updateActions ();

	RK_ASSERT (current);
	current->activate ();
}

void RKMDIWindowHistory::prev () {
	RK_TRACE (APP);

	if (!havePrev ()) return;
	if (current) forward_list.push_front (current);
	current = back_list.last ();
	back_list.pop_back ();

	updateActions ();

	RK_ASSERT (current);
	current->activate ();
}

bool RKMDIWindowHistory::haveNext () {
	RK_TRACE (APP);

	return (!forward_list.isEmpty ());
}

bool RKMDIWindowHistory::havePrev () {
	RK_TRACE (APP);

	return (!back_list.isEmpty ());
}

void RKMDIWindowHistory::updateActions () {
	RK_TRACE (APP);

	if (next_action) {
		next_action->setEnabled (haveNext ());
	}

	if (prev_action) {
		prev_action->setEnabled (havePrev ());
	}
}

void RKMDIWindowHistory::windowDestroyed (QObject *window) {
	RK_TRACE (APP);

	back_list.removeAll (static_cast<RKMDIWindow *> (window));
	forward_list.removeAll (static_cast<RKMDIWindow *> (window));
	if (current == window) current = 0;
	updateActions ();
}

#include "rkworkplace.moc"
