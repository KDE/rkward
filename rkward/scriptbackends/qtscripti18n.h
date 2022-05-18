/*
qtscripti18n - This file is part of the RKWard project. Created: Wed Oct 29 2014
SPDX-FileCopyrightText: 2014-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QTSCRIPTI18N_H
#define QTSCRIPTI18N_H

#include <QObject>

#include "qtscriptbackend.h"  // TODO: For QJSEngine, only. Move back to .cpp-file

#include "../misc/rkmessagecatalog.h"

/** A QObject wrapper around RKMessageCatalog. Meant for use in RKComponentScripting and QtScriptBackend */
class RKMessageCatalogObject : public QObject {
	Q_OBJECT
public:
	RKMessageCatalogObject (const RKMessageCatalog *_catalog, QObject *parent) : QObject (parent), catalog (_catalog) {};
	virtual ~RKMessageCatalogObject () {};

	Q_INVOKABLE QString i18n(const QString &msgid, const QStringList &args) const {
		return (catalog->translate(msgid, args));
	};
	Q_INVOKABLE QString i18nc(const QString &msgctxt, const QString &msgid, const QStringList &args) const {
		return (catalog->translate(msgctxt, msgid, args));
	};
	Q_INVOKABLE QString i18np(const QString &msgid_singular, const QString &msgid_plural, uint count, const QStringList &args) const {
		return (catalog->translate(msgid_singular, msgid_plural, count, args));
	};
	Q_INVOKABLE QString i18ncp(const QString &msgctxt, const QString &msgid_singular, const QString &msgid_plural, uint count, const QStringList &args) const {
		return (catalog->translate(msgctxt, msgid_singular, msgid_plural, count, args));
	};
/** Add an RKMessageCatalog, and the required glue code to the given QScriptEngine. */
	static void addI18nToScriptEngine (QJSEngine *engine, const RKMessageCatalog *catalog);
private:
	const RKMessageCatalog *catalog;
};



#endif
