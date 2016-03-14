/***************************************************************************
                          rkstandardactions  -  description
                             -------------------
    begin                : Sun Nov 18 2007
    copyright            : (C) 2007-2016 by Thomas Friedrichsmeier
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

#include "rkstandardactions.h"

#include <KLocalizedString>
#include <kactioncollection.h>
#include <QAction>

#include "rkstandardicons.h"
#include "rkspecialactions.h"
#include "../windows/rkmdiwindow.h"
#include "../windows/rkcommandeditorwindow.h"
#include "../windows/rkhelpsearchwindow.h"

#include "../debug.h"

QAction* RKStandardActions::copyLinesToOutput (RKMDIWindow *window, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	QAction* ret = window->standardActionCollection ()->addAction ("copy_lines_to_output", receiver, member);
	ret->setText (i18n ("Copy lines to output"));
	return ret;
}

QAction* RKStandardActions::pasteSpecial (RKMDIWindow *window, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	QAction* ret = new RKPasteSpecialAction (window->standardActionCollection ());
	window->standardActionCollection ()->addAction ("paste_special", ret);
	ret->connect (ret, SIGNAL (pasteText(QString)), receiver, member);
	window->standardActionCollection ()->setDefaultShortcut (ret, Qt::ShiftModifier + Qt::ControlModifier + Qt::Key_V);
	return ret;
}

QAction* RKStandardActions::runCurrent (RKMDIWindow *window, const QObject *receiver, const char *member, bool current_or_line) {
	RK_TRACE (MISC);

	QAction* ret = window->standardActionCollection ()->addAction ("run_current", receiver, member);
	if (current_or_line) {
		ret->setText (i18n ("Run line / selection"));
		ret->setStatusTip (i18n ("Runs the current selection (if any) or the current line (if there is no selection)"));
		ret->setToolTip (ret->statusTip ());
	} else {
		ret->setText (i18n ("Run selection"));
	}
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunLine));
	window->standardActionCollection ()->setDefaultShortcuts (ret, QList<QKeySequence>() << Qt::ControlModifier + Qt::Key_Return << Qt::ControlModifier + Qt::Key_Enter);

	return ret;
}

QAction* RKStandardActions::runAll (RKMDIWindow *window, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	QAction* ret = window->standardActionCollection ()->addAction ("run_all", receiver, member);
	ret->setText (i18n ("Run all"));
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunAll));
	window->standardActionCollection ()->setDefaultShortcuts (ret, QList<QKeySequence>() << Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_Return << Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_Enter);

	return ret;
}

class RKSearchRHelpAction : public QAction {
	Q_OBJECT
public:
	RKSearchRHelpAction (QObject *parent, RKScriptContextProvider *context_provider) : QAction (parent) {
		RK_TRACE (MISC);
		provider = context_provider;
		setText (i18n ("&Function reference"));
		setShortcut (Qt::Key_F2);
		connect (this, SIGNAL (triggered(bool)), this, SLOT (doSearch()));
	};
public slots:
	void doSearch () {
		RK_TRACE (MISC);
		QString symbol, package;
		provider->currentHelpContext (&symbol, &package);
		RKHelpSearchWindow::mainHelpSearch ()->getFunctionHelp (symbol, package);
	};
private:
	RKScriptContextProvider *provider;
};

QAction* RKStandardActions::functionHelp (RKMDIWindow *window, RKScriptContextProvider *context_provider) {
	RK_TRACE (MISC);

	QAction* ret = new RKSearchRHelpAction (window, context_provider);
	window->standardActionCollection ()->addAction ("function_reference", ret);
	return ret;
}

#include <kurifilter.h>
#include <ktoolinvocation.h>

class RKSearchOnlineHelpAction : public QAction {
	Q_OBJECT
public:
	RKSearchOnlineHelpAction (QObject *parent, RKScriptContextProvider *context_provider) : QAction (parent) {
		RK_TRACE (MISC);
		provider = context_provider;
		setText (i18n ("Search Online"));
		connect (this, SIGNAL (triggered(bool)), this, SLOT (doSearch()));
	};
public slots:
	void doSearch () {
		RK_TRACE (MISC);
		QString symbol, package;
		provider->currentHelpContext (&symbol, &package);
		KUriFilterData data (symbol + " " + package + " R");
		bool ok = KUriFilter::self ()->filterSearchUri (data, KUriFilter::NormalTextFilter);
		RK_DEBUG (MISC, DL_DEBUG, "Searching for %s in %s online -> %d: %s", qPrintable (symbol), qPrintable (package), ok, qPrintable (data.uri ().url ()));
		KToolInvocation::invokeBrowser (data.uri ().url ());
	};
private:
	RKScriptContextProvider *provider;
};

QAction* RKStandardActions::onlineHelp (RKMDIWindow *window, RKScriptContextProvider *context_provider) {
	RK_TRACE (MISC);

	// KF5 TODO: Add / replace with submenu to select search provider -> KUriFilterSearchProviderActions
	QAction* ret = new RKSearchOnlineHelpAction (window, context_provider);
	window->standardActionCollection ()->addAction ("search_online", ret);
	return ret;
}

#include "rkstandardactions.moc"
