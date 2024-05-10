/*
rkrecenturls - This file is part of the RKWard project. Created: Sat Apr 16 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKRECENTURLS_H
#define RKRECENTURLS_H

#include <QString>
#include <QHash>
#include <QUrl>
#include <QObject>

class KRecentFilesAction;

/** A wrapper around KRecentFilesAction to have a uniform handling across all the places where we remember the last used urls. */
class RKRecentUrls : public QObject {
	Q_OBJECT
public:
/** @see mostRecentUrl() */
	static void addRecentUrl(const QString &id, const QUrl &url);
/** get the url last used for @param id, where id is simply a key string.
    Urls are saved and restored across sessions, _except_ for the empty key, which will keep track of the last used
    "generic" url within a session (and defaults to the current working directory, while not set). */
	static QUrl mostRecentUrl(const QString &id);
	static QList<QUrl> allRecentUrls(const QString &id);
	static KRecentFilesAction* claimAction(const QString &id);
	static void saveConfig();
	static QString scriptsId();
	static QString workspaceId();
	static QString outputId();
	static RKRecentUrls* notifier();
Q_SIGNALS:
	void recentUrlsChanged();
private:
	explicit RKRecentUrls(QObject* parent);
	~RKRecentUrls();
	void notifyChange();
	static void notifyChangeProxy();
	static QHash<QString, KRecentFilesAction*> actions;
	static KRecentFilesAction* action(const QString &id);
	static RKRecentUrls* _notifier;
};

#endif
