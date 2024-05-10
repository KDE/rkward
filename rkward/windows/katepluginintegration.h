/*
katepluginintegration - This file is part of the RKWard project. Created: Mon Jun 12 2017
SPDX-FileCopyrightText: 2017-2023 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KATEPLUGININTEGRATION_H
#define KATEPLUGININTEGRATION_H

#include <KTextEditor/MainWindow>
#include <KTextEditor/Application>
#include <KPluginMetaData>
#include <KXMLGUIClient>

#include <QMap>
#include <QWidget>

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
	explicit KatePluginIntegrationApp(QObject *parent);
	~KatePluginIntegrationApp();
	QObject* loadPlugin(const QString& identifier);
	/** Loads the given plugins, *and* unloads all others. */
	void unloadPlugin(const QString& identifier);
	void loadPlugins(const QStringList &plugins);
	KatePluginIntegrationWindow *mainWindow() const { return window; };
	int knownPluginCount() const { return known_plugins.size(); };
private Q_SLOTS:
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

class KatePluginWindow;
class KatePluginIntegrationWindow : public QObject, public KXMLGUIClient {
	Q_OBJECT
public:
	explicit KatePluginIntegrationWindow(KatePluginIntegrationApp *parent);
	~KatePluginIntegrationWindow();
	KTextEditor::MainWindow *mainWindow() const { return main; };
	KXMLGUIClient* persistentGuiClient() { return this; }
	KXMLGUIClient* dynamicGuiClient() const { return dynamic_actions_client; }
private Q_SLOTS:
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

	/* Apparently, these are truely optional. We provide dummy implementations anyway to
	 * get a better signal to noise ratio in warnings.
	 *
	 * "Viewbar" is the area containing line number, etc. If not created by the main window,
	 * each view gets equipped with its own bar. That may actually be preferrable, as
	 * our global bottom bar is quite crowded, already. */
	QWidget *createViewBar(KTextEditor::View *view);
	void deleteViewBar(KTextEditor::View *view);
	void showViewBar(KTextEditor::View *view);
	void hideViewBar(KTextEditor::View *view);
	void addWidgetToViewBar(KTextEditor::View *view, QWidget *bar);

// New in Kate 2023-07, not yet formalized in KTextEditor
	QWidget *toolviewForName(const QString &toolviewName);
	bool showMessage(const QVariantMap &map);
	bool addWidget(QWidget *widget);
	void activateWidget(QWidget *widget);
	QWidgetList widgets();
	void insertWidgetInStatusbar(QWidget *widget);
private:
friend class KatePluginIntegrationApp;
	KTextEditor::MainWindow *main;
	QObject* createPluginView(KTextEditor::Plugin* plugin);
	struct PluginResources {
		PluginResources() : view(nullptr) {};
		QObject *view;
		QList<KXMLGUIClient*> clients;
		QList<KatePluginWindow*> windows;
	};
	QHash<QObject*, PluginResources> plugin_resources;

	KatePluginIntegrationApp *app;
	KXMLGUIClient *dynamic_actions_client;
private Q_SLOTS:
	void catchXMLGUIClientsHack(KXMLGUIClient* client);
	void activeWindowChanged(RKMDIWindow *window);
private:
	KTextEditor::Plugin* active_plugin;
	void fixUpPluginUI(const QString &id, const PluginResources &resources);
};

#endif
