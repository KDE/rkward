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

class KatePluginIntegration : public QObject, public KXMLGUIClient {
Q_OBJECT
public:
	KatePluginIntegration(QObject *parent);
	KTextEditor::MainWindow *mainWindow() const { return main; };
	QObject* loadPlugin(const QString& identifier);
private slots:
	// These are the implementations of the KTextEditor::MainWindow interface.
	// NOTE that they  are not technically overrides, but get invoked via QMetaObject::invokeMethod()
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
	QWidget *createViewBar(KTextEditor::View *view);
	void deleteViewBar(KTextEditor::View *view);
	void addWidgetToViewBar(KTextEditor::View *view, QWidget *bar);
	void showViewBar(KTextEditor::View *view);
	void hideViewBar(KTextEditor::View *view);
	bool moveToolView(QWidget *widget, KTextEditor::MainWindow::ToolViewPosition pos);
	bool showToolView(QWidget *widget);
	bool hideToolView(QWidget *widget);
	QObject *pluginView(const QString &name);
private:
friend class KatePluginIntegration2;
	KTextEditor::MainWindow *main;
/*
	struct ActivePlugin {
		QWidget *inner;
		RKMDIWindow *outer;
	}; */
	QMap<QString, KPluginMetaData> known_plugins;
	QString idForPlugin(const KPluginMetaData &plugin) const;
};

/** This class provides implementations for the KTextEditor::Application interface. I'd love to merge it into
 *  KatePluginIntegration, but this is not possible due to a name clash in one of the slots (openUrl()). */
class KatePluginIntegration2 : public QObject {
	Q_OBJECT
public:
	KatePluginIntegration2(KatePluginIntegration *_buddy);
	~KatePluginIntegration2();
private slots:
	// And these are the implementations of the KTextEditor::Application interface.
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
friend class KatePluginIntegration;
	KatePluginIntegration *buddy;
	KTextEditor::Application *app;
};

#endif
