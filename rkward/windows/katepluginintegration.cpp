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
#include "rkcommandeditorwindow.h"

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

QWidget * KatePluginIntegration::createToolView (KTextEditor::Plugin *plugin, const QString &identifier, KTextEditor::MainWindow::ToolViewPosition pos, const QIcon &icon, const QString &text) {
	RK_TRACE (APP);
	RKDebug (APP, DL_ERROR, "createToolView");

	QWidget *dummy = new QWidget();
	dummy->setLayout (new QVBoxLayout ());
//	dummy->show();
	return dummy;
	// TODO
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

	RKMDIWindow *w = RKWorkplace::mainWorkplace()->activeWindow (RKMDIWindow::AnyWindowState);
	if (w && w->isType (RKMDIWindow::CommandEditorWindow)) {
		return static_cast<RKCommandEditorWindow*>(w)->getView();
	}
	return 0;
}

KTextEditor::View *KatePluginIntegration::activateView(KTextEditor::Document *document) {
	RK_TRACE (APP);
	return 0;
	// TODO
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
	return false;
	// TODO
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
	return 0;
	// TODO
}

KTextEditor::Document *KatePluginIntegration2::openUrl(const QUrl &url, const QString &encoding) {
	RK_TRACE (APP);
	return 0;
	// TODO
}

bool KatePluginIntegration2::closeDocument(KTextEditor::Document *document) {
	RK_TRACE (APP);
	return false;
	// TODO
}

bool KatePluginIntegration2::closeDocuments(const QList<KTextEditor::Document *> &documents) {
	RK_TRACE (APP);
	return false;
	// TODO
}

KTextEditor::Plugin *KatePluginIntegration2::plugin(const QString &name) {
	RK_TRACE (APP);
	return 0;
	// TODO
}
