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

static QString getId(const QVariantList &l) {
	return l.value(0).toString();
}

static QVariantList getChildlist(const QVariantList &l) {
	return l.value(1).toList();
}

static QString getLabel(const QVariantList &l) {
	return l.value(2).toString();
}

static bool getCallable(const QVariantList &l) {
	return l.value(3).toList().value(0).toBool();
}

static QDomElement addChildElement(QDomNode &parent, const QString &tagname, const QString &nameattribute) {
	QDomDocument doc = parent.isDocument() ? parent.toDocument() : parent.ownerDocument();
	auto ret = doc.createElement(tagname);
	if (!nameattribute.isNull()) ret.setAttribute(QStringLiteral("name"), nameattribute);
	parent.appendChild(ret);
	return ret;
}

void RKRApiMenu::makeAction(QDomElement e, const QString &full_id, const QString &label, QStringList *actionlist) {
	auto s = addChildElement(e, QStringLiteral("Action"), full_id);
	auto t = addChildElement(s, QStringLiteral("Text"), QString());
	t.appendChild(s.ownerDocument().createTextNode(label));

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

void RKRApiMenu::makeXML(QDomElement e, const QVariantList &l, const QString &path, QStringList *actionlist) {
	const auto id = getId(l);
	const QString full_id = path.isEmpty() ? id : path + ',' + id;
	const auto label = getLabel(l);
	const auto callable = getCallable(l);

	if (callable) {
		makeAction(e, full_id, label, actionlist);
	} else {
		auto s = addChildElement(e, QStringLiteral("Menu"), id);
		if (!label.isEmpty()) {
			auto t = addChildElement(s, QStringLiteral("Text"), QString());
			t.appendChild(s.ownerDocument().createTextNode(label));
		}
		const auto children = getChildlist(l);
		for (const auto &child : children) {
			makeXML(s, child.toList(), full_id, actionlist);
		}
	}
}

void RKRApiMenu::commit() {
	if (rep.isEmpty()) return;
	RK_TRACE(MISC);

	QStringList actionlist;
	QDomDocument doc("kpartgui");
	auto r = addChildElement(doc, QStringLiteral("kpartgui"), QStringLiteral("rapi_menu"));
	auto mb = addChildElement(r,  QStringLiteral("MenuBar"), QString());
	const auto menus = getChildlist(rep);
	for (const auto &menu : menus) {
		const auto menuList = menu.toList();
		if (getId(menuList) == "toolbar") {
			auto tb = addChildElement(r, QStringLiteral("ToolBar"), QStringLiteral("mainToolBar"));
			const auto tb_children = getChildlist(menuList);
			for (const auto &tbit : tb_children) {
				makeXML(tb, tbit.toList(), QStringLiteral("toolbar"), &actionlist);
			}
		} else {
			makeXML(mb, menuList, QString(), &actionlist);
		}
	}

	auto f = RKWardMainWindow::getMain()->factory();
	f->removeClient(this);
	qDebug("%s", qPrintable(doc.toString()));
	setXMLGUIBuildDocument(doc);

	// delete any actions that are no longer around
	const auto actions = actionCollection()->actions();
	for (const auto &action : actions) {
		if (!actionlist.contains(action->objectName())) {
			delete actionCollection()->takeAction(action);
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
