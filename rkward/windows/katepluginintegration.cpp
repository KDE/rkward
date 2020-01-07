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

#include "../rkward.h"
#include "rkworkplace.h"
#include "rkworkplaceview.h"
#include "rkcommandeditorwindow.h"
#include "../misc/rkdummypart.h"

#include "../debug.h"

KatePluginIntegration::KatePluginIntegration (QObject *parent) : QObject (parent), KXMLGUIClient () {
	RK_TRACE (APP);

	// This one is passed to each created plugin
	main = new KTextEditor::MainWindow(this);
	// While this one may be accessed from plugins via KTextEditor::Editor::instance()->application()
	KatePluginIntegration2 *buddy = new KatePluginIntegration2(this);
	KTextEditor::Editor::instance()->setApplication (buddy->app);

	// enumerate all available kate plugins
	QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins (QStringLiteral ("ktexteditor"), [](const KPluginMetaData &md) { return md.serviceTypes().contains(QLatin1String("KTextEditor/Plugin")); });
    for (int i = plugins.size() -1; i >= 0; --i) {
		KPluginMetaData &plugin = plugins[i];
		// Note: creates a lookup-table *and* eliminates potential dupes later in the search path 
		known_plugins.insert (idForPlugin (plugin), plugin);

		qDebug ("%s", qPrintable(plugin.fileName()));
	}
	// TODO
}

/** This is a bit lame, but the plugin does not add itself to the parent widget's layout by itself. So we need this class
 *  to do that. Where did the good old KVBox go? */
class KatePluginWrapperWidget : public QWidget {
	Q_OBJECT
public:
	KatePluginWrapperWidget(QWidget *parent) : QWidget(parent) {
		QVBoxLayout *layout = new QVBoxLayout(this);
		setLayout(layout);
		setFocusPolicy (Qt::StrongFocus);
	}
	void childEvent(QChildEvent *ev) override {
		if ((ev->type() == QEvent::ChildAdded) && qobject_cast<QWidget *>(ev->child())) {
			QWidget *widget = qobject_cast<QWidget *>(ev->child());
			setFocusProxy(widget);
			layout()->addWidget(widget);
		}
		QWidget::childEvent(ev);
	}
};

class KatePluginToolWindow : public RKMDIWindow {
	Q_OBJECT
public:
	KatePluginToolWindow(QWidget *parent, RKMDIWindow::Type type) : RKMDIWindow(parent, type, true) {
		RK_TRACE (APP);

		QVBoxLayout *layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		wrapper = new KatePluginWrapperWidget(this);
		layout->addWidget (wrapper);
		setPart(new RKDummyPart(this, wrapper));
	}
	~KatePluginToolWindow() {
		RK_TRACE (APP);
	}
	QWidget* wrapper;
};

QWidget * KatePluginIntegration::createToolView (KTextEditor::Plugin *plugin, const QString &identifier, KTextEditor::MainWindow::ToolViewPosition pos, const QIcon &icon, const QString &text) {
	RK_TRACE (APP);
	RKDebug (APP, DL_ERROR, "createToolView");

	// TODO: Save reference
	// TODO: Set proper RKMDIWindow:type
	KatePluginToolWindow *window = new KatePluginToolWindow(RKWorkplace::mainWorkplace()->view(), RKMDIWindow::ConsoleWindow);
	window->setCaption(text);
	window->setWindowIcon(icon);
	RKWorkplace::mainWorkplace()->placeInToolWindowBar(window, pos);

	return window->wrapper;
}

bool KatePluginIntegration::showToolView (QWidget *widget) {
	RK_TRACE (APP);
	RKDebug (APP, DL_ERROR, "showToolView");
	widget->show ();
	return true;
	// TODO
}

QString KatePluginIntegration::idForPlugin(const KPluginMetaData &plugin) const {
	return QFileInfo(plugin.fileName()).baseName();
}

QObject* KatePluginIntegration::loadPlugin (const QString& identifier) {
	RK_TRACE (APP);

	if (!known_plugins.contains (identifier)) {
		RK_DEBUG (APP, DL_WARNING, "Plugin %s is not known", qPrintable (identifier));
		return 0;
	}

	KPluginFactory *factory = KPluginLoader (known_plugins[identifier].fileName ()).factory ();
	if (factory) {
		KTextEditor::Plugin *plugin = factory->create<KTextEditor::Plugin> (this, QVariantList () << identifier);
		if (plugin) {
			emit KTextEditor::Editor::instance ()->application ()->pluginCreated (identifier, plugin);
			plugin->createView (main);
			return plugin;
		}
	}

    return 0;
	// TODO
}

KXMLGUIFactory *KatePluginIntegration::guiFactory () {
	RK_TRACE (APP);

	// We'd rather like to add the plugin to our own RKMDIWindows, rather than
	// allowing it direct access to the guiFactory()
	qDebug ("%p", sender());
	return factory ();
	// TODO
}

QWidget *KatePluginIntegration::window() {
	RK_TRACE (APP);
	return 0;
	// TODO
}

QList<KTextEditor::View *> KatePluginIntegration::views() {
	RK_TRACE (APP);

	QList<RKMDIWindow*> w = RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow);
	QList<KTextEditor::View*> ret;
	for (int i = 0; i < w.size (); ++i) {
		ret.append (static_cast<RKCommandEditorWindow*>(w[i])->getView());
	}
	return ret;
}

KTextEditor::View *KatePluginIntegration::activeView() {
	RK_TRACE (APP);

	RKMDIWindow *w = RKWorkplace::mainWorkplace()->activeWindow(RKMDIWindow::AnyWindowState);
	if (w && w->isType (RKMDIWindow::CommandEditorWindow)) {
		return static_cast<RKCommandEditorWindow*>(w)->getView();
	}
	return 0;
	// TODO: This probably isn't right: As far as RKWard is concerned, the active window will most likely be the tool window
	//       at this point, while the intention will be to get an active window that the tool should operate on.
	// TODO: Further, it looks like some plugins assume this cannot return 0, so maybe wee need to create a new window on the fly, if needed.
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

KTextEditor::View *KatePluginIntegration::activateView(KTextEditor::Document *document) {
	RK_TRACE (APP);

	RKCommandEditorWindow* w = findWindowForDocument(document);
	if (w) {
		w->activate();
		return w->getView();
	}
	return 0;
}

KTextEditor::View *KatePluginIntegration::openUrl(const QUrl &url, const QString &encoding) {
	RK_TRACE (APP);
	return 0;
	// TODO
}

QWidget *KatePluginIntegration::createViewBar(KTextEditor::View *view) {
	RK_TRACE (APP);
	return 0;
	// TODO
}

QObject *KatePluginIntegration::pluginView(const QString &name) {
	RK_TRACE (APP);
	return 0;
	// TODO
}

void KatePluginIntegration::addWidgetToViewBar(KTextEditor::View* view, QWidget* bar) {
	RK_TRACE (APP);
	// TODO
}

bool KatePluginIntegration::closeSplitView(KTextEditor::View* view) {
	RK_TRACE (APP);
	return false;
	// TODO
}

bool KatePluginIntegration::closeView(KTextEditor::View* view) {
	RK_TRACE (APP);

	RKMDIWindow *w = findWindowForView(view);
	if (w) return RKWorkplace::mainWorkplace()->closeWindow(w);
	return false;
}

void KatePluginIntegration::deleteViewBar(KTextEditor::View* view) {
	RK_TRACE (APP);
	// TODO
}

bool KatePluginIntegration::hideToolView(QWidget* widget) {
	RK_TRACE (APP);
	return false;
	// TODO
}

void KatePluginIntegration::hideViewBar(KTextEditor::View* view) {
	RK_TRACE (APP);
	// TODO
}

bool KatePluginIntegration::moveToolView(QWidget* widget, KTextEditor::MainWindow::ToolViewPosition pos) {
	RK_TRACE (APP);
	// TODO
	return false;
}

void KatePluginIntegration::showViewBar(KTextEditor::View* view) {
	RK_TRACE (APP);
	// TODO
}

void KatePluginIntegration::splitView(Qt::Orientation orientation) {
	RK_TRACE (APP);
	// TODO
}

bool KatePluginIntegration::viewsInSameSplitView(KTextEditor::View* view1, KTextEditor::View* view2) {
	RK_TRACE (APP);
	// TODO
	return false;
}

///  BEGIN  KTextEditor::Application interface

KatePluginIntegration2::KatePluginIntegration2(KatePluginIntegration *_buddy) : QObject (_buddy) {
	RK_TRACE (APP);
	buddy = _buddy;
	app = new KTextEditor::Application (this);
}

KatePluginIntegration2::~KatePluginIntegration2() {
	RK_TRACE (APP);
}

QList<KTextEditor::MainWindow *> KatePluginIntegration2::mainWindows() {
	RK_TRACE (APP);
	QList<KTextEditor::MainWindow *> ret;
	ret.append (buddy->main);
	return ret;
}

KTextEditor::MainWindow *KatePluginIntegration2::activeMainWindow() {
	RK_TRACE (APP);
	return buddy->main;
}

QList<KTextEditor::Document *> KatePluginIntegration2::documents() {
	RK_TRACE (APP);

	QList<RKMDIWindow*> w = RKWorkplace::mainWorkplace()->getObjectList(RKMDIWindow::CommandEditorWindow);
	QList<KTextEditor::Document*> ret;
	for (int i = 0; i < w.size (); ++i) {
		KTextEditor::View *v = static_cast<RKCommandEditorWindow*>(w[i])->getView(); 
		if (v) ret.append (v->document());
	}
	return ret;
}

KTextEditor::Document *KatePluginIntegration2::findUrl(const QUrl &url) {
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

KTextEditor::Document *KatePluginIntegration2::openUrl(const QUrl &url, const QString &encoding) {
	RK_TRACE (APP);

	KTextEditor::View *v = buddy->openUrl(url, encoding);
	if (v) return v->document();
	return 0;
}

bool KatePluginIntegration2::closeDocument(KTextEditor::Document *document) {
	RK_TRACE (APP);

	RKMDIWindow *w = findWindowForDocument(document);
	if (w) return RKWorkplace::mainWorkplace()->closeWindow(w); // NOTE: Closes only a single view of the document
	return false;
}

bool KatePluginIntegration2::closeDocuments(const QList<KTextEditor::Document *> &documents) {
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

KTextEditor::Plugin *KatePluginIntegration2::plugin(const QString &name) {
	RK_TRACE (APP);
	return 0;
	// TODO
}

#include "katepluginintegration.moc"
