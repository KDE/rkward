/*
rkqwebenginewidget - This file is part of the RKWard project. Created: Sat Feb 07 2026
SPDX-FileCopyrightText: 2026 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkqwebenginewidget.h"

#include <QBuffer>
#include <QDir>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMimeDatabase>
#include <QPrintDialog>
#include <QWebEngineFindTextResult>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineView>
#include <QWheelEvent>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>

#include "../misc/rkfindbar.h"
#include "../rkward.h"
#include "../settings/rksettingsmoduler.h"
#include "rkhtmlwindow.h"
#include "rkworkplace.h"

#include "../debug.h"

// TODO: remove dupe
static QUrl restorableUrl(const QUrl &url) {
	return QUrl(url.url().replace(RKSettingsModuleR::helpBaseUrl(), QLatin1String("rkward://RHELPBASE")));
}

class RKWebPage : public QWebEnginePage {
	Q_OBJECT
  public:
	explicit RKWebPage(RKHTMLWindow *window) : QWebEnginePage(window) {
		RK_TRACE(APP);
		RKWebPage::window = window;
		direct_load = false;
		settings()->setFontFamily(QWebEngineSettings::StandardFont, QFontDatabase::systemFont(QFontDatabase::GeneralFont).family());
		settings()->setFontFamily(QWebEngineSettings::FixedFont, QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
	}

	void load(const QUrl &url) {
		RK_TRACE(APP);
		direct_load = true;
		QWebEnginePage::load(url);
	}

	void setHtmlWrapper(const QString &html, const QUrl &baseurl) {
		direct_load = true;
		setHtml(html, baseurl);
	}
	bool supportsContentType(const QString &name) {
		if (name.startsWith(QLatin1String("text"))) return true;
		return false;
	}
	void downloadUrl(const QUrl &url) {
		download(url);
	}
	void setScrollPosition(const QPoint &point) {
		RK_DEBUG(APP, DL_DEBUG, "scrolling to %d, %d", point.x(), point.y());
		runJavaScript(QStringLiteral("window.scrollTo(%1, %2);").arg(point.x()).arg(point.y()));
	}
	void setScrollPositionWhenDone(const QPoint &pos) {
		QMetaObject::Connection *const connection = new QMetaObject::Connection;
		*connection = connect(this, &QWebEnginePage::loadFinished, [this, pos, connection]() {
			QObject::disconnect(*connection);
			delete connection;
			setScrollPosition(pos);
		});
	}

  Q_SIGNALS:
	void pageInternalNavigation(const QUrl &url);

  protected:
	bool acceptNavigationRequest(const QUrl &navurl, QWebEnginePage::NavigationType type, bool is_main_frame) override {
		QUrl cururl(url());
		Q_UNUSED(type);

		RK_TRACE(APP);
		RK_DEBUG(APP, DL_DEBUG, "Navigation request to %s", qPrintable(navurl.toString()));
		if (direct_load && (is_main_frame)) {
			direct_load = false;
			return true;
		}

		if (opened_as_new) {
			RK_ASSERT(opened_as_new == this);
			RK_ASSERT(!window);
			RKWorkplace::mainWorkplace()->openAnyUrl(restorableUrl(navurl));
			opened_as_new = nullptr;
			if (!window) deleteLater(); // this page was _not_ reused
			return false;
		}
		RK_ASSERT(window);

		if (!is_main_frame) {
			if (navurl.isLocalFile() && supportsContentType(QMimeDatabase().mimeTypeForUrl(navurl).name())) return true;
		}

		if (cururl.matches(navurl, QUrl::NormalizePathSegments | QUrl::StripTrailingSlash)) {
			RK_DEBUG(APP, DL_DEBUG, "Page internal navigation request from %s to %s", qPrintable(cururl.toString()), qPrintable(navurl.toString()));
			Q_EMIT pageInternalNavigation(navurl);
			return true;
		}

		window->openURL(navurl);
		return false;
	}

	QWebEnginePage *createWindow(QWebEnginePage::WebWindowType) override {
		RK_TRACE(APP);
		RKWebPage *ret = new RKWebPage(nullptr);
		opened_as_new = ret; // Don't actually create a full window, until we know which URL we're talking about.
		// sigh: acceptNavigationRequest() does  not get called on the new page...
		QMetaObject::Connection *const connection = new QMetaObject::Connection;
		*connection = connect(ret, &RKWebPage::loadStarted, [ret, connection
#ifdef _MSC_VER
		                                                     ,
		                                                     this // capturing "this" makes MSVC happy
#endif
		]() {
			QObject::disconnect(*connection);
			delete connection;
			ret->acceptNavigationRequest(ret->url(), QWebEnginePage::NavigationTypeLinkClicked, true);
		});
		return (ret);
	}

	friend class RKQWebEngineWidget;
	RKHTMLWindow *window;
	bool direct_load;
	static RKWebPage *opened_as_new;
};
RKWebPage *RKWebPage::opened_as_new = nullptr;

class RKWebView : public QWebEngineView {
  public:
	explicit RKWebView(QWidget *parent) : QWebEngineView(parent) {};
	void print(QPrinter *printer) {
		if (!page()) return;
		QWebEngineView::forPage(page())->print(printer);
	};

  protected:
	bool eventFilter(QObject *, QEvent *event) override {
		if (event->type() == QEvent::Wheel) {
			QWheelEvent *we = static_cast<QWheelEvent *>(event);
			if (we->modifiers() & Qt::ControlModifier) {
				setZoomFactor(zoomFactor() + we->angleDelta().y() / 1200.0);
				return true;
			}
		}
		return false;
	}
	void childEvent(QChildEvent *event) override {
		if (event->type() == QChildEvent::ChildAdded) {
			event->child()->installEventFilter(this);
		}
	}
	// NOTE: Code below won't work, due to https://bugreports.qt.io/browse/QTBUG-43602
	/*	void wheelEvent (QWheelEvent *event) override {
	        [handle zooming]
	    } */
};

class RKWebEngineKIOForwarder : public QWebEngineUrlSchemeHandler {
  public:
	explicit RKWebEngineKIOForwarder(QObject *parent) : QWebEngineUrlSchemeHandler(parent) {}
	void requestStarted(QWebEngineUrlRequestJob *request) override {
		RK_DEBUG(APP, DL_DEBUG, "new KIO request to %s", qPrintable(request->requestUrl().url()));
		KIO::StoredTransferJob *job = KIO::storedGet(request->requestUrl(), KIO::NoReload, KIO::HideProgressInfo);
		connect(job, &KIO::StoredTransferJob::result, this, [this, job]() { kioJobFinished(job); });
		jobs.insert(job, request);
	}

  private:
	void kioJobFinished(KIO::StoredTransferJob *job) {
		QWebEngineUrlRequestJob *request = jobs.take(job);
		if (!request) {
			return;
		}
		if (job->error()) {
			request->fail(QWebEngineUrlRequestJob::UrlInvalid); // TODO
			return;
		}
		QBuffer *buf = new QBuffer(request);
		buf->setData(job->data());
		request->reply(QMimeDatabase().mimeTypeForData(job->data()).name().toUtf8(), buf);
	}
	QMap<KIO::StoredTransferJob *, QPointer<QWebEngineUrlRequestJob>> jobs;
};

RKQWebEngineWidget::RKQWebEngineWidget(RKHTMLWindow *parent) : RKHTMLViewer(parent) {
	RK_TRACE(APP);

	if (RKWebPage::opened_as_new) {
		page = RKWebPage::opened_as_new;
		RKWebPage::opened_as_new = nullptr;
		RKWebPage::opened_as_new->window = parent;
	} else {
		page = new RKWebPage(parent);
	}
}

QWidget *RKQWebEngineWidget::createWidget(QWidget *parent) {
	RK_TRACE(APP);

	// We keep our own history.
	page->action(RKWebPage::Back)->setVisible(false);
	page->action(RKWebPage::Forward)->setVisible(false);
	// For now we won't bother with this one: Does not behave well, in particular (but not only) WRT to rkward://-links
	page->action(RKWebPage::DownloadLinkToDisk)->setVisible(false);
	// Not really useful for us, and cannot easily be made to work, as all new pages go through RKWorkplace::openAnyUrl()
	page->action(RKWebPage::ViewSource)->setVisible(false);
	// Well, technically, all our windows are tabs, but we're calling them "window".
	// At any rate, we don't need both "open link in new tab" and "open link in new window".
	page->action(RKWebPage::OpenLinkInNewTab)->setVisible(false);

	connect(page, &RKWebPage::printRequested, this, &RKQWebEngineWidget::print);
	connect(page->profile(), &QWebEngineProfile::downloadRequested, this, [this](QWebEngineDownloadRequest *item) {
		QString defpath;
		defpath = QDir(item->downloadDirectory()).absoluteFilePath(item->downloadFileName());
		QString path = QFileDialog::getSaveFileName(window, i18n("Save as"), defpath);
		if (path.isEmpty()) return;
		QFileInfo fi(path);
		item->setDownloadDirectory(fi.absolutePath());
		item->setDownloadFileName(fi.fileName());
		item->accept();
	});
	connect(page, &RKWebPage::loadFinished, this, [this]() {
		Q_EMIT loadFinished();
	});

	RK_ASSERT(!view);
	view = new QWebEngineView(page, parent);
	connect(view, &QWebEngineView::selectionChanged, this, [this]() {
		Q_EMIT selectionChanged(view->hasSelection());
	});
	return view;
}

void RKQWebEngineWidget::reload() {
	RK_TRACE(APP);
	auto a = page->action(RKWebPage::Reload);
	RK_ASSERT(a);
	if (a) a->trigger();
}

QUrl RKQWebEngineWidget::url() const {
	RK_TRACE(APP);
	return page->url();
}

void RKQWebEngineWidget::load(const QUrl &url) {
	RK_TRACE(APP);
	// TODO: We seem to be loading start page multiple times?
	RK_DEBUG(APP, DL_WARNING, qPrintable(url.url()));
	page->load(url);
}

QString RKQWebEngineWidget::selectedText() const {
	RK_TRACE(APP);
	if (!view) return QString();
	return view->selectedText();
}

void RKQWebEngineWidget::exportPage() {
	RK_TRACE(APP);
	page->downloadUrl(page->url());
}

QPoint RKQWebEngineWidget::scrollPosition() const {
	RK_TRACE(APP);
	return page->scrollPosition().toPoint();
}

void RKQWebEngineWidget::setScrollPosition(const QPoint &pos, bool wait_for_load) {
	RK_TRACE(APP);
	if (wait_for_load) {
		page->setScrollPositionWhenDone(pos);
	} else {
		page->setScrollPosition(pos);
	}
}

void RKQWebEngineWidget::zoomIn() {
	RK_TRACE(APP);
	if (view) view->setZoomFactor(view->zoomFactor() * 1.1);
}

void RKQWebEngineWidget::zoomOut() {
	RK_TRACE(APP);
	if (view) view->setZoomFactor(view->zoomFactor() / 1.1);
}

bool RKQWebEngineWidget::supportsContentType(const QString &mimename) {
	RK_TRACE(APP);
	return page->supportsContentType(mimename);
}

void RKQWebEngineWidget::print() {
	RK_TRACE(APP);
	// NOTE: taken from kwebkitpart, with small mods
	// Make it non-modal, in case a redirection deletes the part
	QPointer<QPrintDialog> dlg(new QPrintDialog(view));
	if (dlg->exec() == QPrintDialog::Accepted) {
		view->print(dlg->printer());
	}
	delete dlg;
}

void RKQWebEngineWidget::installPersistentJS(const QString &script, const QString &id) {
	RK_TRACE(APP);

	auto p = QWebEngineProfile::defaultProfile();
	QWebEngineScript we_script;
	const auto scripts = p->scripts()->find(id);
	for (const auto &script : scripts) {
		p->scripts()->remove(script); // remove any existing variant of the script. It might have been created for the wrong theme.
	}
	we_script.setName(id);
	we_script.setInjectionPoint(QWebEngineScript::DocumentReady);
	we_script.setSourceCode(script);
	p->scripts()->insert(we_script);
}

void RKQWebEngineWidget::runJS(const QString &script, std::function<void(const QVariant &)> callback) {
	RK_TRACE(APP);
	page->runJavaScript(script, callback);
}

void RKQWebEngineWidget::setHTML(const QString &html, const QUrl &url) {
	RK_TRACE(APP);
	page->setHtmlWrapper(html, url);
}

bool RKQWebEngineWidget::installHelpProtocolHandler() {
	RK_TRACE(APP);

	if (!QWebEngineProfile::defaultProfile()->urlSchemeHandler("help")) {
		QWebEngineProfile::defaultProfile()->installUrlSchemeHandler("help", new RKWebEngineKIOForwarder(RKWardMainWindow::getMain()));
	}
	return true;
}

void RKQWebEngineWidget::findRequest(const QString &text, bool backwards, RKFindBar *findbar, bool *found) {
	RK_TRACE(APP);

	// QWebEngine does not offer highlight all
	*found = true; // real result is not available, synchronously
	QWebEnginePage::FindFlags flags;
	if (backwards) flags |= QWebEnginePage::FindBackward;
	if (findbar->isOptionSet(RKFindBar::MatchCase)) flags |= QWebEnginePage::FindCaseSensitively;
	page->findText(text, flags, [this, findbar](QWebEngineFindTextResult result) {
		if (result.numberOfMatches() == 0) {
			findbar->indicateSearchFail();
		}
	});
}

QMenu *RKQWebEngineWidget::createContextMenu(const QPoint &clickpos) {
	RK_TRACE(APP);
	return (QWebEngineView::forPage(page)->createStandardContextMenu());
}

#include "rkqwebenginewidget.moc"
