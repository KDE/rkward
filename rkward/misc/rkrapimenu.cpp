/*
rkrapimenu - This file is part of the RKWard project. Created: Wed Aug 07 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrapimenu.h"

#include <QDomElement>
#include <QDomDocument>
#include <QAction>
#include <QTimer>

#include <KActionCollection>
#include <KXMLGUIFactory>

#include "../rbackend/rkrinterface.h"
#include "../core/robject.h"
#include "../rkward.h"

#include "../debug.h"

RKRApiMenu::RKRApiMenu() : KXMLGUIClient() {
	RK_TRACE(MISC);
}

RKRApiMenu::~RKRApiMenu() {
	RK_TRACE(MISC);
}

void RKRApiMenu::makeXML(QDomDocument &doc, QDomElement e, const QVariantList &l, const QString &path, QStringList *actionlist) {
	const auto id = l.value(0).toString();
	const QString full_id = path.isEmpty() ? id : path + ',' + id;
	const auto children = l.value(1).toList();
	const auto label = l.value(2).toString();
	const auto callable = l.value(3).toList().value(0).toBool();
	auto s = doc.createElement(callable ? "Action" : "Menu");
	s.setAttribute("name", callable ? full_id : id);
	if (!label.isEmpty()) {
		auto t = doc.createElement("Text");
		t.appendChild(doc.createTextNode(label));
		s.appendChild(t);
	}
	for (auto it = children.constBegin(); it != children.constEnd(); ++it) {
		RK_ASSERT(!callable);
		makeXML(doc, s, (*it).toList(), full_id, actionlist);
	}
	e.appendChild(s);

	if (callable) {
		auto a = action(full_id);
		if (!a) {
			a = new QAction();
			a->setObjectName(full_id);
			actionCollection()->addAction(full_id, a);
			QObject::connect(a, &QAction::triggered, a, [full_id]() {
				QString path;
				auto segments = full_id.split(',');
				for (int i = 0; i < segments.size(); ++i) {
					if (i) path += ',';
					path += RObject::rQuote(segments[i]);
				}
				RInterface::issueCommand(new RCommand("rk.menu()$item(" + path + ")$call()", RCommand::App));
			});
		}
		a->setText(label);
		actionlist->append(full_id);
	}
}

void RKRApiMenu::commit() {
	if (rep.isEmpty()) return;
	RK_TRACE(MISC);

	auto f = RKWardMainWindow::getMain()->factory();
	f->removeClient(this);
	QStringList actionlist;
	QDomDocument doc;
	auto menus = rep.value(1).toList();
	auto r = doc.createElement("kpartgui");
	r.setAttribute("name", "rapi_menu");  // TODO: version attribute
	auto mb = doc.createElement("MenuBar");
	for (auto it = menus.constBegin(); it != menus.constEnd(); ++it) {
		makeXML(doc, mb, (*it).toList(), QString(), &actionlist);
	}
	r.appendChild(mb);
	doc.appendChild(r);
	setXMLGUIBuildDocument(doc);

	// delete any actions that are no longer around
	auto all_actions = actionCollection()->actions();
	for (int i = 0; i < all_actions.size(); ++i) {
		if (!actionlist.contains(all_actions[i]->objectName())) {
			delete (actionCollection()->takeAction(all_actions[i]));
		}
	}

	f->addClient(this);
	rep = QVariantList();
}

void RKRApiMenu::updateFromR(const QVariantList &_rep) {
	RK_TRACE(MISC);

	// Actual update is done debounced
	if (rep.isEmpty()) QTimer::singleShot(100, factory(), [this](){ commit(); });
	rep = _rep;
}

QAction *RKRApiMenu::actionByPath(const QStringList &path) {
	commit(); // force commit before lookup
	return action(path.join(','));
}

void RKRApiMenu::enableAction(const QStringList &path, bool enable, bool show) {
	RK_TRACE(MISC);
	auto a = actionByPath(path);
	if (a) {
		a->setEnabled(enable);
		a->setVisible(show);
	}
}
