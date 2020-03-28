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

#ifndef KATEPLUGININTEGRATION_H
#define KATEPLUGININTEGRATION_H

#include <KTextEditor/MainWindow>
#include <KTextEditor/Application>
#include <KPluginMetaData>
#include <KXMLGUIClient>

#include <QMap>

class KatePluginIntegrationWindow;
class RKMDIWindow;

/** This class provides implementations for the KTextEditor::Application interface.
 *  Note that there is a separate interface KatePluginIntegrationWindow / KTextEditor::MainWindow that serves
 *  as an abstraction of main windows. Even though - for now - we will be creating (detachable) plugins only in
 *  one "main" window, we follow this separation to be on the safe side for future extensions (and also there is
 *  a name-clash in on of the slots (openUrl()), otherwise. */
class KatePluginIntegrationApp : public QObject {
	Q_OBJECT
public:
	KatePluginIntegrationApp(QObject *parent);
	~KatePluginIntegrationApp();
	QObject* loadPlugin(const QString& identifier);
	void unloadPlugin(const QString& identifier);
	KatePluginIntegrationWindow *mainWindow() const { return window; };
private slots:
friend class KatePluginIntegrationWindow;
	void saveConfigAndUnload();
	// These are the implementations of the KTextEditor::Application interface.
	// NOTE that they are not technically overrides, but get invoked via QMetaObject::invokeMethod()
	QList<KTextEditor::MainWindow *> mainWindows();
	KTextEditor::MainWindow *activeMainWindow();
	QList<KTextEditor::Document *> documents();
	KTextEditor::Document *findUrl(const QUrl &url);
	KTextEditor::Document *openUrl(const QUrl &url, const QString &encoding = QString());
	bool closeDocument(KTextEditor::Document *document);
	bool closeDocuments(const QList<KTextEditor::Document *> &documents);
	KTextEditor::Plugin *plugin(const QString &name);
private:
friend class RKSettingsModuleKatePlugins;
	KatePluginIntegrationWindow *window;  // For now, only one main window
	KTextEditor::Application *app;
/** Provides a hidden dummy view (created on the fly as needed), for plugins that assume there is always at least one view/document around. */
	KTextEditor::View *dummyView();
	KTextEditor::View *dummy_view;

	struct PluginInfo {
		KPluginMetaData data;
		KTextEditor::Plugin *plugin;
	};
	QMap<QString, PluginInfo> known_plugins;
	QString idForPlugin(const KTextEditor::Plugin *plugin) const;
	QString idForPlugin(const KPluginMetaData &plugin) const;
};

class KatePluginIntegrationWindow : public QObject, public KXMLGUIClient {
	Q_OBJECT
public:
	KatePluginIntegrationWindow(KatePluginIntegrationApp *parent);
	KTextEditor::MainWindow *mainWindow() const { return main; };
private slots:
	// These are the implementations of the KTextEditor::MainWindow interface.
	// NOTE that they are not technically overrides, but get invoked via QMetaObject::invokeMethod()
	QWidget *createToolView(KTextEditor::Plugin *plugin, const QString &identifier, KTextEditor::MainWindow::ToolViewPosition pos, const QIcon &icon, const QString &text);
	KXMLGUIFactory *guiFactory();
	QWidget *window();
	QList<KTextEditor::View *> views();
	KTextEditor::View *activeView();
	KTextEditor::View *activateView(KTextEditor::Document *document);
	KTextEditor::View *openUrl(const QUrl &url, const QString &encoding = QString());
	bool closeView(KTextEditor::View *view);
	void splitView(Qt::Orientation orientation);
	bool closeSplitView(KTextEditor::View *view);
	bool viewsInSameSplitView(KTextEditor::View *view1, KTextEditor::View *view2);
	bool moveToolView(QWidget *widget, KTextEditor::MainWindow::ToolViewPosition pos);
	bool showToolView(QWidget *widget);
	bool hideToolView(QWidget *widget);
	QObject *pluginView(const QString &name);

/* Apparently, these are truely optional, so let's disable them for the time being
	QWidget *createViewBar(KTextEditor::View *view);
	void deleteViewBar(KTextEditor::View *view);
	void showViewBar(KTextEditor::View *view);
	void hideViewBar(KTextEditor::View *view);
	void addWidgetToViewBar(KTextEditor::View *view, QWidget *bar); */
private:
friend class KatePluginIntegrationApp;
	KTextEditor::MainWindow *main;
	QObject* createPluginView(KTextEditor::Plugin* plugin);
	struct PluginResources {
		PluginResources() : view(0) {};
		QObject *view;
		QList<KXMLGUIClient*> clients;
		QList<RKMDIWindow*> windows;
	};
	QHash<KTextEditor::Plugin*, PluginResources> plugin_resources;

	KatePluginIntegrationApp *app;
private slots:
	void catchXMLGUIClientsHack(KXMLGUIClient* client);
	void activeWindowChanged(RKMDIWindow *window);
private:
	KTextEditor::Plugin* active_plugin;
	void fixUpPluginUI(const QString &id, const PluginResources &resources);
};

#endif
