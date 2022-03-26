/***************************************************************************
                          rkhtmlwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
    copyright            : (C) 2005-2022 by Thomas Friedrichsmeier
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
#include "rkhtmlwindow.h"

#include <KLocalizedString>
#include <kmessagebox.h>
#include <kparts/plugin.h>
#include <kactioncollection.h>
#include <kdirwatch.h>
#include <kio/job.h>
#include <kservice.h>
#include <kcodecaction.h>

#include <qfileinfo.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qdir.h>
#include <QHBoxLayout>
#include <QHostInfo>
#include <QPrintDialog>
#include <QMenu>
#include <QFileDialog>
#include <QTextCodec>
#include <QFontDatabase>
#include <QTemporaryFile>
#include <QGuiApplication>
#include <QIcon>
#include <QMimeDatabase>
#include <QCheckBox>
#include <QFileDialog>

#include "../rkglobals.h"
#include "../rbackend/rkrinterface.h"
#include "rkhelpsearchwindow.h"
#include "../rkward.h"
#include "../rkconsole.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettings.h"
#include "../settings/rksettingsmoduleoutput.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardactions.h"
#include "../misc/rkstandardicons.h"
#include "../misc/xmlhelper.h"
#include "../misc/rkxmlguisyncer.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkmessagecatalog.h"
#include "../misc/rkfindbar.h"
#include "../misc/rkoutputdirectory.h"
#include "../plugin/rkcomponentmap.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkworkplaceview.h"
#include "../debug.h"

#ifdef NO_QT_WEBENGINE
#	include <QWebFrame>
#	include <QNetworkRequest>
#	include <kwebpage.h>
#	include <kwebview.h>
class RKWebPage : public KWebPage {
#else
#	include <QWebEnginePage>
#	include <QWebEngineView>
#	include <QWebEngineSettings>
#	include <QWebEngineProfile>
class RKWebPage : public QWebEnginePage {
#endif
	Q_OBJECT
public:
#ifdef NO_QT_WEBENGINE
// NOTE: According to an earlier note at this place, KIOIntegration used to be very buggy around KF5 5.9.0. It seem to just work,
//       at 5.44.0, and the symptoms are probably not terrible for earlier versions, so we use it here (allows us to render help:/-pages
//       inside the help window.
	explicit RKWebPage (RKHTMLWindow* window): KWebPage (window, KPartsIntegration | KIOIntegration) {
#else
	explicit RKWebPage (RKHTMLWindow* window): QWebEnginePage (window) {
#endif
		RK_TRACE (APP);
		RKWebPage::window = window;
		direct_load = false;
#ifdef NO_QT_WEBENGINE
		settings ()->setFontFamily (QWebSettings::StandardFont, QFontDatabase::systemFont(QFontDatabase::GeneralFont).family ());
		settings ()->setFontFamily (QWebSettings::FixedFont, QFontDatabase::systemFont(QFontDatabase::FixedFont).family ());
#else
		settings ()->setFontFamily (QWebEngineSettings::StandardFont, QFontDatabase::systemFont(QFontDatabase::GeneralFont).family ());
		settings ()->setFontFamily (QWebEngineSettings::FixedFont, QFontDatabase::systemFont(QFontDatabase::FixedFont).family ());
#endif
	}

	void load (const QUrl& url) {
		RK_TRACE (APP);
		direct_load = true;
#ifdef NO_QT_WEBENGINE
		mainFrame ()->load (url);
#else
		QWebEnginePage::load (url);
#endif
	}

#ifdef NO_QT_WEBENGINE
	QUrl url () {
		return mainFrame ()->url ();
	}
	void setHtml (const QString &html) {
		mainFrame ()->setHtml (html);
	}
	QPointF scrollPosition () const {
		return mainFrame ()->scrollPosition ();
	}
	void setScrollPosition (const QPoint &pos)  {
		mainFrame ()->setScrollPosition (pos);
	}
	void setScrollPositionWhenDone(const QPoint &pos)  {
		QMetaObject::Connection * const connection = new QMetaObject::Connection;
		*connection = connect(this, &QWebPage::loadFinished, [this, pos, connection](){
			QObject::disconnect(*connection);
			delete connection;
			setScrollPosition(pos);
		});
	}
#else
	bool supportsContentType (const QString &name) {
		if (name.startsWith("text")) return true;
#ifdef __GNUC__
#warning TODO
#endif
		return false;
	}
	void downloadUrl (const QUrl& url) {
		download (url);
	}
	void setScrollPosition (const QPoint &point) {
		RK_DEBUG(APP, DL_DEBUG, "scrolling to %d, %d", point.x (), point.y ());
		runJavaScript (QString ("window.scrollTo(%1, %2);").arg (point.x ()).arg(point.y ()));
	}
	void setScrollPositionWhenDone(const QPoint &pos)  {
		QMetaObject::Connection * const connection = new QMetaObject::Connection;
		*connection = connect(this, &QWebEnginePage::loadFinished, [this, pos, connection](){
			QObject::disconnect(*connection);
			delete connection;
			setScrollPosition(pos);
		});
	}
#endif

signals:
	void pageInternalNavigation (const QUrl& url);
protected:
#ifdef NO_QT_WEBENGINE
/** reimplemented to always emit linkClicked() for pages that need special handling (importantly, rkward://-urls). */
	bool acceptNavigationRequest (QWebFrame* frame, const QNetworkRequest& request, QWebPage::NavigationType type) override {
		QUrl navurl = request.url ();
		QUrl cururl (mainFrame ()->url ());
		bool is_main_frame = frame == mainFrame ();
#else
	bool acceptNavigationRequest (const QUrl &navurl, QWebEnginePage::NavigationType type, bool is_main_frame) override {
		QUrl cururl (url ());
#endif
		Q_UNUSED (type);

		RK_TRACE (APP);
		RK_DEBUG (APP, DL_DEBUG, "Navigation request to %s", qPrintable (navurl.toString ()));
		if (direct_load && (is_main_frame)) {
			direct_load = false;
			return true;
		}

		if (RKHTMLWindow::new_window) {
			RK_ASSERT (RKHTMLWindow::new_window == this);
			RK_ASSERT (!window);
			RKWorkplace::mainWorkplace ()->openAnyUrl (navurl);
			RKHTMLWindow::new_window = nullptr;
			if (!window) deleteLater ();  // this page was _not_ reused
			return false;
		}
		RK_ASSERT(window);

		if (!is_main_frame) {
			if (navurl.isLocalFile () && supportsContentType(QMimeDatabase ().mimeTypeForUrl (navurl).name ())) return true;
		}

		if (cururl.matches (navurl, QUrl::NormalizePathSegments | QUrl::StripTrailingSlash)) {
			RK_DEBUG (APP, DL_DEBUG, "Page internal navigation request from %s to %s", qPrintable (cururl.toString ()), qPrintable (navurl.toString ()));
			emit pageInternalNavigation(navurl);
			return true;
		}

		window->openURL (navurl);
		return false;
	}

#ifdef NO_QT_WEBENGINE
/** reimplemented to schedule new window creation for the next page to load */
	QWebPage* createWindow (QWebPage::WebWindowType) override {
#else
	QWebEnginePage* createWindow (QWebEnginePage::WebWindowType) override {
#endif
		RK_TRACE (APP);
		RKWebPage *ret = new RKWebPage (nullptr);
		RKHTMLWindow::new_window = ret; // Don't actually create a full window, until we know which URL we're talking about.
#ifndef NO_QT_WEBENGINE
		// sigh: acceptNavigationRequest() does  not get called on the new page...
		QMetaObject::Connection * const connection = new QMetaObject::Connection;
		*connection = connect (ret, &RKWebPage::loadStarted, [ret, connection]() {
			QObject::disconnect(*connection);
			delete connection;
			ret->acceptNavigationRequest (ret->url (), QWebEnginePage::NavigationTypeLinkClicked, true);
		});
#endif
		return (ret);
	}

friend class RKHTMLWindow;
	RKHTMLWindow *window;
	bool direct_load;
};

#ifdef NO_QT_WEBENGINE
class RKWebView : public KWebView {
public:
	RKWebView (QWidget *parent) : KWebView (parent, false) {};
#else
#include <QWheelEvent>
class RKWebView : public QWebEngineView {
public:
	RKWebView (QWidget *parent) : QWebEngineView (parent) {};
	void print (QPrinter *printer) {
		if (!page ()) return;
		page ()->print (printer, [](bool){});
	};
protected:
	void wheelEvent (QWheelEvent *event) override {
		if (event->modifiers () & Qt::ControlModifier) {
			setZoomFactor (zoomFactor () + event->angleDelta ().y () / 1200.0);
			event->accept ();
		}
		else QWebEngineView::wheelEvent (event);
	}
#endif
};

#ifndef NO_QT_WEBENGINE
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QBuffer>
class RKWebEngineKIOForwarder : public QWebEngineUrlSchemeHandler {
public:
	RKWebEngineKIOForwarder (QObject *parent) : QWebEngineUrlSchemeHandler (parent) {}
	void requestStarted (QWebEngineUrlRequestJob *request) override {
		KIO::StoredTransferJob *job = KIO::storedGet(request->requestUrl (), KIO::NoReload, KIO::HideProgressInfo);
		connect (job, &KIO::StoredTransferJob::result, this, [this, job](){ kioJobFinished(job); });
		jobs.insert (job, request);
	}
private:
	void kioJobFinished (KIO::StoredTransferJob* job) {
		QWebEngineUrlRequestJob* request = jobs.take (job);
		if (!request) {
			return;
		}
		if (job->error ()) {
			request->fail (QWebEngineUrlRequestJob::UrlInvalid);  // TODO
			return;
		}
		QBuffer *buf = new QBuffer (request);
		buf->setData (job->data ());
		request->reply (QMimeDatabase ().mimeTypeForData (job->data ()).name ().toUtf8 (), buf);
	}
	QMap<KIO::StoredTransferJob*,QPointer<QWebEngineUrlRequestJob>> jobs;
};
#endif


RKWebPage* RKHTMLWindow::new_window = nullptr;

RKHTMLWindow::RKHTMLWindow (QWidget *parent, WindowMode mode) : RKMDIWindow (parent, RKMDIWindow::HelpWindow) {
	RK_TRACE (APP);

	current_cache_file = 0;

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	view = new RKWebView (this);
	if (new_window) {
		page = new_window;
		page->window = this;
		new_window = nullptr;
	} else {
		page = new RKWebPage (this);
	}
	view->setPage (page);
	view->setContextMenuPolicy (Qt::CustomContextMenu);
	layout->addWidget (view, 1);
#ifdef NO_QT_WEBENGINE
	findbar = new RKFindBar (this);
#else
	findbar = new RKFindBar (this, true);
	findbar->setPrimaryOptions (QList<QWidget*>() << findbar->getOption (RKFindBar::FindAsYouType) << findbar->getOption (RKFindBar::MatchCase));
	if (!QWebEngineProfile::defaultProfile ()->urlSchemeHandler ("help")) {
		QWebEngineProfile::defaultProfile ()->installUrlSchemeHandler ("help", new RKWebEngineKIOForwarder (RKWardMainWindow::getMain()));
	}
#endif
	layout->addWidget (findbar);
	findbar->hide ();
	connect (findbar, &RKFindBar::findRequest, this, &RKHTMLWindow::findRequest);
	have_highlight = false;

	part = new RKHTMLWindowPart (this);
	setPart (part);
	part->initActions ();
	initializeActivationSignals ();
	part->setSelectable (true);
	setFocusPolicy (Qt::StrongFocus);
	setFocusProxy (view);

	// We have to connect this in order to allow browsing.
	connect (page, &RKWebPage::pageInternalNavigation, this, &RKHTMLWindow::internalNavigation);
#ifdef NO_QT_WEBENGINE
	connect (page, &QWebPage::downloadRequested, [this](const QNetworkRequest &request) { page->downloadUrl (request.url ()); });
#else
	connect (page->profile (), &QWebEngineProfile::downloadRequested, this, [this](QWebEngineDownloadItem* item) {
		QString path = QFileDialog::getSaveFileName (this, i18n ("Save as"), item->path ());
		if (path.isEmpty ()) return;
		item->setPath (path);
		item->accept ();
	});
#endif
	connect (page, &RKWebPage::printRequested, this, &RKHTMLWindow::slotPrint);
	connect (view, &QWidget::customContextMenuRequested, this, &RKHTMLWindow::makeContextMenu);

	current_history_position = -1;
	url_change_is_from_history = false;

	window_mode = Undefined;
	useMode (mode);

	// needed to enable / disable the run selection action
	connect (view, &RKWebView::selectionChanged, this, &RKHTMLWindow::selectionChanged);
	selectionChanged ();
}

RKHTMLWindow::~RKHTMLWindow () {
	RK_TRACE (APP);

	delete current_cache_file;
}

QUrl RKHTMLWindow::restorableUrl () {
	RK_TRACE (APP);

	return QUrl ((current_url.url ().replace (RKSettingsModuleR::helpBaseUrl(), "rkward://RHELPBASE")));
}

bool RKHTMLWindow::isModified () {
	RK_TRACE (APP);
	return false;
}

void RKHTMLWindow::makeContextMenu (const QPoint& pos) {
	RK_TRACE (APP);

	QMenu *menu = page->createStandardContextMenu ();
	menu->addAction (part->run_selection);
	menu->exec (view->mapToGlobal (pos));
	delete (menu);
}

void RKHTMLWindow::selectionChanged () {
	RK_TRACE (APP);

	if (!(part && part->run_selection)) {
		RK_ASSERT (false);
		return;
	}

	part->run_selection->setEnabled (view->hasSelection ());
}

void RKHTMLWindow::runSelection () {
	RK_TRACE (APP);

	RKConsole::pipeUserCommand (view->selectedText ());
}

void RKHTMLWindow::findRequest (const QString& text, bool backwards, const RKFindBar* findbar, bool* found) {
	RK_TRACE (APP);

#ifdef NO_QT_WEBENGINE
	QWebPage::FindFlags flags = QWebPage::FindWrapsAroundDocument;
	if (backwards) flags |= QWebPage::FindBackward;
	bool highlight = findbar->isOptionSet (RKFindBar::HighlightAll);
	if (highlight) flags |= QWebPage::HighlightAllOccurrences;
	if (findbar->isOptionSet (RKFindBar::MatchCase)) flags |= QWebPage::FindCaseSensitively;

	// clear previous highlight, if any
	if (have_highlight) page->findText (QString (), QWebPage::HighlightAllOccurrences);

	*found = page->findText (text, flags);
	have_highlight = found && highlight;
#else
	// QWebEngine does not offer highlight all
	*found = true;
	QWebEnginePage::FindFlags flags;
	if (backwards) flags |= QWebEnginePage::FindBackward;
	if (findbar->isOptionSet (RKFindBar::MatchCase)) flags |= QWebEnginePage::FindCaseSensitively;
	page->findText (text, flags, [this](bool found) { if (!found) this->findbar->indicateSearchFail(); });
#endif
}


void RKHTMLWindow::slotPrint () {
	RK_TRACE (APP);

	// NOTE: taken from kwebkitpart, with small mods
	// Make it non-modal, in case a redirection deletes the part
	QPointer<QPrintDialog> dlg (new QPrintDialog (view));
	if (dlg->exec () == QPrintDialog::Accepted) {
		view->print (dlg->printer ());
	}
	delete dlg;
}

void RKHTMLWindow::slotExport() {
	RK_TRACE(APP);

	page->downloadUrl(page->url());
}

void RKHTMLWindow::slotSave() {
	RK_TRACE(APP);
	RK_ASSERT(dir);
	dir->save(dir->filename());
}

void RKHTMLWindow::slotSaveAs() {
	RK_TRACE (APP);
	RK_ASSERT(dir);
	dir->save();
}

void RKHTMLWindow::slotRevert() {
	RK_TRACE (APP);
	RK_ASSERT(dir);
	dir->revert();
}

void RKHTMLWindow::slotActivate() {
	RK_TRACE (APP);
	RK_ASSERT(dir);
	dir->activate();
}

void RKHTMLWindow::openLocationFromHistory (VisitedLocation &loc) {
	RK_TRACE (APP);
	RK_ASSERT (window_mode == HTMLHelpWindow);

	int history_last = url_history.count () - 1;
	RK_ASSERT (current_history_position >= 0);
	RK_ASSERT (current_history_position <= history_last);
	QPoint scroll_pos = loc.scroll_position.toPoint ();
	if (loc.url == current_url) {
		page->setScrollPosition (scroll_pos);
	} else {
		url_change_is_from_history = true;
		openURL (loc.url);            // TODO: merge into restoreBrowserState()?
		page->setScrollPositionWhenDone(scroll_pos);
		url_change_is_from_history = false;
	}

	part->back->setEnabled (current_history_position > 0);
	part->forward->setEnabled (current_history_position < history_last);
}

void RKHTMLWindow::slotForward () {
	RK_TRACE (APP);

	++current_history_position;
	openLocationFromHistory (url_history[current_history_position]);
}

void RKHTMLWindow::slotBack () {
	RK_TRACE (APP);

	// if going back from the end of the history, save that position, first.
	if (current_history_position >= (url_history.count () - 1)) {
		changeURL (current_url);
		--current_history_position;
	}
	--current_history_position;
	openLocationFromHistory (url_history[current_history_position]);
}

void RKHTMLWindow::openRKHPage (const QUrl &url) {
	RK_TRACE (APP);

	RK_ASSERT (url.scheme () == "rkward");
	changeURL (url);
	bool ok = false;
	if ((url.host () == "component") || (url.host () == "page")) {
		useMode (HTMLHelpWindow);

		startNewCacheFile ();
		RKHelpRenderer render (current_cache_file);
		ok = render.renderRKHelp (url);
		current_cache_file->close ();

		QUrl cache_url = QUrl::fromLocalFile (current_cache_file->fileName ());
		cache_url.setFragment (url.fragment ());
		page->load (cache_url);
	} else if (url.host ().toUpper () == "RHELPBASE") {	// NOTE: QUrl () may lowercase the host part, internally
		QUrl fixed_url = QUrl (RKSettingsModuleR::helpBaseUrl ());
		fixed_url.setPath (url.path ());
		if (url.hasQuery ()) fixed_url.setQuery (url.query ());
		if (url.hasFragment ()) fixed_url.setFragment (url.fragment ());
		ok = openURL (fixed_url);
	}
	if (!ok) {
		fileDoesNotExistMessage ();
	}
}

// static
bool RKHTMLWindow::handleRKWardURL (const QUrl &url, RKHTMLWindow *window) {
	RK_TRACE (APP);

	if (url.scheme () == "rkward") {
		if (url.host () == "runplugin") {
			QString path = url.path ();
			if (path.startsWith ('/')) path = path.mid (1);
			int sep = path.indexOf ('/');
			// NOTE: These links may originate externally, even from untrusted sources. The submit mode *must* remain "ManualSubmit" for this reason!
			RKComponentMap::invokeComponent (path.left (sep), path.mid (sep+1).split ('\n', QString::SkipEmptyParts), RKComponentMap::ManualSubmit);
		} else if (url.host () == "rhelp") {
			// TODO: find a nice solution to render this in the current window
			QStringList spec = url.path ().mid (1).split ('/');
			QString function, package, type;
			if (!spec.isEmpty ()) function = spec.takeLast ();
			if (!spec.isEmpty ()) package = spec.takeLast ();
			if (!spec.isEmpty ()) type = spec.takeLast ();
			RKHelpSearchWindow::mainHelpSearch ()->getFunctionHelp (function, package, type);
		} else if (url.host () == "settings") {
			QString path = url.path ();
			if (path.startsWith ('/')) path = path.mid (1);
			RKSettings::configureSettings(path);
		} else {
			if (window) window->openRKHPage (url);
			else RKWorkplace::mainWorkplace ()->openHelpWindow (url);	// will recurse with window set, via openURL()
		}
		return true;
	}
	return false;
}

bool RKHTMLWindow::openURL (const QUrl &url) {
	RK_TRACE (APP);

	if (handleRKWardURL (url, this)) return true;

	QPoint restore_position;
	if (url == current_url) restore_position = page->scrollPosition().toPoint();

	QMimeType mtype = QMimeDatabase ().mimeTypeForUrl (url);
	if (window_mode == HTMLOutputWindow) {
		if (url != current_url) {
			// output window should not change url after initialization open any links in new windows
			if (!current_url.isEmpty ()) {
				RKWorkplace::mainWorkplace ()->openAnyUrl (url);
				return false;
			}

			current_url = url;	// needs to be set before registering
			RKOutputWindowManager::self ()->registerWindow (this);
			dir = RKOutputDirectory::findOutputByWorkPath(url.toLocalFile());
			part->setOutputDirectoryActionsEnabled(dir != nullptr);
			if (dir) {
				connect(dir, &RKOutputDirectory::stateChange, this, &RKHTMLWindow::updateState);
			}
		}
	}

	if (url.isLocalFile () && (page->supportsContentType (mtype.name ()) || window_mode == HTMLOutputWindow)) {
		changeURL (url);
		QFileInfo out_file (url.toLocalFile ());
		bool ok = out_file.exists();
		if (ok)  {
			if (!mtype.inherits ("text/html")) {
				RK_DEBUG (APP, DL_WARNING, "Applying workaround for https://bugs.kde.org/show_bug.cgi?id=405386");
				QFile f (url.toLocalFile ());
				f.open (QIODevice::ReadOnly);
				page->setHtml (f.readAll());
				f.close ();
			} else {
				page->load (url);
			}
			if (!restore_position.isNull()) page->setScrollPositionWhenDone(restore_position);
		} else {
			fileDoesNotExistMessage ();
		}
		return ok;
	}

	if (url_change_is_from_history || url.scheme ().toLower ().startsWith (QLatin1String ("help"))) {	// handle any page that we have previously handled (from history)
		changeURL (url);
		page->load (url);
		return true;
	}

	// special casing for R's dynamic help pages. These should be considered local, even though they are served through http
	if (url.scheme ().toLower ().startsWith (QLatin1String ("http"))) {
		QString host = url.host ();
		if ((host == "127.0.0.1") || (host == "localhost") || host == QHostInfo::localHostName ()) {
			KIO::TransferJob *job = KIO::get (url, KIO::Reload);
			connect (job, static_cast<void (KIO::TransferJob::*)(KIO::Job*, const QString&)>(&KIO::TransferJob::mimetype), this, &RKHTMLWindow::mimeTypeDetermined);
			// WORKAROUND. See slot.
			connect (job, &KIO::TransferJob::result, this, &RKHTMLWindow::mimeTypeJobFail);
			return true;
		}
	}

	RKWorkplace::mainWorkplace ()->openAnyUrl (url, QString (), page->supportsContentType (mtype.name ()));	// NOTE: text/html type urls, which we have not handled, above, are forced to be opened externally, to avoid recursion. E.g. help:// protocol urls.
	return true;
}

void RKHTMLWindow::mimeTypeJobFail (KJob* job) {
	RK_TRACE (APP);
	
	KIO::TransferJob* tj = static_cast<KIO::TransferJob*> (job);
	if (tj->error ()) {
		// WORKAROUND for bug in KIO version 5.9.0: After a redirect, the transfer job would claim "does not exist". Here, we help it get over _one_ redirect, hoping R's help server
		// won't do more redirection than that.
		// TODO: Report this!
		QUrl url = tj->url ();
		if (!tj->redirectUrl ().isEmpty ()) url = tj->redirectUrl ();
		KIO::TransferJob *secondchance = KIO::get (url, KIO::Reload);
		connect (secondchance, static_cast<void (KIO::TransferJob::*)(KIO::Job*, const QString&)>(&KIO::TransferJob::mimetype), this, &RKHTMLWindow::mimeTypeDetermined);
		connect (secondchance, &KIO::TransferJob::result, this, &RKHTMLWindow::mimeTypeJobFail2);
	}
}

void RKHTMLWindow::mimeTypeJobFail2 (KJob* job) {
	RK_TRACE (APP);
	
	KIO::TransferJob* tj = static_cast<KIO::TransferJob*> (job);
	if (tj->error ()) {
		// WORKAROUND continued. See above.
		QUrl url = tj->url ();
		if (!tj->redirectUrl ().isEmpty ()) url = tj->redirectUrl ();
		RKWorkplace::mainWorkplace ()->openAnyUrl (url);
	}
}

void RKHTMLWindow::mimeTypeDetermined (KIO::Job* job, const QString& type) {
	RK_TRACE (APP);

	KIO::TransferJob* tj = static_cast<KIO::TransferJob*> (job);
	QUrl url = tj->url ();
	tj->putOnHold ();
	if (page->supportsContentType (type)) {
		changeURL (url);
		page->load (url);
	} else {
		RKWorkplace::mainWorkplace ()->openAnyUrl (url, type);
	}
}

void RKHTMLWindow::internalNavigation (const QUrl& new_url) {
	RK_TRACE (APP);

	QUrl real_url = current_url;    // Note: This could be something quite different from new_url: a temp file for rkward://-urls. We know the base part of the URL has not actually changed, when this gets called, though.
	real_url.setFragment (new_url.fragment ());

	changeURL (real_url);
}

void RKHTMLWindow::changeURL (const QUrl &url) {
	QUrl prev_url = current_url;
	current_url = url;
	updateCaption (url);

	if (!url_change_is_from_history) {
		if (window_mode == HTMLHelpWindow) {
			if (current_history_position >= 0) {	// just skip initial blank page
				url_history = url_history.mid (0, current_history_position);

				VisitedLocation loc;
				loc.url = prev_url;
				saveBrowserState (&loc);
				if (url_history.value (current_history_position).url == url) { // e.g. a redirect. We still save the most recent browser state, but do not keep two entries for the same page
					url_history.pop_back ();
					--current_history_position;
				}
				url_history.append (loc);
			}

			++current_history_position;
 			part->back->setEnabled (current_history_position > 0);
			part->forward->setEnabled (false);
		}
	}
}

void RKHTMLWindow::updateState(){
	updateCaption(current_url);
}

void RKHTMLWindow::updateCaption (const QUrl &url) {
	RK_TRACE (APP);

	if (window_mode == HTMLOutputWindow) {
		if (dir) {
			QString name = dir->caption();
			QString mods;
			if (dir->isActive()) mods.append(i18n("[Active]"));
			// TODO: use icon(s), instead
			bool mod = dir->isModifiedFast();
			if (mod) mods.append("(*)");
			if (!mods.isEmpty()) {
				name.append(' ');
				name.append(mods);
			}
			setCaption(name);
			part->revert->setEnabled(mod);
		} else {
			setCaption (i18n ("Output %1", url.fileName ()));
		}
	}
	else setCaption (url.fileName ());
}

void RKHTMLWindow::refresh () {
	RK_TRACE (APP);

	view->reload ();
}

void RKHTMLWindow::scrollToBottom () {
	RK_TRACE (APP);

	RK_ASSERT (window_mode == HTMLOutputWindow);
#ifdef NO_QT_WEBENGINE
	page->mainFrame ()->setScrollBarValue (Qt::Vertical, view->page ()->mainFrame ()->scrollBarMaximum (Qt::Vertical));
#else
	page->runJavaScript(QString("{ let se = (document.scrollingElement || document.body); se.scrollTop = se.scrollHeight; }"));
#endif
}

void RKHTMLWindow::zoomIn () {
	RK_TRACE (APP);
	view->setZoomFactor (view->zoomFactor () * 1.1);
}

void RKHTMLWindow::zoomOut () {
	RK_TRACE (APP);
	view->setZoomFactor (view->zoomFactor () / 1.1);
}

void RKHTMLWindow::setTextEncoding (QTextCodec* encoding) {
	RK_TRACE (APP);

	page->settings ()->setDefaultTextEncoding (encoding->name ());
	view->reload ();
}

void RKHTMLWindow::useMode (WindowMode new_mode) {
	RK_TRACE (APP);

	if (window_mode == new_mode) return;

	if (new_mode == HTMLOutputWindow) {
		type = RKMDIWindow::OutputWindow | RKMDIWindow::DocumentWindow;
		setWindowIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowOutput));
		part->setOutputWindowSkin ();
		setMetaInfo (i18n ("Output Window"), QUrl ("rkward://page/rkward_output"), RKSettings::PageOutput);
		connect (page, &RKWebPage::loadFinished, this, &RKHTMLWindow::scrollToBottom);
		page->action (RKWebPage::Reload)->setText (i18n ("&Refresh Output"));

//	TODO: This would be an interesting extension, but how to deal with concurrent edits?
//		page->setContentEditable (true);
	} else {
		RK_ASSERT (new_mode == HTMLHelpWindow);

		type = RKMDIWindow::HelpWindow | RKMDIWindow::DocumentWindow;
		setWindowIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowHelp));
		part->setHelpWindowSkin ();
		disconnect (page, &RKWebPage::loadFinished, this, &RKHTMLWindow::scrollToBottom);
	}

	updateCaption (current_url);
	window_mode = new_mode;
}

void RKHTMLWindow::startNewCacheFile () {
	delete current_cache_file;
	current_cache_file = new QTemporaryFile (QDir::tempPath () + QLatin1String ("/rkward_XXXXXX") + QLatin1String (".html"));
	current_cache_file->open ();
}

void RKHTMLWindow::fileDoesNotExistMessage () {
	RK_TRACE (APP);

	startNewCacheFile ();
	if (window_mode == HTMLOutputWindow) {
		current_cache_file->write (i18n ("<HTML><BODY><H1>RKWard output file could not be found</H1>\n</BODY></HTML>").toUtf8 ());
	} else {
		current_cache_file->write (QString ("<html><body><h1>" + i18n ("Page does not exist or is broken") + "</h1></body></html>").toUtf8 ());
	}
	current_cache_file->close ();

	QUrl cache_url = QUrl::fromLocalFile (current_cache_file->fileName ());
	page->load (cache_url);
}

void RKHTMLWindow::flushOutput () {
	RK_TRACE (APP);

	if (dir) {
		dir->clear();
		return;
	}

	// TODO: remove legacy code below, eventually
	int res = KMessageBox::questionYesNo (this, i18n ("Do you really want to clear the output? This will also remove all image files used in the output. It will not be possible to restore it."), i18n ("Flush output?"));
	if (res==KMessageBox::Yes) {
		QFile out_file (current_url.toLocalFile ());
		RCommand *c = new RCommand ("rk.flush.output (" + RCommand::rQuote (out_file.fileName ()) + ", ask=FALSE)\n", RCommand::App);
		connect (c->notifier (), &RCommandNotifier::commandFinished, this, &RKHTMLWindow::refresh);
		RKProgressControl *status = new RKProgressControl (this, i18n ("Flushing output"), i18n ("Flushing output"), RKProgressControl::CancellableNoProgress);
		status->addRCommand (c, true);
		status->doNonModal (true);
		RKGlobals::rInterface ()->issueCommand (c);
	}
}

void RKHTMLWindow::saveBrowserState (VisitedLocation* state) {
	RK_TRACE (APP);

	if (page) {
		state->scroll_position = page->scrollPosition ();
	} else {
		state->scroll_position = QPoint ();
	}
}

RKHTMLWindowPart::RKHTMLWindowPart (RKHTMLWindow* window) : KParts::Part (window) {
	RK_TRACE (APP);
	setComponentName (QCoreApplication::applicationName (), QGuiApplication::applicationDisplayName ());
	RKHTMLWindowPart::window = window;
	setWidget (window);
}

void RKHTMLWindowPart::initActions () {
	RK_TRACE (APP);

	// We keep our own history.
	window->page->action (RKWebPage::Back)->setVisible (false);
	window->page->action (RKWebPage::Forward)->setVisible (false);
	// For now we won't bother with this one: Does not behave well, in particular (but not only) WRT to rkward://-links
	window->page->action (RKWebPage::DownloadLinkToDisk)->setVisible (false);
#ifndef NO_QT_WEBENGINE
	// Not really useful for us, and cannot easily be made to work, as all new pages go through RKWorkplace::openAnyUrl()
	window->page->action (RKWebPage::ViewSource)->setVisible (false);
	// Well, technically, all our windows are tabs, but we're calling them "window".
	// At any rate, we don't need both "open link in new tab" and "open link in new window".
	window->page->action (RKWebPage::OpenLinkInNewTab)->setVisible (false);
#endif

	// common actions
	actionCollection ()->addAction (KStandardAction::Copy, "copy", window->view->pageAction (RKWebPage::Copy), SLOT (trigger()));
	QAction* zoom_in = actionCollection ()->addAction ("zoom_in", new QAction (QIcon::fromTheme("zoom-in"), i18n ("Zoom In"), this));
	connect (zoom_in, &QAction::triggered, window, &RKHTMLWindow::zoomIn);
	QAction* zoom_out = actionCollection ()->addAction ("zoom_out", new QAction (QIcon::fromTheme("zoom-out"), i18n ("Zoom Out"), this));
	connect (zoom_out, &QAction::triggered, window, &RKHTMLWindow::zoomOut);
	actionCollection ()->addAction (KStandardAction::SelectAll, "select_all", window->view->pageAction (RKWebPage::SelectAll), SLOT (trigger()));
	// unfortunately, this will only affect the default encoding, not necessarily the "real" encoding
	KCodecAction *encoding = new KCodecAction (QIcon::fromTheme("character-set"), i18n ("Default &Encoding"), this, true);
	encoding->setWhatsThis(i18n ("Set the encoding to assume in case no explicit encoding has been set in the page or in the HTTP headers."));
	actionCollection ()->addAction ("view_encoding", encoding);
	connect (encoding, static_cast<void (KCodecAction::*)(QTextCodec *)>(&KCodecAction::triggered), window, &RKHTMLWindow::setTextEncoding);

	print = actionCollection()->addAction(KStandardAction::Print, "print_html", window, SLOT (slotPrint()));
	export_page = actionCollection()->addAction("save_html", new QAction(QIcon::fromTheme("file-save"), i18n("Export Page as HTML"), this));
	connect(export_page, &QAction::triggered, window, &RKHTMLWindow::slotExport);

	run_selection = RKStandardActions::runCurrent (window, window, SLOT (runSelection()));

	// help window actions
	back = actionCollection ()->addAction (KStandardAction::Back, "help_back", window, SLOT (slotBack()));
	back->setEnabled (false);

	forward = actionCollection ()->addAction (KStandardAction::Forward, "help_forward", window, SLOT (slotForward()));
	forward->setEnabled (false);

	// output window actions
	window->file_save_action = actionCollection()->addAction(KStandardAction::Save, window, SLOT(slotSave()));
	window->file_save_action->setText(i18n("Save Output"));
	window->file_save_as_action = actionCollection()->addAction(KStandardAction::SaveAs, window, SLOT(slotSaveAs()));
	window->file_save_action->setText(i18n("Save Output As"));

	outputFlush = actionCollection ()->addAction ("output_flush", window, SLOT (flushOutput()));
	outputFlush->setText (i18n ("&Clear Output"));
	outputFlush->setIcon (QIcon::fromTheme("edit-delete"));

	outputRefresh = actionCollection()->addAction("output_refresh", window->page->action(RKWebPage::Reload));

	revert = actionCollection()->addAction("output_revert", window, SLOT(slotRevert()));
	revert->setText(i18n("&Revert to last saved state"));
	revert->setIcon(QIcon::fromTheme("edit-undo"));

	activate = actionCollection()->addAction("output_activate", window, SLOT(slotActivate()));
	activate->setText(i18n("Set Output as &Active"));
	activate->setIcon(QIcon::fromTheme("emblem-favorite"));
	activate->setWhatsThis(i18n("Set this output as the file to append output to."));

	actionCollection ()->addAction (KStandardAction::Find, "find", window->findbar, SLOT (activate()));
	QAction* findAhead = actionCollection ()->addAction ("find_ahead", new QAction (i18n ("Find as you type"), this));
	actionCollection ()->setDefaultShortcut (findAhead, '/');
	connect (findAhead, &QAction::triggered, window->findbar, &RKFindBar::activate);
	actionCollection ()->addAction (KStandardAction::FindNext, "find_next", window->findbar, SLOT (forward()));
	actionCollection ()->addAction (KStandardAction::FindPrev, "find_previous", window->findbar, SLOT (backward()));
}

void RKHTMLWindowPart::setOutputDirectoryActionsEnabled(bool enable) {
	RK_TRACE(APP);

	window->file_save_action->setVisible(enable);
	window->file_save_as_action->setVisible(enable);
	revert->setVisible(enable);
	activate->setVisible(enable);
}

void RKHTMLWindowPart::setOutputWindowSkin() {
	RK_TRACE(APP);

	print->setText(i18n("Print output"));
	setXMLFile("rkoutputwindow.rc");
	run_selection->setVisible(false);
}

void RKHTMLWindowPart::setHelpWindowSkin() {
	RK_TRACE(APP);

	print->setText(i18n("Print page"));
	setXMLFile("rkhelpwindow.rc");
	run_selection->setVisible(true);
}

//////////////////////////////////////////
//////////////////////////////////////////

bool RKHelpRenderer::renderRKHelp (const QUrl &url) {
	RK_TRACE (APP);

	if (url.scheme () != "rkward") {
		RK_ASSERT (false);
		return (false);
	}

	bool for_component = false;		// is this a help page for a component, or a top-level help page?
	if (url.host () == "component") for_component = true;

	QStringList anchors, anchornames;

	RKComponentHandle *chandle = 0;
	if (for_component) {
		chandle = componentPathToHandle (url.path ());
		if (!chandle) return false;
	}

	XMLHelper component_xml_helper (for_component ? chandle->getFilename () : QString (), for_component ? chandle->messageCatalog () : 0);
	component_xml = &component_xml_helper;
	QString help_file_name;
	QDomElement element;
	QString help_base_dir = RKCommonFunctions::getRKWardDataDir () + "pages/";
	QString css_filename = QUrl::fromLocalFile (help_base_dir + "rkward_help.css").toString ();

	// determine help file, and prepare
	if (for_component) {
		component_doc_element = component_xml_helper.openXMLFile (DL_ERROR);
		if (component_doc_element.isNull ()) return false;
		element = component_xml_helper.getChildElement (component_doc_element, "help", DL_ERROR);
		if (!element.isNull ()) {
			help_file_name = component_xml_helper.getStringAttribute (element, "file", QString (), DL_ERROR);
			if (!help_file_name.isEmpty ()) help_file_name = QFileInfo (chandle->getFilename ()).absoluteDir ().filePath (help_file_name);
		}
	} else {
		help_file_name = help_base_dir + url.path () + ".rkh";
	}
	RK_DEBUG (APP, DL_DEBUG, "rendering help page for local file %s", help_file_name.toLatin1().data());

	// open help file
	const RKMessageCatalog *catalog = component_xml_helper.messageCatalog ();
	if (!for_component) catalog = RKMessageCatalog::getCatalog ("rkward__pages", RKCommonFunctions::getRKWardDataDir () + "po/");
	XMLHelper help_xml_helper (help_file_name, catalog);
	help_xml = &help_xml_helper;
	help_doc_element = help_xml_helper.openXMLFile (DL_ERROR);
	if (help_doc_element.isNull () && (!for_component)) return false;

	// initialize output, and set title
	QString page_title (i18n ("No Title"));
	if (for_component) {
		page_title = chandle->getLabel ();
	} else {
		element = help_xml_helper.getChildElement (help_doc_element, "title", DL_WARNING);
		page_title = help_xml_helper.i18nElementText (element, false, DL_WARNING);
	}
	writeHTML ("<html><head><title>" + page_title + "</title><link rel=\"stylesheet\" type=\"text/css\" href=\"" + css_filename + "\">"
	           "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head>\n<body><div id=\"main\">\n<h1>" + page_title + "</h1>\n");

	if (help_doc_element.isNull ()) {
		RK_ASSERT (for_component);
		writeHTML (i18n ("<h1>Help page missing</h1>\n<p>The help page for this component has not yet been written (or is broken). Please consider contributing it.</p>\n"));
	}
	if (for_component) {
		QString component_id = componentPathToId (url.path());
		RKComponentHandle *handle = componentPathToHandle (url.path());
		if (handle && handle->isAccessible ()) writeHTML ("<a href=\"rkward://runplugin/" + component_id + "/\">" + i18n ("Use %1 now", page_title) + "</a>");
	}

	// fix all elements containing an "src" attribute
	QDir base_path (QFileInfo (help_file_name).absolutePath());
	XMLChildList src_elements = help_xml_helper.findElementsWithAttribute (help_doc_element, "src", QString (), true, DL_DEBUG);
	for (XMLChildList::iterator it = src_elements.begin (); it != src_elements.end (); ++it) {
		QString src = (*it).attribute ("src");
		if (QUrl (src).isRelative ()) {
			src = "file://" + QDir::cleanPath (base_path.filePath (src));
			(*it).setAttribute ("src", src);
		}
	}

	// render the sections
	element = help_xml_helper.getChildElement (help_doc_element, "summary", DL_INFO);
	if (!element.isNull ()) {
		writeHTML (startSection ("summary", i18n ("Summary"), QString (), &anchors, &anchornames));
		writeHTML (renderHelpFragment (element));
	}

	element = help_xml_helper.getChildElement (help_doc_element, "usage", DL_INFO);
	if (!element.isNull ()) {
		writeHTML (startSection ("usage", i18n ("Usage"), QString (), &anchors, &anchornames));
		writeHTML (renderHelpFragment (element));
	}

	XMLChildList section_elements = help_xml_helper.getChildElements (help_doc_element, "section", DL_INFO);
	for (XMLChildList::iterator it = section_elements.begin (); it != section_elements.end (); ++it) {
		QString title = help_xml_helper.i18nStringAttribute (*it, "title", QString (), DL_WARNING);
		QString shorttitle = help_xml_helper.i18nStringAttribute (*it, "shorttitle", QString (), DL_DEBUG);
		QString id = help_xml_helper.getStringAttribute (*it, "id", QString (), DL_WARNING);
		writeHTML (startSection (id, title, shorttitle, &anchors, &anchornames));
		writeHTML (renderHelpFragment (*it));
	}

	// the section "settings" is the most complicated, as the labels of the individual GUI items has to be fetched from the component description. Of course it is only meaningful for component help, and not rendered for top level help pages.
	if (for_component) {
		element = help_xml_helper.getChildElement (help_doc_element, "settings", DL_INFO);
		if (!element.isNull ()) {
			writeHTML (startSection ("settings", i18n ("GUI settings"), QString (), &anchors, &anchornames));
			XMLChildList setting_elements = help_xml_helper.getChildElements (element, QString (), DL_WARNING);
			for (XMLChildList::iterator it = setting_elements.begin (); it != setting_elements.end (); ++it) {
				if ((*it).tagName () == "setting") {
					QString id = help_xml_helper.getStringAttribute (*it, "id", QString (), DL_WARNING);
					QString title = help_xml_helper.i18nStringAttribute (*it, "title", QString (), DL_INFO);
					if (title.isEmpty ()) title = resolveLabel (id);
					writeHTML ("<h4>" + title + "</h4>");
					writeHTML (renderHelpFragment (*it));
				} else if ((*it).tagName () == "caption") {
					QString id = help_xml_helper.getStringAttribute (*it, "id", QString (), DL_WARNING);
					QString title = help_xml_helper.i18nStringAttribute (*it, "title", QString (), DL_INFO);
					if (title.isEmpty ()) title = resolveLabel (id);
					writeHTML ("<h3>" + title + "</h3>");
				} else {
					help_xml_helper.displayError (&(*it), "Tag not allowed, here", DL_WARNING);
				}
			}
		}
	}

	// "related" section
	element = help_xml_helper.getChildElement (help_doc_element, "related", DL_INFO);
	if (!element.isNull ()) {
		writeHTML (startSection ("related", i18n ("Related functions and pages"), QString (), &anchors, &anchornames));
		writeHTML (renderHelpFragment (element));
	}

	// "technical" section
	element = help_xml_helper.getChildElement (help_doc_element, "technical", DL_INFO);
	if (!element.isNull ()) {
		writeHTML (startSection ("technical", i18n ("Technical details"), QString (), &anchors, &anchornames));
		writeHTML (renderHelpFragment (element));
	}

	if (for_component) {
		// "dependencies" section
		QList<RKComponentDependency> deps = chandle->getDependencies ();
		if (!deps.isEmpty ()) {
			writeHTML (startSection ("dependencies", i18n ("Dependencies"), QString (), &anchors, &anchornames));
			writeHTML (RKComponentDependency::depsToHtml (deps));
		}
	}

	// "about" section
	RKComponentAboutData about;
	if (for_component) {
		about = chandle->getAboutData ();
	} else {
		about = RKComponentAboutData (help_xml_helper.getChildElement (help_doc_element, "about", DL_INFO), *help_xml);
	}
	if (about.valid) {
		writeHTML (startSection ("about", i18n ("About"), QString (), &anchors, &anchornames));
		writeHTML (about.toHtml ());
	}

	// create a navigation bar
	QUrl url_copy = url;
	QString navigation = i18n ("<h1>On this page:</h1>");
	RK_ASSERT (anchornames.size () == anchors.size ());
	for (int i = 0; i < anchors.size (); ++i) {
		QString anchor = anchors[i];
		QString anchorname = anchornames[i];
		if (!(anchor.isEmpty () || anchorname.isEmpty ())) {
			url_copy.setFragment (anchor);
			navigation.append ("<p><a href=\"" + url_copy.url () + "\">" + anchorname + "</a></p>\n");
		}
	}
	writeHTML ("</div><div id=\"navigation\">" + navigation + "</div>");
	writeHTML ("</body></html>\n");

	return (true);
}

QString RKHelpRenderer::resolveLabel (const QString& id) const {
	RK_TRACE (APP);

	QDomElement source_element = component_xml->findElementWithAttribute (component_doc_element, "id", id, true, DL_WARNING);
	if (source_element.isNull ()) {
		RK_DEBUG (PLUGIN, DL_ERROR, "No such UI element: %s", qPrintable (id));
	}
	return (component_xml->i18nStringAttribute (source_element, "label", i18n ("Unnamed GUI element"), DL_WARNING));
}

QString RKHelpRenderer::renderHelpFragment (QDomElement &fragment) {
	RK_TRACE (APP);

	QString text = help_xml->i18nElementText (fragment, true, DL_WARNING);

	// Can't resolve links and references based on the already parsed dom-tree, because they can be inside string to be translated.
	// I.e. resolving links before doing i18n will cause i18n-lookup to fail
	int pos = 0;
	int npos;
	QString ret;
	while ((npos = text.indexOf ("<link", pos)) >= 0) {
		ret += text.midRef(pos, npos - pos);

		QString href;
		int href_start = text.indexOf (" href=\"", npos + 5);
		if (href_start >= 0) {
			href_start += 7;
			int href_end = text.indexOf ("\"", href_start);
			href = text.mid (href_start, href_end - href_start);
		}
		QString linktext;
		int end = text.indexOf (">", npos) + 1;
		if (text[end-2] != QChar ('/')) {
			int nend = text.indexOf ("</link>", end);
			linktext = text.mid (end, nend - end);
			end = nend + 7;
		}

		ret += prepareHelpLink (href, linktext);
		pos = end;
	}
	ret += text.midRef(pos);

	if (component_xml) {
		text = ret;
		ret.clear ();
		pos = 0;
		while ((npos = text.indexOf ("<label ", pos)) >= 0) {
			ret += text.midRef(pos, npos - pos);

			QString id;
			int id_start = text.indexOf ("id=\"", npos + 6);
			if (id_start >= 0) {
				id_start += 4;
				int id_end = text.indexOf ("\"", id_start);
				id = text.mid (id_start, id_end - id_start);
				pos = text.indexOf ("/>", id_end) + 2;
			}
			ret += resolveLabel (id);
		}
		ret += text.midRef(pos);
	}

	RK_DEBUG (APP, DL_DEBUG, "%s", qPrintable (ret));
	return ret;
}

QString RKHelpRenderer::prepareHelpLink (const QString &href, const QString &text) {
	RK_TRACE (APP);

	QString ret = "<a href=\"" + href + "\">";
	if (!text.isEmpty ()) {
		ret += text;
	} else {
		QString ltext;
		QUrl url (href);
		if (url.scheme () == "rkward") {
			if (url.host () == "component") {
				RKComponentHandle *chandle = componentPathToHandle (url.path ());
				if (chandle) ltext = chandle->getLabel ();
			} else if (url.host () == "rhelp") {
				ltext = i18n ("R Reference on '%1'", url.path ().mid (1));
			} else if (url.host () == "page") {
				QString help_base_dir = RKCommonFunctions::getRKWardDataDir () + "pages/";
		
				XMLHelper xml (help_base_dir + url.path () + ".rkh", RKMessageCatalog::getCatalog ("rkward__pages", RKCommonFunctions::getRKWardDataDir () + "po/"));
				QDomElement doc_element = xml.openXMLFile (DL_WARNING);
				QDomElement title_element = xml.getChildElement (doc_element, "title", DL_WARNING);
				ltext = xml.i18nElementText (title_element, false, DL_WARNING);
			}

			if (ltext.isEmpty ()) {
				ltext = i18n ("BROKEN REFERENCE");
				RK_DEBUG (APP, DL_WARNING, "Broken reference to %s", url.path ().toLatin1 ().data ());
			}
		}
		ret.append (ltext);
	}
	return (ret + "</a>");
}

QString RKHelpRenderer::componentPathToId (QString path) {
	RK_TRACE (APP);

	QStringList path_segments = path.split ('/', QString::SkipEmptyParts);
	if (path_segments.count () > 2) return 0;
	if (path_segments.count () < 1) return 0;
	if (path_segments.count () == 1) path_segments.push_front ("rkward");
	RK_ASSERT (path_segments.count () == 2);

	return (path_segments.join ("::"));
}

RKComponentHandle *RKHelpRenderer::componentPathToHandle (QString path) {
	RK_TRACE (APP);

	return (RKComponentMap::getComponentHandle (componentPathToId (path)));
}

QString RKHelpRenderer::startSection (const QString &name, const QString &title, const QString &shorttitle, QStringList *anchors, QStringList *anchor_names) {
	QString ret = "<a name=\"" + name + "\">";
	ret.append ("<h2>" + title + "</h2>\n");
	anchors->append (name);
	if (!shorttitle.isNull ()) anchor_names->append (shorttitle);
	else anchor_names->append (title);
	return (ret);
}

void RKHelpRenderer::writeHTML (const QString& string) {
	RK_TRACE (APP);

	device->write (string.toUtf8 ());
}

/////////////////////////////////////
/////////////////////////////////////

#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <KIO/CopyJob>

// static
RKOutputWindowManager* RKOutputWindowManager::_self = 0;

RKOutputWindowManager* RKOutputWindowManager::self () {
	if (!_self) {
		RK_TRACE (APP);

		_self = new RKOutputWindowManager ();
	}
	return _self;
}

RKOutputWindowManager::RKOutputWindowManager () : QObject () {
	RK_TRACE (APP);

	file_watcher = new KDirWatch (this);
	connect (file_watcher, &KDirWatch::dirty, this, &RKOutputWindowManager::fileChanged);
	connect (file_watcher, &KDirWatch::created, this, &RKOutputWindowManager::fileChanged);
}

RKOutputWindowManager::~RKOutputWindowManager () {
	RK_TRACE (APP);

	file_watcher->removeFile (current_default_path);
	delete (file_watcher);
}

QString watchFilePath (const QString &file) {
	RK_TRACE (APP);

	QFileInfo fi = QFileInfo (file);
	if (fi.isSymLink ()) {
		// Monitor the actual data and not the symlink (taken from ktexteditor)
		return (fi.canonicalFilePath ());
	}
	return (file);
}

void RKOutputWindowManager::registerWindow (RKHTMLWindow *window) {
	RK_TRACE (APP);

	RK_ASSERT (window->mode () == RKHTMLWindow::HTMLOutputWindow);
	QUrl url = window->url ();

	if (!url.isLocalFile ()) {
		RK_ASSERT (false);		// should not happen right now, but might be an ok condition in the future. We can't monitor non-local files, though.
		return;
	}

	url = url.adjusted (QUrl::NormalizePathSegments);
	QString file = watchFilePath (url.toLocalFile ());
	if (!windows.contains (file, window)) {
		if (!windows.contains (file)) {
			if (file != current_default_path) {
				RK_DEBUG (APP, DL_DEBUG, "starting to watch %s for changes", qPrintable (file));
				file_watcher->addFile (file);
			}
		}
	
		windows.insertMulti (file, window);
		connect (window, &QObject::destroyed, this, &RKOutputWindowManager::windowDestroyed);
	} else {
		RK_ASSERT (false);
	}
}

void RKOutputWindowManager::setCurrentOutputPath (const QString &_path) {
	RK_TRACE (APP);

	QUrl url = QUrl::fromLocalFile (_path);
	url = url.adjusted (QUrl::NormalizePathSegments);
	QString path = watchFilePath (url.toLocalFile ());
	RK_DEBUG (APP, DL_DEBUG, "setting default output file to %s (%s), from %s", qPrintable (path), qPrintable (_path), qPrintable (current_default_path));

#ifdef Q_OS_WIN
	// On windows, when flushing the output (i.e. deleting, re-creating it), KDirWatch seems to purge the file from the
	// watch list (KDE 4.10.2; always?), so we need to re-add it. To make things complex, however, this may happen
	// asynchronously, with this function called (via rk.set.output.html.file()), _before_ KDirWatch purges the file.
	// To hack around the race condition, we re-watch the output file after a short delay.
	QTimer::singleShot (100, this, SLOT (rewatchOutput()));
#endif
	if (path == current_default_path) return;

	auto old_win = windows.value(current_default_path);
	auto new_win = windows.value(path);

	if (!new_win) {
		RK_DEBUG(APP, DL_DEBUG, "starting to watch %s for changes, KDirWatch method %d", qPrintable(path), file_watcher->internalMethod());
		file_watcher->addFile(path);
	}
	if (!old_win) {
		if (!current_default_path.isEmpty()) {
			RK_DEBUG(APP, DL_DEBUG, "no longer watching %s for changes", qPrintable(current_default_path));
			file_watcher->removeFile(current_default_path);
		}
	}

	current_default_path = path;
	if (old_win) old_win->updateState();
	if (new_win) new_win->updateState();
}

void RKOutputWindowManager::rewatchOutput () {
	RK_TRACE (APP);
	// called on Windows, only
	file_watcher->removeFile (current_default_path);
	file_watcher->addFile (current_default_path);
}

QList<RKHTMLWindow*> RKOutputWindowManager::existingOutputWindows(const QString &path) const {
	RK_TRACE (APP);

	return (windows.values(path));
}

void RKOutputWindowManager::fileChanged (const QString &path) {
	RK_TRACE (APP);

	RKHTMLWindow *w = 0;
	QList<RKHTMLWindow *> window_list = windows.values (path);
	for (int i = 0; i < window_list.size (); ++i) {
		window_list[i]->refresh ();
		w = window_list[i];
	}
	RK_DEBUG (APP, DL_DEBUG, "changed %s, window %p", qPrintable (path), w);

	if (w) {
		if (RKSettingsModuleOutput::autoRaise ()) w->activate ();
		if (w->outputDirectory()) w->outputDirectory()->setKnownModified(true);
	} else {
		RK_ASSERT (path == current_default_path);
		if (RKSettingsModuleOutput::autoShow ()) {
			RKOutputDirectory* dir = RKOutputDirectory::findOutputByWorkPath(path);
			if (dir) {
				dir->view(true);
			} else {
				RKWorkplace::mainWorkplace()->openHTMLWindow(QUrl::fromUserInput(path, QString(), QUrl::AssumeLocalFile));
			}
		}
	}
}

void RKOutputWindowManager::windowDestroyed(QObject *window) {
	RK_TRACE (APP);

	// warning: Do not call any methods on the window. It is half-destroyed, already.
	RKHTMLWindow *w = static_cast<RKHTMLWindow*>(window);

	QString path = windows.key(w);
	windows.remove(path, w);

	// if there are no further windows for this file, stop listening
	if ((path != current_default_path) && (!windows.contains(path))) {
		RK_DEBUG(APP, DL_DEBUG, "no longer watching %s for changes", qPrintable(path));
		file_watcher->removeFile(path);
	}
}

#include "rkhtmlwindow.moc"
