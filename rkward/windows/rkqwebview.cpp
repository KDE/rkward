/*
rkqwebview - This file is part of the RKWard project. Created: Sun Feb 08 2026
SPDX-FileCopyrightText: 2026 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkqwebview.h"

#include <QMenu>
#include <QQuickItem>
#include <QQuickWidget>

#include "rkhtmlwindow.h"

#include "../debug.h"

RKQWebView::RKQWebView(RKHTMLWindow *parent) : RKHTMLViewer(parent) {
	RK_TRACE(APP);
}

QWidget *RKQWebView::createWidget(QWidget *parent) {
	RK_TRACE(APP);
	view = new QQuickWidget(parent);
	view->setSource(QUrl(QStringLiteral("qrc:/qml/rkqwebview.qml")));
	view->setResizeMode(QQuickWidget::SizeRootObjectToView);
	return view;
}

QUrl RKQWebView::url() const {
	RK_TRACE(APP);
	return QUrl();
}

void RKQWebView::load(const QUrl &url) {
	RK_TRACE(APP);
	auto wv = view->rootObject()->findChild<QObject *>("webView");
	RK_ASSERT(wv);
	if (!wv) return;
	wv->setProperty("url", url);
}

void RKQWebView::print() {
	RK_TRACE(APP);
}

void RKQWebView::installPersistentJS(const QString &script, const QString &id) {
	RK_TRACE(APP);
}

void RKQWebView::runJS(const QString &script, std::function<void(const QVariant &)> callback) {
	RK_TRACE(APP);
}

void RKQWebView::setHTML(const QString &html, const QUrl &url) {
	RK_TRACE(APP);
}

bool RKQWebView::installHelpProtocolHandler() {
	RK_TRACE(APP);
	return false;
}

void RKQWebView::findRequest(const QString &text, bool backwards, RKFindBar *findbar, bool *found) {
	RK_TRACE(APP);
}

QMenu *RKQWebView::createContextMenu(const QPoint &clickpos) {
	RK_TRACE(APP);
	return new QMenu();
}

QString RKQWebView::selectedText() const {
	RK_TRACE(APP);

	return QString();
}

void RKQWebView::exportPage() {
	RK_TRACE(APP);
}

QPoint RKQWebView::scrollPosition() const {
	RK_TRACE(APP);
	return QPoint();
}

void RKQWebView::setScrollPosition(const QPoint &pos, bool wait_for_load) {
	RK_TRACE(APP);
}

bool RKQWebView::supportsContentType(const QString &mimename) {
	RK_TRACE(APP);
	return true;
}

void RKQWebView::zoomIn() {
	RK_TRACE(APP);
}

void RKQWebView::zoomOut() {
	RK_TRACE(APP);
}
