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

#include "rksettingsmodulecommandeditor.h"

#include "../debug.h"

static QString _scripts_id("rscripts");
QString RKRecentUrls::scriptsId() { return _scripts_id; }
static QString _workspace_id("workspaces");
QString RKRecentUrls::workspaceId() { return _workspace_id; }
static QString _output_id("rkoutput");
QString RKRecentUrls::outputId() { return _output_id; }

QHash<QString, KRecentFilesAction*> RKRecentUrls::actions;

void RKRecentUrls::addRecentUrl(const QString& id, const QUrl& url) {
	RK_TRACE(SETTINGS);
	action(id)->addUrl(url);
}

QUrl RKRecentUrls::mostRecentUrl(const QString& id) {
	RK_TRACE(SETTINGS);
	if (id.isEmpty() && !actions.contains(id)) {
		return QUrl::fromLocalFile(QDir::currentPath());
	}
	return action(id)->urls().value(0);
}

RKRecentUrls::RKRecentUrls() {
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
	auto ret = action(id);
	RK_ASSERT(!ret->parent());
	return ret;
}

KRecentFilesAction * RKRecentUrls::action(const QString& id) {
	RK_TRACE(SETTINGS);
	if (!actions.contains(id)) {
		auto cg = config();
		auto act = new KRecentFilesAction(nullptr);
		if (!id.isEmpty()) act->loadEntries(cg.group(id));
		act->setMaxItems(RKSettingsModuleCommandEditor::maxNumRecentFiles());  // TODO: Move setting somewhere else
		QObject::connect(act, &QObject::destroyed, [id]() { RKRecentUrls::actions.remove(id); });
		actions.insert(id, act);
	}
	return actions[id];
}

void RKRecentUrls::cleanup() {
	for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
		(*it)->deleteLater();
	}
	actions.clear();
}
