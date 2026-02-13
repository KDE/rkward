/*
rkqwebview - This file is part of the RKWard project. Created: Sun Feb 08 2026
SPDX-FileCopyrightText: 2026 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKQWEBVIEW_H
#define RKQWEBVIEW_H

#include <QPointer>

#include "rkhtmlviewer.h"

class QQuickWidget;
class QQuickItem;

class RKQWebView : public RKHTMLViewer {
	Q_OBJECT
  public:
	QWidget *createWidget(QWidget *parent) override;
	QUrl url() const override;
	void load(const QUrl &url) override;
	void print() override;
	void installPersistentJS(const QString &script, const QString &id) override;
	void runJS(const QString &script, std::function<void(const QVariant &)> callback) override;
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
  private Q_SLOTS:
	void onPageLoad(const QUrl &url, const QString &error, int status);
	QObject *webView() const;
};

#endif
