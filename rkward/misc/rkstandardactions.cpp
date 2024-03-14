/*
rkstandardactions - This file is part of RKWard (https://rkward.kde.org). Created: Sun Nov 18 2007
SPDX-FileCopyrightText: 2007-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkstandardactions.h"

#include <KLocalizedString>
#include <kactioncollection.h>
#include <QAction>
#include <QDesktopServices>

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
	window->standardActionCollection ()->setDefaultShortcut (ret, Qt::ShiftModifier | Qt::ControlModifier | Qt::Key_V);
	return ret;
}

QAction* RKStandardActions::runCurrent (RKMDIWindow *window, const QObject *receiver, const char *member, bool current_or_line) {
	RK_TRACE (MISC);

	QAction* ret = window->standardActionCollection ()->addAction ("run_current", receiver, member);
	if (current_or_line) {
		ret->setText (i18n ("Run line / selection"));
		ret->setWhatsThis(i18n ("Runs the current selection (if any) or the current line (if there is no selection)"));
		ret->setToolTip (ret->statusTip ());
	} else {
		ret->setText (i18n ("Run selection"));
	}
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunLine));
	window->standardActionCollection ()->setDefaultShortcuts (ret, {
		Qt::ControlModifier | Qt::Key_Return,
		Qt::ControlModifier | Qt::Key_Enter
	});

	return ret;
}

QAction* RKStandardActions::runAll (RKMDIWindow *window, const QObject *receiver, const char *member) {
	RK_TRACE (MISC);

	QAction* ret = window->standardActionCollection ()->addAction ("run_all", receiver, member);
	ret->setText (i18n ("Run all"));
	ret->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionRunAll));
	window->standardActionCollection ()->setDefaultShortcuts (ret, {
		Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Return,
		Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Enter
	});

	return ret;
}

class RKSearchRHelpAction : public QAction {
	Q_OBJECT
public:
	RKSearchRHelpAction (QObject *parent, RKScriptContextProvider *context_provider) : QAction (parent) {
		RK_TRACE (MISC);
		provider = context_provider;
		setText (i18n ("&Function reference"));
		connect (this, &QAction::triggered, this, &RKSearchRHelpAction::doSearch);
	};
public Q_SLOTS:
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
	window->standardActionCollection ()->setDefaultShortcut (ret, Qt::Key_F2);
	return ret;
}

#include <KIO/KUriFilterSearchProviderActions>
#include <QMenu>

class RKSearchOnlineHelpAction : public QObject {
	Q_OBJECT
public:
	RKSearchOnlineHelpAction (QObject *parent, RKScriptContextProvider *context_provider) : QObject (parent) {
		RK_TRACE (MISC);
		provider = context_provider;
		menu = new QMenu ();
		container = nullptr;
		connect (menu, &QMenu::aboutToShow, this, &RKSearchOnlineHelpAction::init);
		actions = new KIO::KUriFilterSearchProviderActions (this);
	};
	~RKSearchOnlineHelpAction () {
		RK_TRACE (MISC);
		menu->deleteLater();
		if (container) container->deleteLater();
	}
	QAction *action () {
		return menu->menuAction ();
	}
public Q_SLOTS:
	void init () {
		RK_TRACE (MISC);
		QString symbol, package;
		bool enabled = true;
		provider->currentHelpContext (&symbol, &package);
		QString searchtext = symbol + " " + package + " R";
		actions->setSelectedText(searchtext);
		menu->clear();

		if (symbol.isEmpty()) {
			menu->addSection(i18n("[No keyword detected]"));
			enabled = false;
		} else {
			menu->addSection(searchtext);
		}

		// Coerce WebshortcutsMenu to single level
		if (container) container->deleteLater();
		container = new QMenu;
		actions->addWebShortcutsToMenu(container);
		QList<QAction*> actions;
		auto cactions = container->actions();
		if (cactions.count() == 1) {
			QMenu *internal = cactions.first()->menu();
			if (internal) actions = internal->actions();
		}
		if (actions.isEmpty()) actions = cactions;
		for (int i = 0; i < actions.count(); ++i) {
			menu->addAction(actions[i]);
			actions[i]->setEnabled(enabled || (i == actions.count() - 1));
		}
	};
private:
	QMenu *menu;
	QMenu *container;
	KIO::KUriFilterSearchProviderActions *actions;
	RKScriptContextProvider *provider;
};

QAction* RKStandardActions::onlineHelp (RKMDIWindow *window, RKScriptContextProvider *context_provider) {
	RK_TRACE (MISC);

	auto *a = new RKSearchOnlineHelpAction (window, context_provider);
	QAction* ret = a->action ();
	ret->setText(i18n("Search online"));
	window->standardActionCollection ()->addAction ("search_online", ret);
	return ret;
}

#include "rkstandardactions.moc"
