/*
rkmessagecatalog - This file is part of RKWard (https://rkward.kde.org). Created: Mon Jun 24 2013
SPDX-FileCopyrightText: 2013-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkmessagecatalog.h"

#include <QFile>
#include <KLocalizedString>
#include <KLazyLocalizedString>

#include "../debug.h"

// statics
RKMessageCatalog::CatalogHash RKMessageCatalog::catalogs;

RKMessageCatalog::RKMessageCatalog (const QString &name, const QString& path) {
	RK_TRACE (MISC);

	catalog_name = QFile::encodeName (name);
	if (!path.isEmpty ()) {
		RK_DEBUG (MISC, DL_DEBUG, "Registering the path %s for catalog %s", qPrintable (path), qPrintable (name));
		KLocalizedString::addDomainLocaleDir (catalog_name, path);
	}
}

RKMessageCatalog::~RKMessageCatalog () {
	RK_TRACE (MISC);
}

QString RKMessageCatalog::translate(const QString &msgctxt, const QString &msgid, const QStringList &args) const {
	RK_TRACE(MISC);
	auto ret = ki18ndc(catalog_name.constData(), msgctxt.toUtf8().constData(), msgid.toUtf8().constData());
	for(int i = 0; i < args.size(); ++i) ret = ret.subs(args[i]);
	return ret.toString();
}

QString RKMessageCatalog::translate(const QString &msgctxt, const QString &msgid_singular, const QString &msgid_plural, unsigned long int count, const QStringList &args) const {
	RK_TRACE(MISC);
	auto ret = ki18ndcp(catalog_name.constData(), msgctxt.toUtf8().constData(), msgid_singular.toUtf8().constData(), msgid_plural.toUtf8().constData()).subs(count);
	for(int i = 0; i < args.size(); ++i) ret = ret.subs(args[i]);
	return ret.toString();
}

QString RKMessageCatalog::translate(const QString &msgid, const QStringList &args) const {
	RK_TRACE(MISC);
	auto ret = ki18nd(catalog_name.constData(), msgid.toUtf8().constData());
	for(int i = 0; i < args.size(); ++i) ret = ret.subs(args[i]);
	return ret.toString();
}

QString RKMessageCatalog::translate(const QString &msgid_singular, const QString &msgid_plural, unsigned long int count, const QStringList &args) const {
	RK_TRACE(MISC);
	auto ret = ki18ndp(catalog_name.constData(), msgid_singular.toUtf8().constData(), msgid_plural.toUtf8().constData()).subs(count);
	for(int i = 0; i < args.size(); ++i) ret = ret.subs(args[i]);
	return ret.toString();
}

RKMessageCatalog* RKMessageCatalog::CatalogHash::getCatalog (const QString& name, const QString& pathhint) {
	RK_TRACE (MISC);

	RKMessageCatalog *ret = catalogs.value (name, nullptr);
	if (ret) return ret;
	setup_mutex.lock ();
		// try to look up again, in case initialized from another thread
		ret = catalogs.value (name, nullptr);
		if (!ret) {
			ret = new RKMessageCatalog (name, pathhint);
			catalogs.insert (name, ret);
		}
	setup_mutex.unlock ();
	return ret;
}

RKMessageCatalog::CatalogHash::~CatalogHash() {
	RK_TRACE (MISC);

	QHash<QString, RKMessageCatalog*>::const_iterator it;
	for (it = catalogs.constBegin (); it != catalogs.constEnd (); ++it) {
		delete (it.value ());
	}
}

// static
RKMessageCatalog* RKMessageCatalog::getCatalog (const QString& name, const QString& pathhint) {
	RK_TRACE (MISC);

	return catalogs.getCatalog (name, pathhint);
}

RKMessageCatalog* RKMessageCatalog::nullCatalog () {
	// ok, not thread-safe, here, but the worst that can happen is creating more than one dummy catalog.
	return (getCatalog  ("rkward_dummy", QString ()));
}

// static
void RKMessageCatalog::switchLanguage (const QString &new_language_code) {
	RK_TRACE (MISC);

	KLocalizedString::setLanguages (QStringList () << new_language_code);
}
