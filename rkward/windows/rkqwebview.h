/*
rkqwebview - This file is part of the RKWard project. Created: Sun Feb 08 2026
SPDX-FileCopyrightText: 2026 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKQWEBVIEW_H
#define RKQWEBVIEW_H

#include <QMap>
#include <QPointer>

#include "rkhtmlviewer.h"

class QQuickWidget;
class QQuickItem;

/** Interface abstraction for QWebView. Note that QWebView is essentially QML only until Qt 6.10. Once
 *  we can assume Qt 6.11,some simplifications will be possible. */
class RKQWebView : public RKHTMLViewer {
	Q_OBJECT
  public:
	QWidget *createWidget() override;
	QUrl url() const override;
	void load(const QUrl &url) override;
	void print() override;
	void installPersistentJS(const QString &script, const QString &id) override;
	void runJS(const QString &script, std::function<void(const QVariant &)> callback = std::function<void(const QVariant &)>()) override;
	void setHTML(const QString &html, const QUrl &url) override;
	bool installHelpProtocolHandler() override;
	void findRequest(const QString &text, bool backwards, RKFindBar *findbar, bool *found) override;
	QMenu *createContextMenu(const QPoint &clickpos) override;
	QString selectedText() const override;
	void exportPage() override;
	QPoint scrollPosition() const override;
	void setScrollPosition(const QPoint &pos, bool wait_for_load) override;
	bool supportsContentType(const QString &mimename) override;
	void zoomIn() override;
	void zoomOut() override;

  private:
	friend class RKHTMLViewer;
	RKQWebView(RKHTMLWindow *parent);
	QPointer<QQuickWidget> view;
	QUrl currentAcceptedUrl() const;
	QMap<QString, QString> persistentScripts;
	std::function<void(const QVariant &)> runjs_callback;
  private Q_SLOTS:
	void onUrlChanged(const QUrl &url, const QString &error, int status);
	void onLoadFinished(const QUrl &url);
	void onRunJSResult(const QVariant &result);
	QObject *webView() const;
};

#endif
