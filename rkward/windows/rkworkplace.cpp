/*
rkworkplace - This file is part of RKWard (https://rkward.kde.org). Created: Thu Sep 21 2006
SPDX-FileCopyrightText: 2006-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkworkplace.h"

#include <KParts/PartManager>
#include <KMessageBox>
#include <KLocalizedString>
#include <KActionCollection>
#include <KSharedConfig>
#include <KMessageWidget>
#include <KIO/OpenUrlJob>

#include <QFileInfo>
#include <QCryptographicHash>
#include <QKeyEvent>
#include <QDir>
#include <QApplication>
#include <QMimeDatabase>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QDesktopServices>

#include "detachedwindowcontainer.h"
#include "rkcommandeditorwindow.h"
#include "rkhtmlwindow.h"
#include "rkpdfwindow.h"
#include "rkworkplaceview.h"
#include "rktoolwindowbar.h"
#include "rktoolwindowlist.h"
#include "../core/robject.h"
#include "../core/rcontainerobject.h"
#include "../core/robjectlist.h"
#include "../dataeditor/rkeditor.h"
#include "../dataeditor/rkeditordataframe.h"
#include "../dialogs/rksavemodifieddialog.h"
#include "../robjectviewer.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettingsmodulecommandeditor.h"
#include "../rbackend/rkrinterface.h"
#include "../windows/rkwindowcatcher.h"
#include "../rbackend/rcommand.h"
#include "../misc/rkoutputdirectory.h"
#include "../misc/rkxmlguipreviewarea.h"
#include "../rkward.h"

#include "../debug.h"

// static
RKWorkplace *RKWorkplace::main_workplace = nullptr;

#include <QLabel> // remove ME

RKWorkplace::RKWorkplace (QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);
	RK_ASSERT (main_workplace == nullptr);

	main_workplace = this;
	QDesktopServices::setUrlHandler("rkward", this, "openRKWardUrl");
	_workspace_config = nullptr;
	window_placement_override = RKMDIWindow::AnyWindowState;

	// message area
	message_area = new QWidget (this);
	QVBoxLayout *message_layout = new QVBoxLayout (message_area);
	message_layout->setContentsMargins (0, 0, 0, 0);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	vbox->setSpacing (0);
	vbox->addWidget (message_area);

	tool_window_bars[RKToolWindowList::Top] = new RKToolWindowBar (KMultiTabBar::Top, this);
	vert_splitter = new QSplitter (Qt::Vertical, this);
	tool_window_bars[RKToolWindowList::Top]->setSplitter (vert_splitter);

	QWidget *harea = new QWidget (vert_splitter);
	QHBoxLayout *hbox = new QHBoxLayout (harea);
	hbox->setContentsMargins (0, 0, 0, 0);
	hbox->setSpacing (0);
	vert_splitter->setCollapsible (vert_splitter->indexOf (harea), false);
	vert_splitter->setStretchFactor (vert_splitter->indexOf (harea), 1);

	tool_window_bars[RKToolWindowList::Left] = new RKToolWindowBar (KMultiTabBar::Left, harea);
	horiz_splitter = new QSplitter (Qt::Horizontal, harea);
	tool_window_bars[RKToolWindowList::Left]->setSplitter (horiz_splitter);

	wview = new RKWorkplaceView (horiz_splitter);
	horiz_splitter->setCollapsible (horiz_splitter->indexOf (wview), false);
	horiz_splitter->setStretchFactor(horiz_splitter->indexOf (wview), 1);

	tool_window_bars[RKToolWindowList::Right] = new RKToolWindowBar (KMultiTabBar::Right, harea);
	tool_window_bars[RKToolWindowList::Right]->setSplitter (horiz_splitter);
	hbox->addWidget (tool_window_bars[RKToolWindowList::Left]);
	hbox->addWidget (horiz_splitter);
	hbox->addWidget (tool_window_bars[RKToolWindowList::Right]);

	tool_window_bars[RKToolWindowList::Bottom] = new RKToolWindowBar (KMultiTabBar::Bottom, this);
	tool_window_bars[RKToolWindowList::Bottom]->setSplitter (vert_splitter);
	vbox->addWidget (tool_window_bars[RKToolWindowList::Top]);
	vbox->addWidget (vert_splitter);
	auto bottom_box = new QHBoxLayout();
	vbox->addLayout(bottom_box);
	bottom_box->addWidget (tool_window_bars[RKToolWindowList::Bottom]);
	status_bar = new QStatusBar();
	bottom_box->addWidget(status_bar, 0, Qt::AlignRight);

	KConfigGroup toolbar_config = KSharedConfig::openConfig ()->group ("ToolwindowBars");
	for (int i = 0; i < TOOL_WINDOW_BAR_COUNT; ++i) tool_window_bars[i]->restoreSize (toolbar_config);

	history = new RKMDIWindowHistory (this);

	connect (RKWardMainWindow::getMain (), &RKWardMainWindow::aboutToQuitRKWard, this, &RKWorkplace::saveSettings);
}

RKWorkplace::~RKWorkplace () {
	RK_TRACE (APP);

	delete _workspace_config;
//	closeAll ();	// not needed, as the windows will autodelete themselves using QObject mechanism. Of course, closeAll () should be called *before* quitting.
	for (int i = 0; i < windows.size (); ++i) {
		disconnect (windows[i], nullptr, this, nullptr);
	}
}

void RKWorkplace::addMessageWidget (KMessageWidget* message) {
	RK_TRACE (APP);

	message_area->layout ()->addWidget (message);
	topLevelWidget ()->show ();
	topLevelWidget ()->raise ();
}

QString workspaceConfigFileName (const QUrl &url) {
	QString base_name = QString (QCryptographicHash::hash (url.toDisplayString ().toUtf8 (), QCryptographicHash::Md5).toHex());
	QDir dir (QStandardPaths::writableLocation (QStandardPaths::GenericDataLocation));
	dir.mkpath ("rkward");
	return (dir.absoluteFilePath ("rkward/workspace_config_" + base_name));
}

KConfigBase *RKWorkplace::workspaceConfig () {
	if (!_workspace_config) {
		RK_TRACE (APP);
		_workspace_config = new KConfig (workspaceConfigFileName (workspaceURL ()));
	}
	return _workspace_config;
}

QString RKWorkplace::portableUrl (const QUrl &url) {
	QUrl ret = url;
	if (url.scheme () == workspaceURL ().scheme () && url.authority () == workspaceURL ().authority ()) {
		QString relative = QDir (workspaceURL ().path ()).relativeFilePath (url.path ());
		ret = QUrl (relative);
	}
	// QUrl::toDisplayString () used here, for the side effect of stripping credentials
	return QUrl (ret).adjusted (QUrl::NormalizePathSegments).toDisplayString ();
}

void RKWorkplace::setWorkspaceURL (const QUrl &url, bool keep_config) {
	RK_TRACE (APP);

	if (url != current_url) {
		current_url = url;
		if (keep_config && _workspace_config) {
			KConfig * _new_config = _workspace_config->copyTo (workspaceConfigFileName (workspaceURL ()));
			delete _workspace_config;
			_workspace_config = _new_config;
		} else {
			delete _workspace_config;
			_workspace_config = nullptr;
		}
		Q_EMIT workspaceUrlChanged(url);
	}
}

void RKWorkplace::saveSettings () {
	RK_TRACE (APP);

	KConfigGroup toolbar_config = KSharedConfig::openConfig ()->group ("ToolwindowBars");
	for (int i = 0; i < TOOL_WINDOW_BAR_COUNT; ++i) tool_window_bars[i]->saveSize (toolbar_config);
}

void RKWorkplace::initActions (KActionCollection *ac) {
	RK_TRACE (APP);

	wview->initActions (ac);
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
		if (!window->tool_window_bar) placeInToolWindowBar (window, RKToolWindowList::Bottom);
		else window->tool_window_bar->reclaimDetached (window);
	} else {
		view ()->addWindow (window);
		view ()->topLevelWidget ()->raise ();
		view ()->topLevelWidget ()->activateWindow ();
	}

	RK_ASSERT (window->getPart ());
	RKWardMainWindow::getMain ()->partManager ()->addPart (window->getPart (), !window->isToolWindow ());
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

	DetachedWindowContainer *detached = new DetachedWindowContainer (window, was_attached);
	detached->show ();
	if (!was_attached) window->activate ();
}

void RKWorkplace::setWindowNotManaged(RKMDIWindow* window) {
	RK_TRACE (APP);
	window->state = RKMDIWindow::Detached;
}

void RKWorkplace::addWindow (RKMDIWindow *window, bool attached) {
	RK_TRACE (APP);

	// first handle placement overrides
	if (window_placement_override == RKMDIWindow::Attached) {
		if (!attached) window->state = RKMDIWindow::Detached;   // Ok, yeah. BAD style. But windows that would go to detached by default would not prepareToBeAttached(), without this.
		                                                        // TODO: Create third state: NotManaged
		attached = true;
	} else if (window_placement_override == RKMDIWindow::Detached) {
		attached = false;
	}

	// style override. Windows may or may not handle this
	if (!window_style_override.isEmpty ()) window->setWindowStyleHint (window_style_override);

	// next handle name overrides, if any
	if (!window_name_override.isEmpty ()) {
		int pos = -1;
		for (int i = 0; i < named_windows.size (); ++i) {
			if (named_windows[i].id == window_name_override) {
				pos = i;
				break;
			}
		}
		if (pos < 0) {   // not yet known: implicit registration -> create corresponding named_window_spec on the fly.
			registerNamedWindow(window_name_override, nullptr, attached ? RKWardMainWindow::getMain() : nullptr);
			pos = named_windows.size () - 1;
		}

		NamedWindow &nw = named_windows[pos];
		if (nw.window != window) {
			if (nw.window) {  // kill existing window (going to be replaced)
				// TODO: this is not really elegant, yet, as it will change tab-placement (for attached windows), and discard / recreate container (for detached windows)
				disconnect (nw.window, &QObject::destroyed, this, &RKWorkplace::namedWindowDestroyed);
				nw.window->close(RKMDIWindow::NoAskSaveModified);
			}
			nw.window = window;
			connect (nw.window, &QObject::destroyed, this, &RKWorkplace::namedWindowDestroyed);
		}
		named_windows[pos] = nw;

		// add window in the correct area
		if (nw.parent == RKWardMainWindow::getMain ()) attached = true;
		else if (nw.parent == nullptr) attached = false;
		else { // custom parent
			RKXMLGUIPreviewArea* area = qobject_cast<RKXMLGUIPreviewArea*>(nw.parent);
			if (!area) {
				RK_ASSERT(area);
				return;
			}
			window->prepareToBeAttached();
			area->setWindow(window);
			// TODO: do this is somewhat inconsistent. But such windows are not attached to the main workplace view, which makes them rather behave detached.
			window->state = RKMDIWindow::Detached;
			// NOTE: The window is _not_ added to the window list/window history in this case.
			return;
		}
	}

	windows.append (window);
	connect (window, &QObject::destroyed, this, &RKWorkplace::removeWindow);
	connect (window, &RKMDIWindow::windowActivated, history, &RKMDIWindowHistory::windowActivated);
	if (window->isToolWindow () && !window->tool_window_bar) return;

	if (attached) attachWindow (window);
	else detachWindow (window, false);
}

void RKWorkplace::placeToolWindows() {
	RK_TRACE (APP);

	const auto windows = RKToolWindowList::registeredToolWindows ();
	for (const RKToolWindowList::ToolWindowRepresentation& rep : windows) {
		placeInToolWindowBar (rep.window, rep.default_placement);
	}
}

void RKWorkplace::placeInToolWindowBar (RKMDIWindow *window, int position) {
	RK_TRACE (APP);

	RK_ASSERT (window->isToolWindow ());
	bool needs_registration = (!window->tool_window_bar && (position != RKToolWindowList::Nowhere));
	if ((position < 0) || (position >= TOOL_WINDOW_BAR_COUNT)) {
		RK_ASSERT (position == RKToolWindowList::Nowhere);	// should never happen...
		position = RKToolWindowList::Nowhere;		// ... but let's set this explicitly, in case of a broken workplace representation
	}
	if (position == RKToolWindowList::Nowhere) {
		if (window->tool_window_bar) window->tool_window_bar->removeWidget (window);
	} else {
		tool_window_bars[position]->addWidget (window);
	}

	if (!windows.contains (window)) addWindow (window, true);	// first time we see this window
	else if (needs_registration) attachWindow (window);
}

void RKWorkplace::setWindowPlacementOverrides(const QString& placement, const QString& name, const QString& style) {
	RK_TRACE (APP);

	if (placement == "attached") window_placement_override = RKMDIWindow::Attached;
	else if (placement == "detached") window_placement_override = RKMDIWindow::Detached;
	else {
		RK_ASSERT (placement.isEmpty ());
		window_placement_override = RKMDIWindow::AnyWindowState;
	}
	window_name_override = name;
	window_style_override = style;
}

void RKWorkplace::registerNamedWindow (const QString& id, QObject* owner, QWidget* parent, RKMDIWindow* window) {
	RK_TRACE (APP);

	NamedWindow nw;
	nw.id = id;
	nw.owner = owner;
	nw.parent = parent;
	nw.window = window;

	for (int i = 0; i < named_windows.size (); ++i) {
		RK_ASSERT (named_windows[i].id != id);
	}

	named_windows.append (nw);
	if (owner) connect (owner, &QObject::destroyed, this, &RKWorkplace::namedWindowOwnerDestroyed);
	if (window) connect (window, &QObject::destroyed, this, &RKWorkplace::namedWindowOwnerDestroyed);
}

void RKWorkplace::namedWindowDestroyed (QObject* window) {
	RK_TRACE (APP);

	for (int i = 0; i < named_windows.size (); ++i) {
		if (named_windows[i].window == window) {
			if (!named_windows[i].owner) {
				named_windows.removeAt (i);
				return;
			} else {
				named_windows[i].window = nullptr;
			}
		}
	}
}

void RKWorkplace::namedWindowOwnerDestroyed (QObject* owner) {
	RK_TRACE (APP);

	for (int i = 0; i < named_windows.size (); ++i) {
		if (named_windows[i].owner == owner) {
			if (named_windows[i].window) {
				named_windows[i].window->deleteLater ();
			}
			named_windows.removeAt (i);
			return;
		}
	}
}

void RKWorkplace::openRKWardUrl(const QUrl &url) {
	RK_TRACE(APP);
	RK_ASSERT(url.scheme() == "rkward");
	RKHTMLWindow::handleRKWardURL(url);
}

bool RKWorkplace::openAnyUrl (const QUrl &url, const QString &known_mimetype, bool force_external) {
	RK_TRACE (APP);

	if (url.scheme () == "rkward") {
		if (RKHTMLWindow::handleRKWardURL (url)) return true;
	}
	QMimeDatabase mdb;
	QMimeType mimetype;
	if (!known_mimetype.isEmpty ()) mimetype = mdb.mimeTypeForName (known_mimetype);
	else mimetype = mdb.mimeTypeForUrl (url);

	if (!force_external) {
	// NOTE: Currently a known mimetype implies that the URL is local or served from the local machine.
	// Thus, external web pages are *not* opened, here. Which is the behavior we want, although the implementation is ugly
		if (mimetype.inherits("text/html") || url.scheme().toLower().startsWith(QLatin1String("help"))) {
			openHelpWindow (url, true);
			return true;	// TODO
		} else if (mimetype.inherits("application/pdf")) {
			openPDFWindow(url);
			return true;
		}
		QString lname = url.fileName().toLower();
		if (lname.endsWith(QLatin1String(".rdata")) || lname.endsWith(QLatin1String(".rda"))) {
			RKWardMainWindow::getMain()->askOpenWorkspace(url);
			return true;	// TODO
		}
		if (lname.endsWith(".rko")) {
			auto win = openOutputWindow(url, false);
			return win != nullptr;
		}
		if (mimetype.inherits ("text/plain")) {
			return (openScriptEditor (url, QString ()));
		}
		RK_DEBUG (APP, DL_INFO, "Don't know how to handle mimetype %s.", qPrintable (mimetype.name ()));
	}

	if (KMessageBox::questionTwoActions (this, i18n ("The url you are trying to open ('%1') is not a local file or the filetype is not supported by RKWard. Do you want to open the url in the default application?", url.toDisplayString ()), i18n ("Open in default application?"), KStandardGuiItem::open(), KStandardGuiItem::cancel()) != KMessageBox::PrimaryAction) {
		return false;
	}
	auto openUrlJob = new KIO::OpenUrlJob(url);
	openUrlJob->start();
	return false;
}

RKMDIWindow* RKWorkplace::openScriptEditor (const QUrl &url, const QString& encoding, int flags, const QString &force_caption) {
	RK_TRACE (APP);

// is this url already opened?
	if (!url.isEmpty ()) {
		RKWorkplaceObjectList script_windows = getObjectList (RKMDIWindow::CommandEditorWindow, RKMDIWindow::AnyWindowState);
		for (int i = 0; i < script_windows.size (); ++i) {
			QUrl ourl = static_cast<RKCommandEditorWindow *> (script_windows[i])->url ();
			if (url == ourl) {
				if (view ()->windowInActivePane (script_windows[i])) {
					script_windows[i]->activate ();
					return (script_windows[i]);
				}
			}
		}
	}

	RKCommandEditorWindow *editor = new RKCommandEditorWindow (view (), url, encoding, flags);

	if (!force_caption.isEmpty ()) editor->setCaption (force_caption);
	addWindow (editor);
	return (editor);
}

RKMDIWindow* RKWorkplace::openPDFWindow(const QUrl &url) {
	RK_TRACE(APP);
	auto pw = getNamedWindow<RKPDFWindow>(window_name_override);
	// TODO: also match by url?
	if (pw) {
		pw->openURL(url);
	} else {
		pw = new RKPDFWindow(view());
		addWindow(pw);
		pw->openURL(url);
	}
	return pw;
}

RKMDIWindow* RKWorkplace::openHelpWindow (const QUrl &url, bool only_once) {
	RK_TRACE (APP);

	if (only_once) {
		RKWorkplaceObjectList help_windows = getObjectList (RKMDIWindow::HelpWindow, RKMDIWindow::AnyWindowState);
		for (int i = 0; i < help_windows.size (); ++i) {
			if (static_cast<RKHTMLWindow *> (help_windows[i])->url ().matches (url, QUrl::StripTrailingSlash | QUrl::NormalizePathSegments)) {
				if (view ()->windowInActivePane (help_windows[i])) {
					help_windows[i]->activate ();
					return (help_windows[i]);
				}
			}
		}
	}
	// if we're working with a window hint, try to _reuse_ the existing window, even if it did not get found, above
	auto w = getNamedWindow<RKHTMLWindow>(window_name_override);
	if (w) {
		if (!url.isEmpty()) w->openURL(url);
//		w->activate ();   // HACK: Keep preview windows from stealing focus
		return w;
	}

	RKHTMLWindow *hw = new RKHTMLWindow (view (), RKHTMLWindow::HTMLHelpWindow);
	if (!url.isEmpty()) hw->openURL(url);
	addWindow (hw);
	return (hw);
}

RKMDIWindow* RKWorkplace::openHTMLWindow(const QUrl &url) {
	RK_TRACE (APP);

	// special treatment, for now
	return openHelpWindow(url, true);
}

RKMDIWindow* RKWorkplace::openOutputWindow(const QUrl &url, bool create) {
	RK_TRACE (APP);

	if (create) RK_ASSERT(url.isEmpty());

	RKOutputDirectoryCallResult res = RKOutputDirectory::get(url.toLocalFile(), create);
	if (res.failed()) {
		KMessageBox::error(this, i18n("Failed to open output file. Error message was '%1'", res.error));
		return nullptr;
	}
	RK_ASSERT(res.dir());
	return res.dir()->getOrCreateView(true); // Will call openNewOutputWindow(), unless a view alredy exists
}

RKMDIWindow* RKWorkplace::openNewOutputWindow(RKOutputDirectory* dir) {
	RK_TRACE (APP);

	RKHTMLWindow* win = new RKHTMLWindow(view(), RKHTMLWindow::HTMLOutputWindow);
	win->openURL(QUrl::fromLocalFile(dir->workPath()));
	RK_ASSERT(win->url().toLocalFile() == dir->workPath());
	addWindow(win);
	return win;
}

void RKWorkplace::newX11Window (QWindow* window_to_embed, int device_number) {
	RK_TRACE (APP);

	RKCaughtX11Window *window = new RKCaughtX11Window (window_to_embed, device_number);
	window->state = RKMDIWindow::Detached;
	addWindow (window, false);
}

void RKWorkplace::newRKWardGraphisWindow (RKGraphicsDevice* dev, int device_number) {
	RK_TRACE (APP);

	RKCaughtX11Window *window = new RKCaughtX11Window (dev, device_number);
	window->state = RKMDIWindow::Detached;
	addWindow (window, false);
}

RKMDIWindow* RKWorkplace::newObjectViewer (RObject *object) {
	RK_TRACE (APP);
	RK_ASSERT (object);

	RKWorkplaceObjectList object_windows = getObjectList (RKMDIWindow::ObjectWindow, RKMDIWindow::AnyWindowState);
	for (int i = 0; i < object_windows.size (); ++i) {
		RObjectViewer *viewer = static_cast<RObjectViewer *> (object_windows[i]);
		if (viewer->object () == object) {
			if (view ()->windowInActivePane (viewer)) {
				viewer->activate ();
				return viewer;
			}
		}
	}

	RObjectViewer *ov = new RObjectViewer (view (), object);
	addWindow (ov);
	return ov;
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

	RKEditorDataFrame* ed = new RKEditorDataFrame(name, nullptr);
	addWindow (ed);
	ed->activate ();

	return ed;
}

RKEditor *RKWorkplace::editObject (RObject *object) {
	RK_TRACE (APP);
	RK_ASSERT (object);

	RObject *iobj = object;
	if (!iobj->isDataFrame ()) {
		if (iobj->isVariable () && iobj->parentObject ()->isDataFrame ()) {
			iobj = iobj->parentObject ();
		} else {
			return nullptr;
		}
	}

	bool activate = window_style_override != "preview";
	RKEditor *ed = nullptr;
	QList<RKEditor*> existing_editors = object->editors ();
	RKMDIWindow *nw = getNamedWindow(window_name_override);
	for (int i = 0; i < existing_editors.size (); ++i) {
		RObject *eobj = existing_editors[i]->getObject ();
		if (eobj == iobj) {
			auto candidate = existing_editors[i];
			if (view()->windowInActivePane(candidate) || (nw == candidate)) {
				ed = candidate;
				break;
			}
		}
	}

	if (!ed) {
		unsigned long size = 1;
		const auto objDims = iobj->getDimensions ();
		for (int dim : objDims) {
			size *= dim;
		}
		if ((RKSettingsModuleGeneral::warnLargeObjectThreshold () != 0) && (size > RKSettingsModuleGeneral::warnLargeObjectThreshold ())) {
			if (KMessageBox::warningContinueCancel (view (), i18n ("You are about to edit object \"%1\", which is very large (%2 fields). RKWard is not optimized to handle very large objects in the built in data editor. This will use a lot of memory, and - depending on your system - might be very slow. For large objects it is generally recommended to edit using command line means or to split into smaller chunks before editing. On the other hand, if you have enough memory, or the data is simple enough (numeric data is easier to handle, than factor), editing may not be a problem at all. You can configure this warning (or turn it off entirely) under Settings->Configure RKWard->General.\nReally edit object?", iobj->getFullName (), size), i18n ("About to edit very large object")) != KMessageBox::Continue) {
				return nullptr;
			}
		}

		ed = new RKEditorDataFrame(static_cast<RContainerObject*>(iobj), nullptr);
		addWindow (ed);
	}

	if (activate) ed->activate ();
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

bool RKWorkplace::closeWindow (RKMDIWindow *window, RKMDIWindow::CloseWindowMode ask_save) {
	RK_TRACE (APP);
	RK_ASSERT (windows.contains (window));

	bool tool_window = window->isToolWindow ();
	bool closed = window->close (ask_save);		// all the rest should happen in removeWindow ()

	if (closed && tool_window) windowRemoved ();	// for regular windows, this happens in removeWindow(), already
	return closed;
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

bool RKWorkplace::closeAll(int type, int state) {
	RK_TRACE(APP);

	return closeWindows(getObjectList(type, state));
}

bool RKWorkplace::closeWindows(QList<RKMDIWindow*> windows, RKMDIWindow::CloseWindowMode ask_save) {
	RK_TRACE(APP);

	RKWardMainWindow::getMain()->lockGUIRebuild(true);

	bool ok = (ask_save == RKMDIWindow::NoAskSaveModified) || RKSaveModifiedDialog::askSaveModified(this, windows, false);
	if (ok) {
		for (int i = windows.size() - 1; i >= 0; --i) {
			RK_ASSERT(closeWindow(windows[i], RKMDIWindow::NoAskSaveModified));
		}
	}
	RKWardMainWindow::getMain()->lockGUIRebuild(false);
	return ok;
}

bool RKWorkplace::closeWorkspace() {
	RK_TRACE(APP);

	bool ok = RKSaveModifiedDialog::askSaveModified(this, windows, true);
	if (!ok) return false;
	RKOutputDirectory::purgeAllNoAsk();
	return closeWindows(windows, RKMDIWindow::NoAskSaveModified);
}

void RKWorkplace::removeWindow (QObject *object) {
	RK_TRACE (APP);
	RKMDIWindow *window = static_cast<RKMDIWindow *> (object);

	// remove from history first (otherwise, we might find it there, when trying to activate a new window)
	history->removeWindow (window);

	// WARNING: the window is dead. Don't call any functions on it.

	RK_ASSERT (windows.contains (window));
	windows.removeAll (window);		// do this first! view()->removeWindow will call activePage() indirectly from setCaption, causing us to iterate over all known windows!
	if (view ()->hasWindow (window)) view ()->removeWindow (window, true);

	windowRemoved ();
}

void RKWorkplace::windowRemoved () {
	RK_TRACE (APP);

	if (activeWindow (RKMDIWindow::AnyWindowState) != nullptr) return;	// some RKMDIWindow is already active
	QWidget *appWin = QApplication::activeWindow ();
	if (appWin && appWin != RKWardMainWindow::getMain () && !qobject_cast<DetachedWindowContainer*> (appWin)) return; // a dialog window or the like is active

	// try to activate an attached document window, first
	RKMDIWindow *window = view ()->activePage ();
	if (window) {
		window->activate (true);
		return;
	}

	// some document window in the history? Try that.
	window = history->previousDocumentWindow ();
	if (window) {
		window->activate (true);
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

RKMDIWindow *RKWorkplace::activeWindow(RKMDIWindow::State state) const {
	RK_TRACE (APP);

	for (auto it = windows.constBegin (); it != windows.constEnd (); ++it) {
		if (!(state & ((*it)->state))) continue;

		if ((*it)->isActive ()) {
			return *it;
		}
	}
	return nullptr;
}

RKMDIWindow * RKWorkplace::windowForPart(KParts::Part* part) const {
	for (auto it = windows.constBegin(); it != windows.constEnd(); ++it) {
		if ((*it)->getPart() == part) {
			return *it;
		}
	}
	return nullptr;
}

QUrl checkAdjustRestoredUrl (const QString &_url, const QString &old_base) {
	QUrl url = QUrl::fromUserInput (_url, QString (), QUrl::AssumeLocalFile);

	if (old_base.isEmpty ()) return (url);
	QUrl new_base_url = RKWorkplace::mainWorkplace ()->workspaceURL ().adjusted (QUrl::RemoveFilename | QUrl::NormalizePathSegments);
	if (new_base_url.isEmpty ()) return (url);
	QUrl old_base_url (old_base);
	if (old_base_url == new_base_url) return (url);

	// TODO: Should we also care about non-local files? In theory: yes, but stat'ing remote files for existence can take a long time.
	if (!(old_base_url.isLocalFile () && new_base_url.isLocalFile () && url.isLocalFile ())) return (url);

	// if the file exists, unadjusted, return it.
	if (QFileInfo::exists(url.toLocalFile())) return (url);

	// check whether a file exists for the adjusted url
	QString relative = QDir (new_base_url.path ()).absoluteFilePath (QDir (old_base_url.path ()).relativeFilePath (url.path ()));
	if (QFileInfo::exists(relative)) return (QUrl::fromLocalFile(relative));
	return (url);
}

QString RKWorkplace::makeItemDescription (RKMDIWindow *win) const {
	QString type, specification;
	QStringList params;
	if (win->isType (RKMDIWindow::DataEditorWindow)) {
		type = "data";
		specification = static_cast<RKEditor*> (win)->getObject ()->getFullName ();
	} else if (win->isType (RKMDIWindow::CommandEditorWindow)) {
		type = "script";
		specification = static_cast<RKCommandEditorWindow*> (win)->url ().url ();
		if (specification.isEmpty ()) specification = static_cast<RKCommandEditorWindow*> (win)->id ();
	} else if (win->isType (RKMDIWindow::OutputWindow)) {
		RKOutputDirectory *dir = RKOutputDirectory::findOutputByWindow(win);
		if (dir) {
			type = "rkoutput";
			specification = QUrl::fromLocalFile(dir->filename()).url();
			if (dir->isActive()) type.append(QStringLiteral(".active"));
		} else {
			// legacy support for rk.set.html.output.file()
			type = "output";
			specification = static_cast<RKHTMLWindow*> (win)->url ().url ();
		}
	} else if (win->isType (RKMDIWindow::HelpWindow)) {
		type = "help";
		specification = static_cast<RKHTMLWindow*> (win)->restorableUrl ().url ();
	} else if (win->isType(RKMDIWindow::PDFWindow)) {
		type = "pdf";
		specification = static_cast<RKPDFWindow*>(win)->url().url();
	} else if (win->isToolWindow ()) {
		type = RKToolWindowList::idOfWindow (win);
	} else if (win->isType (RKMDIWindow::ObjectWindow)) {
		type = "object";
		specification = static_cast<RObjectViewer*> (win)->object ()->getFullName ();
	}
	if (!type.isEmpty ()) {
		if (!win->isAttached ()) {
			params.append (QString ("detached,") + QString::number (win->x ()) + ',' + QString::number (win->y ()) + ',' + QString::number (win->width ()) + ',' + QString::number (win->height ()));
		}
		if (win->isToolWindow ()) {
			int sidebar = RKToolWindowList::Nowhere;
			for (int i = 0; i < TOOL_WINDOW_BAR_COUNT; ++i) {
				if (win->tool_window_bar == tool_window_bars[i]) {
					sidebar = i;
					break;
				}
			}
			params.append (QString ("sidebar,") + QString::number (sidebar));
		}
		return (type + "::" + params.join (":") + "::" + specification);
	}

	return QString ();
}

struct ItemSpecification {
	QString type;
	QString specification;
	QStringList params;
};

ItemSpecification parseItemDescription (const QString &description) {
	ItemSpecification ret;

	// Item format for rkward <= 0.5.4: "type:specification"
	// Item format for rkward <= 0.5.5: "type::[optional_params1[:optional_params2[:...]]]::specification"
	int typeend = description.indexOf (':');
	if ((typeend < 0) || (typeend >= (description.size () - 1))) {
		RK_ASSERT (false);
		return ret;
	}
	ret.type = description.left (typeend);
	if (description.at (typeend + 1) == ':') {	// rkward 0.5.5 or later
		int specstart = description.indexOf ("::", typeend + 2);
		if (specstart < typeend) {
			RK_ASSERT (false);
			return ret;
		}
		ret.params = description.mid (typeend + 2, specstart - typeend - 2).split (':', Qt::SkipEmptyParts);
		ret.specification = description.mid (specstart + 2);
	} else {
		ret.specification = description.mid (typeend + 1);
	}

	return ret;
}

RKMDIWindow* restoreDocumentWindowInternal (RKWorkplace* wp, const ItemSpecification &spec, const QString &base) {
	RK_TRACE (APP);

	RKMDIWindow *win = nullptr;
	if (spec.type == "data") {
		RObject *object = RObjectList::getObjectList ()->findObject (spec.specification);
		if (object) win = wp->editObject (object);
	} else if (spec.type == "script") {
		QUrl url = checkAdjustRestoredUrl (spec.specification, base);
		win = wp->openScriptEditor (url, QString ());
	} else if (spec.type == "output") {
		win = wp->openOutputWindow (checkAdjustRestoredUrl(spec.specification, base));
	} else if (spec.type.startsWith("rkoutput")) {
		RKOutputDirectory *dir = RKOutputDirectory::get(checkAdjustRestoredUrl(spec.specification, base).toLocalFile(), false).dir();
		if (!dir) return nullptr;
		if (spec.type.endsWith(".active")) dir->activate();
		win = RKWorkplace::mainWorkplace()->openOutputWindow(QUrl::fromLocalFile(dir->workPath()));
	} else if (spec.type == "help") {
		auto url = checkAdjustRestoredUrl (spec.specification, base);
		if (!url.isEmpty()) win = wp->openHelpWindow(url, true);
	} else if (spec.type == "pdf") {
		win = wp->openPDFWindow(checkAdjustRestoredUrl(spec.specification, base));
	} else if (spec.type == "object") {
		RObject *object = RObjectList::getObjectList ()->findObject (spec.specification);
		if (object) win = wp->newObjectViewer (object);
	}
	return win;
}

bool RKWorkplace::restoreDocumentWindow (const QString &description, const QString &base) {
	RK_TRACE (APP);

	return (restoreDocumentWindowInternal(this, parseItemDescription(description), base) != nullptr);
}

QStringList RKWorkplace::makeWorkplaceDescription () {
	RK_TRACE (APP);

	QStringList workplace_description;

	// first, save the base directory of the workplace. This allows us to cope better with moved workspaces while restoring.
	QUrl base_url = workspaceURL ().adjusted (QUrl::RemoveFilename);
	if (base_url.isLocalFile () && !base_url.isEmpty ()) workplace_description.append ("base::::" + base_url.url ());

	// window order in the workplace view may have changed with respect to our list. Thus we first generate a properly sorted list
	const RKWorkplaceObjectList list = getObjectList (RKMDIWindow::DocumentWindow, RKMDIWindow::Detached);
	for (RKMDIWindow *win : list) {
		QString desc = makeItemDescription (win);
		if (!desc.isEmpty ()) workplace_description.append (desc);
	}

	workplace_description.append (QStringLiteral ("layout::::") + wview->listLayout ());
	workplace_description.append (wview->listContents ());

	const auto objectList = getObjectList (RKMDIWindow::ToolWindow, RKMDIWindow::AnyWindowState);
	for (RKMDIWindow *win : objectList) {
		QString desc = makeItemDescription (win);
		if (!desc.isEmpty ()) workplace_description.append (desc);
	}
	return workplace_description;
}

void RKWorkplace::saveWorkplace(const QUrl& for_url, RCommandChain *chain) {
	RK_TRACE (APP);
// TODO: This is still a mess. All workplace-related settings, including the workspaceConfig(), should be saved to a single place, and in 
// standard KConfig format.
	if (RKSettingsModuleGeneral::workplaceSaveMode() != RKSettingsModuleGeneral::SaveWorkplaceWithWorkspace) return;

	QString file_param;
	if (!for_url.isEmpty()) file_param = QString("file=") + RObject::rQuote(for_url.toLocalFile() + QStringLiteral(".rkworkplace")) + QStringLiteral(", ");
	RInterface::issueCommand(new RCommand("rk.save.workplace(" + file_param + "description=" + RObject::rQuote(makeWorkplaceDescription().join("\n")) + ')', RCommand::App, i18n("Save Workplace layout")), chain);
}

void RKWorkplace::restoreWorkplace (RCommandChain *chain, bool merge) {
	RK_TRACE (APP);
	if (RKSettingsModuleGeneral::workplaceSaveMode () != RKSettingsModuleGeneral::SaveWorkplaceWithWorkspace) return;

	QString no_close_windows;
	if (merge) no_close_windows = "close.windows = FALSE";
	RInterface::issueCommand(new RCommand("rk.restore.workplace(" + no_close_windows + ')', RCommand::App, i18n ("Restore Workplace layout")), chain);
}

void RKWorkplace::restoreWorkplace (const QStringList &description) {
	RK_TRACE (APP);

	RKWardMainWindow::getMain ()->lockGUIRebuild (true);
	QString base;
	for (int i = 0; i < description.size (); ++i) {
		ItemSpecification spec = parseItemDescription (description[i]);
		RKMDIWindow *win = nullptr;
		if (spec.type == "base") {
			RK_ASSERT (i == 0);
			base = spec.specification;
		} else if (restoreDocumentWindowInternal (this, spec, base)) {
			// it was restored. nothing else to do
		} else if (spec.type == "layout") {
			view ()->restoreLayout (spec.specification);
		} else if (spec.type == "pane_end") {
			view ()->nextPane ();
		} else {
			win = RKToolWindowList::findToolWindowById (spec.type);
			RK_ASSERT (win);
		}

		// apply generic window parameters
		if (win) {
			for (int p = 0; p < spec.params.size (); ++p) {
				if (spec.params[p].startsWith (QLatin1String ("sidebar"))) {
					int position = spec.params[p].section (',', 1).toInt ();
					placeInToolWindowBar (win, position);
				}
				if (spec.params[p].startsWith (QLatin1String ("detached"))) {
					QStringList geom = spec.params[p].split (',');
					win->hide ();
					win->setGeometry (geom.value (1).toInt (), geom.value (2).toInt (), geom.value (3).toInt (), geom.value (4).toInt ());
					detachWindow (win);
				}
			}
		}
	}
	view ()->purgeEmptyPanes ();
	RKWardMainWindow::getMain ()->lockGUIRebuild (false);
}

void RKWorkplace::splitAndAttachWindow (RKMDIWindow* source) {
	RK_TRACE (APP);
	RK_ASSERT (source);

	if (source->isType (RKMDIWindow::CommandEditorWindow)) {
		QUrl url = static_cast<RKCommandEditorWindow*> (source)->url ();
		openScriptEditor (url, QString ());
	} else if (source->isType (RKMDIWindow::HelpWindow)) {
		openHelpWindow (static_cast<RKHTMLWindow*> (source)->url ());
	} else if (source->isType (RKMDIWindow::OutputWindow)) {
		openOutputWindow (static_cast<RKHTMLWindow*> (source)->url ());
	} else if (source->isType (RKMDIWindow::DataEditorWindow)) {
		editObject (static_cast<RKEditor*> (source)->getObject ());
	} else if (source->isType (RKMDIWindow::ObjectWindow)) {
		newObjectViewer (static_cast<RObjectViewer*> (source)->object ());
	} else {
		openHelpWindow (QUrl ("rkward://page/rkward_split_views"));
	}
}


///////////////////////// END RKWorkplace ////////////////////////////
///////////////////// BEGIN RKMDIWindowHistory ///////////////////////

#include "../misc/rkstandardicons.h"
#include <QListWidget>

class RKMDIWindowHistoryWidget : public QListWidget {
public:
	RKMDIWindowHistoryWidget() : QListWidget(nullptr) {
		RK_TRACE (APP);

		current = 0;
		setFocusPolicy (Qt::StrongFocus);
		setWindowFlags (Qt::Popup);
	}

	~RKMDIWindowHistoryWidget () {
		RK_TRACE (APP);
	}

	void update (const QList<RKMDIWindow*> &windows) {
		RK_TRACE (APP);

		clear ();
		_windows = windows;
		for (int i = windows.count() - 1; i >= 0; --i) {  // most recent first / on top
			RKMDIWindow *win = _windows.at(i);
			QListWidgetItem *item = new QListWidgetItem (this);
			item->setIcon (RKStandardIcons::iconForWindow (win));
			item->setText (win->windowTitle ());
		}
		if (current >= count ()) current = count () - 1;
		if (current < 0) {
			hide ();
			return;
		}
		setCurrentRow (current);
	}

	void next () {
		RK_TRACE (APP);

		if (--current < 0) current = count () - 1;
		setCurrentRow (current);
	}

	void prev () {
		RK_TRACE (APP);

		if (++current >= count ()) current = 0;
		setCurrentRow (current);
	}

private:
	void focusOutEvent (QFocusEvent *) override {
		RK_TRACE (APP);

		deleteLater ();
	}

	void keyReleaseEvent (QKeyEvent *ev) override {
		RK_TRACE (APP);

		if (ev->modifiers () == Qt::NoModifier) {
			commit ();
		}
	}

	void mouseReleaseEvent (QMouseEvent *ev) override {
		RK_TRACE (APP);

		// HACK to get by without slots, and the associated moc'ing
		QListWidget::mouseReleaseEvent (ev);
		commit ();
	}

	void commit () {
		RK_TRACE (APP);

		current = currentRow ();
		if ((current > 0) && (current < count ())) {
			RKMDIWindow *win = _windows.value (count () - 1 - current);
			RK_ASSERT (win);
			win->activate (true);
		}
		deleteLater ();
	}

	int current;
	QList<RKMDIWindow*> _windows;
};

RKMDIWindowHistory::RKMDIWindowHistory (QObject *parent) : QObject (parent) {
	RK_TRACE (APP);

	switcher = nullptr;
}

RKMDIWindowHistory::~RKMDIWindowHistory () {
	RK_TRACE (APP);

	RK_DEBUG (APP, DL_DEBUG, "Remaining windows in history: %d", recent_windows.count ());
}

void RKMDIWindowHistory::windowActivated (RKMDIWindow *window) {
	RK_TRACE (APP);

	if (!window) return;
	if (!recent_windows.isEmpty () && (window == recent_windows.last ())) return;

	// update lists
	recent_windows.removeAll (window);		// remove dupes
	recent_windows.append (window);

	updateSwitcher ();

	Q_EMIT activeWindowChanged (window);
}

void RKMDIWindowHistory::next (QAction* prev_action, QAction *next_action) {
	RK_TRACE (APP);

	if (recent_windows.isEmpty ()) return;
	getSwitcher (prev_action, next_action)->next ();
}

void RKMDIWindowHistory::prev (QAction* prev_action, QAction *next_action) {
	RK_TRACE (APP);

	if (recent_windows.isEmpty ()) return;
	getSwitcher (prev_action, next_action)->prev ();
}

RKMDIWindow* RKMDIWindowHistory::previousDocumentWindow () {
	RK_TRACE (APP);

	for (int i = recent_windows.count () - 1; i >= 0; --i) {
		if (!recent_windows[i]->isToolWindow ()) return (recent_windows[i]);
	}
	return nullptr;
}

void RKMDIWindowHistory::updateSwitcher () {
	RK_TRACE (APP);

	if (switcher) switcher->update (recent_windows);
}

void RKMDIWindowHistory::removeWindow (RKMDIWindow *window) {
	RK_TRACE (APP);

	recent_windows.removeAll (window);
	updateSwitcher ();
}

RKMDIWindowHistoryWidget* RKMDIWindowHistory::getSwitcher (QAction* prev_action, QAction *next_action) {
	RK_TRACE (APP);

	if (switcher) return switcher;

	switcher = new RKMDIWindowHistoryWidget ();
	connect (switcher, &QObject::destroyed, this, &RKMDIWindowHistory::switcherDestroyed);
	switcher->addAction (prev_action);
	switcher->addAction (next_action);
	switcher->update (recent_windows);
	switcher->show ();
	QWidget *act = QApplication::activeWindow ();
	if (act) {
		int center_x = act->x () + act->width () / 2;
		int center_y = act->y () + act->height () / 2;
		switcher->move (center_x - switcher->width () / 2, center_y - switcher->height () / 2);
	} else {
		RK_ASSERT (false);
	}
	switcher->setFocus ();

	return switcher;
}

void RKMDIWindowHistory::switcherDestroyed () {
	RK_TRACE (APP);

	RK_ASSERT (switcher);
	switcher = nullptr;
}

void RKMDIWindowHistory::popLastWindow (RKMDIWindow* match) {
	RK_TRACE (APP);

	if (recent_windows.isEmpty ()) return;
	else if (recent_windows.last () == match) recent_windows.removeLast ();
	updateSwitcher ();
}


