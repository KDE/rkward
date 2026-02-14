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

RKQWebView::RKQWebView(RKHTMLWindow *parent) : RKHTMLViewer(parent), view(nullptr) {
	RK_TRACE(APP);
}

void RKQWebView::onPageLoad(const QUrl &_url, const QString &error, int status) {
	RK_DEBUG(APP, DL_WARNING, "loading %s %d", qPrintable(_url.toString()), status);
	/* TODO: we may need to inject a script similar for handling target=new
	    and page internal navigation?
	RKHTMLViewer::runJS(u"document.addEventListener('click', e => {"
	    "  const origin = e.target.closest('a');"
	    "  if (origin) alert(origin.href);"
	    "})\n"_s); */
	if (status == 0) {
		bool new_window = false;
		Q_EMIT navigationRequest(webView()->property("acceptedUrl").toUrl(), _url, new_window);
		// we may be redirected via the above signal
		if (_url != url()) {
			RK_DEBUG(APP, DL_DEBUG, "redirecting %s to %s", qPrintable(_url.toString()), qPrintable(url().toString()));
		}
	} else if (status == 2) {
		Q_EMIT loadFinished();
	}
}

QWidget *RKQWebView::createWidget() {
	RK_TRACE(APP);
	view = new QQuickWidget();
	view->setSource(QUrl(QStringLiteral("qrc:/qml/rkqwebview.qml")));
	view->setResizeMode(QQuickWidget::SizeRootObjectToView);
	connect(webView(), SIGNAL(loaded(const QUrl &, const QString &, int)), this, SLOT(onPageLoad(const QUrl &, const QString &, int)));
	return view;
}

QObject *RKQWebView::webView() const {
	// TODO: we have some premature calls to this function. Find and fix them
	if (!view) {
		RK_ASSERT(view);
		return nullptr;
	}
	if (view->status() != QQuickWidget::Ready) {
		RK_ASSERT(view->status() == QQuickWidget::Ready);
		return nullptr;
	}
	auto wv = view->rootObject()->findChild<QObject *>("webView");
	RK_ASSERT(wv);
	return wv;
}

QUrl RKQWebView::url() const {
	RK_TRACE(APP);
	auto wv = webView();
	return wv ? wv->property("url").toUrl() : QUrl();
}

void RKQWebView::load(const QUrl &url) {
	RK_TRACE(APP);
	webView()->setProperty("acceptedUrl", url);
	webView()->setProperty("url", url);
}

void RKQWebView::print() {
	RK_TRACE(APP);
	RKHTMLViewer::runJS(u"window.print()"_s);
}

void RKQWebView::installPersistentJS(const QString &script, const QString &id) {
	RK_TRACE(APP);
}

void RKQWebView::runJS(const QString &script, std::function<void(const QVariant &)> callback) {
	RK_TRACE(APP);
	QMetaObject::invokeMethod(webView(), "runJavaScript", Q_ARG(QString, script), Q_ARG(QJSValue, QJSValue()));
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
