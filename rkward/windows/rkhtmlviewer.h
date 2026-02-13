/*
rkhtmlviewerwidget - This file is part of the RKWard project. Created: Sat Feb 07 2026
SPDX-FileCopyrightText: 2026 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKHTMLVIEWERWIDGET_H
#define RKHTMLVIEWERWIDGET_H

#include <QObject>
#include <QString>
#include <QUrl>

class QMenu;
class QWidget;
class RKFindBar;
class RKHTMLWindow;

/** Abstract base class for the HTML viewer component */
class RKHTMLViewer : public QObject {
	Q_OBJECT
  public:
	static RKHTMLViewer *getNew(RKHTMLWindow *parent);
	virtual QWidget *createWidget(QWidget *parent) = 0;
	virtual void reload() {
		load(url());
	}
	virtual QUrl url() const = 0;
	virtual void load(const QUrl &url) = 0;
	virtual void exportPage() = 0;
	virtual void print() = 0;
	virtual void installPersistentJS(const QString &script, const QString &id) = 0;
	virtual void runJS(const QString &script, std::function<void(const QVariant &)> callback) = 0;
	void runJS(const QString &script) {
		runJS(script, [](const QVariant &) {});
	}
	virtual void setHTML(const QString &html, const QUrl &url) = 0;
	virtual bool installHelpProtocolHandler() {
		return false;
	}
	virtual QPoint scrollPosition() const = 0;
	virtual void setScrollPosition(const QPoint &pos, bool wait_for_load) = 0;
	// TODO: Could be implemented in base-class as JS call
	virtual void findRequest(const QString &text, bool backwards, RKFindBar *findbar, bool *found) = 0;
	virtual QMenu *createContextMenu(const QPoint &clickpos) = 0;
	virtual QString selectedText() const = 0;
	virtual bool supportsContentType(const QString &mimename) = 0;
	virtual void zoomIn() = 0;
	virtual void zoomOut() = 0;
  Q_SIGNALS:
	// TODO: code to emit these signals
	void pageInternalNavigation(const QUrl &new_url);
	void selectionChanged(bool has_selection);
	void loadFinished();
	void navigationRequest(const QUrl &current_real_url, const QUrl &requested_url, bool is_new_window);

  protected:
	RKHTMLViewer(QObject *parent);
	RKHTMLWindow *window;
};

#endif
