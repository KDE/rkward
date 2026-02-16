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

#include "../misc/rkfindbar.h"
#include "rkhtmlwindow.h"
#include "rkworkplace.h"

#include "../debug.h"

RKQWebView::RKQWebView(RKHTMLWindow *parent) : RKHTMLViewer(parent), view(nullptr) {
	RK_TRACE(APP);
}

QUrl RKQWebView::currentAcceptedUrl() const {
	return webView()->property("acceptedUrl").toUrl();
}

void RKQWebView::onUrlChanged(const QUrl &_url, const QString &error, int status) {
	// WebView status enum: 0: LoadStarted, 2: LoadFinished, 3: Error.
	// I do not actually know, what status==1 is, but it does get sent in some cases. I assume it's "aborted"
	RK_DEBUG(APP, DL_WARNING, "webview %p url changed to %s status %d, msg %s", this, qPrintable(_url.toString()), status, qPrintable(error));
	bool new_window = false;
	Q_EMIT navigationRequest(currentAcceptedUrl(), _url, new_window);
	// we may be redirected via the above signal
	if (_url != url()) {
		RK_DEBUG(APP, DL_DEBUG, "redirecting %s to %s", qPrintable(_url.toString()), qPrintable(url().toString()));
	}
	if (status == 0 && _url.scheme() == u"rkward"_s) {
		// Some QWebView plugins will also trigger openRKWardURl() via QDesktopServices::setUrlHandler()
		// Others won't. In either case, we've handled it from here.
		RKWorkplace::mainWorkplace()->suppressRKWardUrlHandling(_url);
	}
}

void RKQWebView::onLoadFinished(const QUrl &url) {
	RK_TRACE(APP);
	RK_ASSERT(currentAcceptedUrl() == url);
	for (const auto &script : std::as_const(persistentScripts)) {
		RKHTMLViewer::runJS(script);
	}
	Q_EMIT loadFinished();
}

QWidget *RKQWebView::createWidget() {
	RK_TRACE(APP);
	view = new QQuickWidget();
	view->setResizeMode(QQuickWidget::SizeRootObjectToView);
	view->setSource(QUrl(QStringLiteral("qrc:/qml/rkqwebview.qml")));
	connect(webView(), SIGNAL(pageUrlChanged(const QUrl &, const QString &, int)), this, SLOT(onUrlChanged(const QUrl &, const QString &, int)));
	connect(webView(), SIGNAL(loadFinished(const QUrl &)), this, SLOT(onLoadFinished(const QUrl &)));
	connect(webView(), SIGNAL(runJSResult(const QVariant &)), this, SLOT(onRunJSResult(const QVariant &)));
	/* TODO: we may need to inject a script along the lines below for handling target=new
	    and page internal navigation?
	RKHTMLViewer::runJS(u"document.addEventListener('click', e => {"
	    "  const origin = e.target.closest('a');"
	    "  if (origin) alert(origin.href);"
	    "})\n"_s); */
	return view;
}

QObject *RKQWebView::webView() const {
	if (!view) {
		// could happen (happened in the past) due to premature calls to this function.
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
	// TODO: Doesn't work. Why?
	RKHTMLViewer::runJS(u"window.print()"_s);
}

void RKQWebView::installPersistentJS(const QString &script, const QString &id) {
	RK_TRACE(APP);
	persistentScripts[id] = script;
}

void RKQWebView::runJS(const QString &script, std::function<void(const QVariant &)> callback) {
	RK_TRACE(APP);
	RK_ASSERT(runjs_callback == nullptr); // no nested calls, please!
	runjs_callback = callback;
	QMetaObject::invokeMethod(webView(), "runJSWrapper", Q_ARG(QString, script));
}

void RKQWebView::onRunJSResult(const QVariant &result) {
	RK_TRACE(APP);
	if (runjs_callback != nullptr) runjs_callback(result);
	runjs_callback = std::function<void(const QVariant &)>();
}

void RKQWebView::setHTML(const QString &html, const QUrl &_url) {
	RK_TRACE(APP);
	QMetaObject::invokeMethod(webView(), "loadHtml", Q_ARG(QString, html), Q_ARG(QUrl, _url));
}

bool RKQWebView::installHelpProtocolHandler() {
	RK_TRACE(APP);
	return false;
}

void RKQWebView::findRequest(const QString &text, bool backwards, RKFindBar *findbar, bool *found) {
	RK_TRACE(APP);

	*found = true; // real result is not available, synchronously
	auto _txt = text;
	// NOTE: window.find() is marked as not standardized at the time of this writing!
	QString script = u"window.find('%1', %2, %3, true)"_s.arg(_txt.replace(u"'"_s, u"\\'"_s),
	                                                          (findbar->isOptionSet(RKFindBar::MatchCase) ? u"true"_s : u"false"_s),
	                                                          (backwards ? u"true"_s : u"false"_s));
	runJS(script, [findbar](const QVariant &res) {
		if (!res.toBool()) findbar->indicateSearchFail();
	});
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
