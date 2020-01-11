/***************************************************************************
                          katepluginintegration  -  description
                             -------------------
    begin                : Mon Jun 12 2017
    copyright            : (C) 2017-2020 by Thomas Friedrichsmeier
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

#include "katepluginintegration.h"

#include <QWidget>
#include <QFileInfo>
#include <QVBoxLayout>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KPluginMetaData>
#include <KTextEditor/Editor>
#include <KTextEditor/Application>
#include <KTextEditor/Plugin>
#include <KTextEditor/SessionConfigInterface>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KXMLGUIFactory>

#include "../rkward.h"
#include "rkworkplace.h"
#include "rkworkplaceview.h"
#include "rkcommandeditorwindow.h"
#include "../misc/rkdummypart.h"
#include "../settings/rksettingsmodulecommandeditor.h"

#include "../debug.h"

///  BEGIN  KTextEditor::Application interface

KatePluginIntegrationApp::KatePluginIntegrationApp(QObject *parent) : QObject (parent) {
	RK_TRACE (APP);

	dummy_view = 0;
	window = new KatePluginIntegrationWindow(this);
	app = new KTextEditor::Application(this);
	KTextEditor::Editor::instance()->setApplication(app);

	// enumerate all available kate plugins
	QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral ("ktexteditor"), [](const KPluginMetaData &md) { return md.serviceTypes().contains(QLatin1String("KTextEditor/Plugin")); });
    for (int i = plugins.size() -1; i >= 0; --i) {
		PluginInfo info;
		info.plugin = 0;
		info.data = plugins[i];
		// Note: creates a lookup-table *and* eliminates potential dupes later in the search path
		known_plugins.insert(idForPlugin(info.data), info);

		// TODO: remove me
		qDebug ("%s", qPrintable(info.data.fileName()));
	}
	// NOTE: Destructor is too late for this, esp. As plugin destructors will try to unregister from the guiFactory(), and such.
	connect(RKWardMainWindow::getMain(), &RKWardMainWindow::aboutToQuitRKWard, this, &KatePluginIntegrationApp::saveConfigAndUnload);
}

KatePluginIntegrationApp::~KatePluginIntegrationApp() {
	RK_TRACE (APP);
}

KTextEditor::View *KatePluginIntegrationApp::dummyView() {
	if (!dummy_view) {
		RK_TRACE (APP);
		KTextEditor::Document *doc = KTextEditor::Editor::instance()->createDocument (this);
		dummy_view = doc->createView(0);
		dummy_view->hide();
		// Make sure it does not accumulate cruft.
		connect(doc, &KTextEditor::Document::textChanged, doc, &KTextEditor::Document::clear);
	}
	return dummy_view;
}

QString KatePluginIntegrationApp::idForPlugin(const KTextEditor::Plugin *plugin) const {
	for (auto it = known_plugins.constBegin(); it != known_plugins.constEnd(); ++it) {
		if (it.value().plugin == plugin) return it.key();
	}
	return QString();
}

QString KatePluginIntegrationApp::idForPlugin(const KPluginMetaData &plugin) const {
	return QFileInfo(plugin.fileName()).baseName();
}

QObject* KatePluginIntegrationApp::loadPlugin (const QString& identifier) {
	RK_TRACE (APP);

	if (!known_plugins.contains (identifier)) {
		RK_DEBUG (APP, DL_WARNING, "Plugin %s is not known", qPrintable (identifier));
		return 0;
	}

	KPluginFactory *factory = KPluginLoader(known_plugins[identifier].data.fileName ()).factory ();
	if (factory) {
		KTextEditor::Plugin *plugin = factory->create<KTextEditor::Plugin>(this, QVariantList () << identifier);
		if (plugin) {
			known_plugins[identifier].plugin = plugin;
			emit KTextEditor::Editor::instance()->application()->pluginCreated(identifier, plugin);
			QObject* created = mainWindow()->createPluginView(plugin);
			if (created) {
				emit mainWindow()->main->pluginViewCreated(identifier, created);
				KTextEditor::SessionConfigInterface *interface = qobject_cast<KTextEditor::SessionConfigInterface *>(created);
				if (interface) {
					// NOTE: Some plugins (noteably the Search in files plugin) will misbehave, unless readSessionConfig has been called!
					KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("KatePlugin:%1:").arg(identifier));
					interface->readSessionConfig(group);
				}
			}
			return plugin;
		}
	}

    return 0;
}

void KatePluginIntegrationApp::saveConfigAndUnload() {
	RK_TRACE (APP);

	for (auto it = known_plugins.constBegin(); it != known_plugins.constEnd(); ++it) {
		KTextEditor::Plugin* plugin = it.value().plugin;
		if (!plugin) continue;
		QObject* view = mainWindow()->pluginView(it.key());
		if (view) {
			KTextEditor::SessionConfigInterface* interface = qobject_cast<KTextEditor::SessionConfigInterface *>(view);
			if (interface) {
				KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("KatePlugin:%1:").arg(it.key()));
				interface->writeSessionConfig(group);
			}
			emit mainWindow()->main->pluginViewDeleted(it.key(), view);
			delete view;
		}
		emit app->pluginDeleted(it.key(), plugin);
		delete plugin;
	}
	known_plugins.clear();
}

QList<KTextEditor::MainWindow *> KatePluginIntegrationApp::mainWindows() {
	RK_TRACE (APP);
	QList<KTextEditor::MainWindow *> ret;
	ret.append (window->main);
	return ret;
}

KTextEditor::MainWindow *KatePluginIntegrationApp::activeMainWindow() {
	RK_TRACE (APP);
	return window->main;
}

RKCommandEditorWindow* findWindowForView(KTextEditor::View *view) {
	RK_TRACE (APP);

	QList<RKMDIWindow*> w = RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow);
	for (int i = 0; i < w.size(); ++i) {
		KTextEditor::View *v = static_cast<RKCommandEditorWindow*>(w[i])->getView();
		if (v && (v == view)) {
			return static_cast<RKCommandEditorWindow*>(w[i]);
		}
	}
	return 0;
}

RKCommandEditorWindow* findWindowForDocument(KTextEditor::Document *document) {
	RK_TRACE (APP);

	QList<RKMDIWindow*> w = RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow);
	for (int i = 0; i < w.size(); ++i) {
		KTextEditor::View *v = static_cast<RKCommandEditorWindow*>(w[i])->getView();
		if (v && (v->document() == document)) {
			return static_cast<RKCommandEditorWindow*>(w[i]);
		}
	}
	return 0;
}

QList<KTextEditor::Document *> KatePluginIntegrationApp::documents() {
	RK_TRACE (APP);

	QList<RKMDIWindow*> w = RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow);
	QList<KTextEditor::Document*> ret;
	for (int i = 0; i < w.size (); ++i) {
		KTextEditor::View *v = static_cast<RKCommandEditorWindow*>(w[i])->getView();
		if (v) ret.append(v->document());
	}
	if (ret.isEmpty()) {
		// See the NOTE in KatePluginIntegrationWindow::activeView()
		ret.append(dummyView()->document());
	}
	return ret;
}

KTextEditor::Document *KatePluginIntegrationApp::findUrl(const QUrl &url) {
	RK_TRACE (APP);

	QUrl _url = url.adjusted(QUrl::NormalizePathSegments);  // Needed?
	QList<RKMDIWindow*> w = RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow);
	for (int i = 0; i < w.size (); ++i) {
		if (_url == static_cast<RKCommandEditorWindow*>(w[i])->url().adjusted(QUrl::NormalizePathSegments)) {
			KTextEditor::View *v = static_cast<RKCommandEditorWindow*>(w[i])->getView();
			if (v) return v->document();
		}
	}
	return 0;
}

KTextEditor::Document *KatePluginIntegrationApp::openUrl(const QUrl &url, const QString &encoding) {
	RK_TRACE (APP);

	KTextEditor::View *v = window->openUrl(url, encoding);
	if (v) return v->document();
	return 0;
}

bool KatePluginIntegrationApp::closeDocument(KTextEditor::Document *document) {
	RK_TRACE (APP);

	RKMDIWindow *w = findWindowForDocument(document);
	if (w) return RKWorkplace::mainWorkplace()->closeWindow(w); // NOTE: Closes only a single view of the document
	return false;
}

bool KatePluginIntegrationApp::closeDocuments(const QList<KTextEditor::Document *> &documents) {
	RK_TRACE (APP);

	bool allfound = true;
	QList<RKMDIWindow*> w = RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow);
	QList<RKMDIWindow*> toclose;
	for (int i = 0; i < documents.size(); ++i) {
		bool found = false;
		for (int j = 0; j < w.size(); ++j) {
			KTextEditor::View *v = static_cast<RKCommandEditorWindow*>(w[j])->getView();
			if (v && v->document() == documents[i]) {
				toclose.append(w[i]);
				found = true;
				break;
			}
		}
		if (!found) allfound = false;
	}

	return RKWorkplace::mainWorkplace()->closeWindows(toclose) && allfound;
}

KTextEditor::Plugin *KatePluginIntegrationApp::plugin(const QString &name) {
	RK_TRACE (APP);

	if (known_plugins.contains(name)) {
		return known_plugins[name].plugin;
	}
	return 0;
}

///  END  KTextEditor::Application interface
///  BEGIN  KTextEditor::MainWindow interface


KatePluginIntegrationWindow::KatePluginIntegrationWindow (KatePluginIntegrationApp *parent) : QObject (parent), KXMLGUIClient () {
	RK_TRACE (APP);

	// This one is passed to each created plugin
	main = new KTextEditor::MainWindow(this);
	// While this one may be accessed from plugins via KTextEditor::Editor::instance()->application()
	app = parent;
	active_plugin = 0;
}

class KatePluginToolWindow : public RKMDIWindow {
	Q_OBJECT
public:
	KatePluginToolWindow(QWidget *parent) : RKMDIWindow(parent, RKMDIWindow::KatePluginWindow, true) {
		RK_TRACE (APP);

		QVBoxLayout *layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		setPart(new RKDummyPart(this, this));
		initializeActivationSignals();
		setFocusPolicy(Qt::ClickFocus);
	}
	~KatePluginToolWindow() {
		RK_TRACE (APP);
	}

/** This is a bit lame, but the plugin does not add itself to the parent widget's layout by itself. So we need this override
 *  to do that. Where did the good old KVBox go? */
	void childEvent(QChildEvent *ev) override {
		if ((ev->type() == QEvent::ChildAdded) && qobject_cast<QWidget *>(ev->child())) {
			QWidget *widget = qobject_cast<QWidget *>(ev->child());
			layout()->addWidget(widget);
			setFocusProxy(widget);
		}
		QWidget::childEvent(ev);
	}
};

QWidget* KatePluginIntegrationWindow::createToolView (KTextEditor::Plugin *plugin, const QString &identifier, KTextEditor::MainWindow::ToolViewPosition pos, const QIcon &icon, const QString &text) {
	RK_TRACE (APP);

	RK_DEBUG(APP, DL_DEBUG, "createToolView for %p, %s, position %d, %s", plugin, qPrintable(identifier), pos, qPrintable(text));
	// TODO: Set proper RKMDIWindow:type
	KatePluginToolWindow *window = new KatePluginToolWindow(RKWorkplace::mainWorkplace()->view());
	window->setCaption(text);
	window->setWindowIcon(icon);
	RKWorkplace::mainWorkplace()->placeInToolWindowBar(window, pos);
	RKToolWindowList::registerToolWindow(window, identifier, (RKToolWindowList::Placement) pos, 0);
	plugin_resources[plugin].windows.append(window);

	return window;
}

bool KatePluginIntegrationWindow::showToolView (QWidget *widget) {
	RK_TRACE (APP);
	RKMDIWindow *w = qobject_cast<RKMDIWindow*>(widget);
	if (w) w->activate();
	else {
		RK_DEBUG(APP, DL_ERROR, "Failed to find mdi window for (plugin) tool view %p", widget);
		RK_ASSERT(w);
		widget->show();
	}
	return true;
}

KXMLGUIFactory *KatePluginIntegrationWindow::guiFactory () {
	RK_TRACE (APP);

	// NOTE: We'd rather like to add the plugin to our own RKMDIWindows, rather than
	// allowing it direct access to the guiFactory()
	// See the HACK in createPluginView().
	return factory ();
}

QWidget *KatePluginIntegrationWindow::window() {
	RK_TRACE (APP);

	return RKWorkplace::mainWorkplace()->view()->window();
}

QList<KTextEditor::View *> KatePluginIntegrationWindow::views() {
	RK_TRACE (APP);

	QList<RKMDIWindow*> w = RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow);
	QList<KTextEditor::View*> ret;
	for (int i = 0; i < w.size (); ++i) {
		ret.append (static_cast<RKCommandEditorWindow*>(w[i])->getView());
	}
	return ret;
}

KTextEditor::View *KatePluginIntegrationWindow::activeView() {
	RK_TRACE (APP);

	RKMDIWindow *w = RKWorkplace::mainWorkplace()->activeWindow(RKMDIWindow::AnyWindowState);
	if (w && w->isType (RKMDIWindow::CommandEditorWindow)) {
		return static_cast<RKCommandEditorWindow*>(w)->getView();
	}
	// NOTE: As far as RKWard is concerned, the active window will most likely be the tool window at this point, while the
	//       intention will be to get an active window that the tool should operate on. So get the last used window from
	//       history. (Another option would be to check which window is on top in the view area, but this will be difficult
	//       for split views.
	RKMDIWindow* candidate =  RKWorkplace::getHistory()->previousDocumentWindow();
	if (candidate && candidate->isType(RKMDIWindow::CommandEditorWindow)) return static_cast<RKCommandEditorWindow*>(candidate)->getView();
	// NOTE: It looks like some plugins assume this cannot return 0. That's a bug in the plugin, but still one that could
	//       be quite prevalent, as in kate, that assumption holds. So, to be safe, we create a dummy window on the fly.
	return app->dummyView();
}

KTextEditor::View *KatePluginIntegrationWindow::activateView(KTextEditor::Document *document) {
	RK_TRACE (APP);

	RKCommandEditorWindow* w = findWindowForDocument(document);
	if (w) {
		w->activate();
		return w->getView();
	}
	if (app->dummy_view && document == app->dummy_view->document()) return app->dummy_view;
	return 0;
}

KTextEditor::View *KatePluginIntegrationWindow::openUrl(const QUrl &url, const QString &encoding) {
	RK_TRACE (APP);
	RKMDIWindow *w = RKWorkplace::mainWorkplace()->openScriptEditor(url, encoding, RKSettingsModuleCommandEditor::matchesScriptFileFilter(url.fileName()));
	if (w) return static_cast<RKCommandEditorWindow*>(w)->getView();

	RK_ASSERT(w);  // should not happen
	return 0;
}

QObject *KatePluginIntegrationWindow::pluginView(const QString &name) {
	RK_TRACE (APP);

	return plugin_resources.value(app->plugin(name)).view;
}

bool KatePluginIntegrationWindow::closeSplitView(KTextEditor::View* view) {
	RK_TRACE (APP);

	// TODO: This should close the area that this view is in, not necessarily the view itself. However, if the same doc
	//       is also present in the area to merge into, then close this view, keeping the other.
	return closeView(view);
}

bool KatePluginIntegrationWindow::closeView(KTextEditor::View* view) {
	RK_TRACE (APP);

	RKMDIWindow *w = findWindowForView(view);
	if (w) return RKWorkplace::mainWorkplace()->closeWindow(w);
	return false;
}


bool KatePluginIntegrationWindow::hideToolView(QWidget* widget) {
	RK_TRACE (APP);

	RKMDIWindow *w = qobject_cast<RKMDIWindow*>(widget);
	if (w) w->close(false);
	else {
		RK_ASSERT(w);
		widget->hide();
	}
	return true;
}

/* These appear to be truly optional, so let's disable them for now.
void KatePluginIntegrationWindow::hideViewBar(KTextEditor::View* view) {}
void KatePluginIntegrationWindow::showViewBar(KTextEditor::View* view) {}
void KatePluginIntegrationWindow::deleteViewBar(KTextEditor::View* view) {}
void KatePluginIntegrationWindow::addWidgetToViewBar(KTextEditor::View* view, QWidget* bar) {}
QWidget *KatePluginIntegrationWindow::createViewBar(KTextEditor::View *view) {} */

bool KatePluginIntegrationWindow::moveToolView(QWidget* widget, KTextEditor::MainWindow::ToolViewPosition pos) {
	RK_TRACE (APP);

	RKMDIWindow *w = qobject_cast<RKMDIWindow*>(widget);
	if (w) {
		RKWorkplace::mainWorkplace ()->placeInToolWindowBar (w, pos);
		return true;
	}
	return false;
}

void KatePluginIntegrationWindow::splitView(Qt::Orientation orientation) {
	RK_TRACE (APP);
	RKWorkplace::mainWorkplace()->view()->splitView(orientation);
}

bool KatePluginIntegrationWindow::viewsInSameSplitView(KTextEditor::View* view1, KTextEditor::View* view2) {
	RK_TRACE (APP);
	// TODO not sure what the semantics of this really are. The two views are in the same view area (not visible, simultaneously), or in two areas split side-by-side?
	// However, this is essentially unused in kate.
	return false;
}

void fixupPluginUI(const QString &id, int num_of_client, KXMLGUIClient* client, RKMDIWindow* window) {
	RK_TRACE (APP);

	if (num_of_client == 0) {
		if (id == QStringLiteral("katesearchplugin")) {
			window->setCaption("Search in Scripts");
			// TODO
		}
	}
}

QObject* KatePluginIntegrationWindow::createPluginView(KTextEditor::Plugin* plugin) {
	RK_TRACE (APP);

	// HACK: Currently, plugins will add themselves to the main window's UI, without asking. We don't want that, as
	//       our MDI windows are enabled / disabled on activation. To hack around this, the catch the added clients,
	//       and put them, where they belong.
	connect(factory(), &KXMLGUIFactory::clientAdded, this, &KatePluginIntegrationWindow::catchXMLGUIClientsHack);
	active_plugin = plugin;
	PluginResources& resources = plugin_resources.insert(plugin, PluginResources()).value();
	resources.view = plugin->createView(main);
	active_plugin = 0;
	disconnect(factory(), &KXMLGUIFactory::clientAdded, this, &KatePluginIntegrationWindow::catchXMLGUIClientsHack);
	KXMLGUIClient* hacked_parent = this;
	QString id = app->idForPlugin(plugin);
	for (int i = 0; i < resources.clients.size(); ++i) {
		KXMLGUIClient* client = resources.clients[i];
		RKMDIWindow* window = resources.windows.value(i);
		if (window) {
			hacked_parent = window->getPart();;
		}
		factory()->removeClient(client);
		fixupPluginUI(id, i, client, window);
		hacked_parent->insertChildClient(client);
	}
	// TODO: If child clients were added to the window, itself, we need to tell the main window to rebuild.

	connect(plugin, &QObject::destroyed, [&]() { plugin_resources.remove(plugin); });
	return resources.view;
}

void KatePluginIntegrationWindow::catchXMLGUIClientsHack(KXMLGUIClient* client) {
	RK_TRACE (APP);
	if (active_plugin) {
		RK_ASSERT(plugin_resources.contains(active_plugin));
		plugin_resources[active_plugin].clients.append(client);
	} else {
		RK_DEBUG(APP, DL_DEBUG, "XML client created by unknown kate plugin");
	}
}

// TODO: Don't forget to make sure to emit all the signals!
// TODO: Apply plugin specific hacks as needed (e.g. moving "Tool" menu, removing broken actions)

///  END  KTextEditor::MainWindow interface

#include "katepluginintegration.moc"
