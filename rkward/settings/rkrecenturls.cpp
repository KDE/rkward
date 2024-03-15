/*
rkrecenturls - This file is part of the RKWard project. Created: Sat Apr 16 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrecenturls.h"

#include <QDir>

#include <KSharedConfig>
#include <KConfigGroup>
#include <KRecentFilesAction>
#include <kconfigwidgets_version.h>

#include "rksettingsmodulegeneral.h"
#include "../rkward.h"

#include "../debug.h"

static QString _scripts_id("rscripts");  // clazy:exclude=non-pod-global-static
QString RKRecentUrls::scriptsId() { return _scripts_id; }
static QString _workspace_id("workspaces");  // clazy:exclude=non-pod-global-static
QString RKRecentUrls::workspaceId() { return _workspace_id; }
static QString _output_id("rkoutput");  // clazy:exclude=non-pod-global-static
QString RKRecentUrls::outputId() { return _output_id; }

QHash<QString, KRecentFilesAction*> RKRecentUrls::actions;
RKRecentUrls* RKRecentUrls::_notifier = nullptr;

void RKRecentUrls::addRecentUrl(const QString& id, const QUrl& url) {
	RK_TRACE(SETTINGS);
	action(id)->addUrl(url);
	notifyChangeProxy();
}

QUrl RKRecentUrls::mostRecentUrl(const QString& id) {
	RK_TRACE(SETTINGS);
	if (id.isEmpty() && !actions.contains(id)) {
		return QUrl::fromLocalFile(QDir::currentPath());
	}
	auto list = allRecentUrls(id);
	if (list.isEmpty()) return QUrl::fromLocalFile(QDir::currentPath());
	return list.first();
}

QList<QUrl> RKRecentUrls::allRecentUrls(const QString& id) {
	return action(id)->urls();
}

RKRecentUrls* RKRecentUrls::notifier() {
	RK_TRACE(SETTINGS);
	if (!_notifier) {
		_notifier = new RKRecentUrls(RKWardMainWindow::getMain());
	}
	return _notifier;
}

void RKRecentUrls::notifyChangeProxy() {
	if (_notifier) _notifier->notifyChange();
}

void RKRecentUrls::notifyChange() {
	Q_EMIT recentUrlsChanged();
}

RKRecentUrls::RKRecentUrls(QObject* parent) : QObject(parent) {
	RK_TRACE(SETTINGS);
	// Not currrently used
}

RKRecentUrls::~RKRecentUrls() {
	RK_TRACE(SETTINGS);
}

static KConfigGroup config() {
	auto config = KSharedConfig::openConfig();
	return config->group(QStringLiteral("Recent Files"));
}

void RKRecentUrls::saveConfig() {
	RK_TRACE(SETTINGS);
	auto cg = config();
	for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
		if (it.key().isEmpty()) continue;
		it.value()->saveEntries(cg.group(it.key()));
	}
}

KRecentFilesAction* RKRecentUrls::claimAction(const QString& id) {
	RK_TRACE(SETTINGS);
	return action(id);
}

KRecentFilesAction * RKRecentUrls::action(const QString& id) {
	RK_TRACE(SETTINGS);
	if (!actions.contains(id)) {
		auto cg = config();
		auto act = new KRecentFilesAction(RKWardMainWindow::getMain());
		if (!id.isEmpty()) act->loadEntries(cg.group(id));
		act->setMaxItems(RKSettingsModuleGeneral::maxNumRecentFiles());  // TODO: Move setting somewhere else
		QObject::connect(act, &QObject::destroyed, [id]() { RKRecentUrls::actions.remove(id); });
		QObject::connect(act, &KRecentFilesAction::recentListCleared, &RKRecentUrls::notifyChangeProxy);
		actions.insert(id, act);
	}
	return actions[id];
}

