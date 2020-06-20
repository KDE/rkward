/***************************************************************************
                          rkmessagecatalog  -  description
                             -------------------
    begin                : Mon Jun 24 2013
    copyright            : (C) 2013-2018 by Thomas Friedrichsmeier
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

#include "rkmessagecatalog.h"

#include <libintl.h>
#include <QFile>
#include <QLocale>
#include <KLocalizedString>

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

// Adopted from KDE's gettext.h
/* The separator between msgctxt and msgid in a .mo file. */
#define GETTEXT_CONTEXT_GLUE "\004"

QString RKMessageCatalog::translate (const QString &msgctxt, const QString &msgid) const {
	RK_TRACE (MISC);

	return i18ndc (catalog_name, msgctxt.toUtf8 (), msgid.toUtf8 ());
}

QString RKMessageCatalog::translate (const QString &msgctxt, const QString &msgid_singular, const QString &msgid_plural, unsigned long int count) const {
	RK_TRACE (MISC);

	return i18ndcp (catalog_name, msgctxt.toUtf8 (), msgid_singular.toUtf8 (), msgid_plural.toUtf8 (), count);
}

QString RKMessageCatalog::translate (const QString &msgid) const {
	RK_TRACE (MISC);

	return i18nd (catalog_name, msgid.toUtf8 ());
}

QString RKMessageCatalog::translate (const QString &msgid_singular, const QString &msgid_plural, unsigned long int count) const {
	RK_TRACE (MISC);

	return i18ndp (catalog_name, msgid_singular.toUtf8 (), msgid_plural.toUtf8 (), count);
}

RKMessageCatalog* RKMessageCatalog::CatalogHash::getCatalog (const QString& name, const QString& pathhint) {
	RK_TRACE (MISC);

	RKMessageCatalog *ret = catalogs.value (name, 0);
	if (ret) return ret;
	setup_mutex.lock ();
		// try to look up again, in case initialized from another thread
		ret = catalogs.value (name, 0);
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

#ifdef Q_OS_WIN
	extern "C" int __declspec(dllimport) _nl_msg_cat_cntr;
#endif

// static
void RKMessageCatalog::switchLanguage (const QString &new_language_code) {
	RK_TRACE (MISC);

	KLocalizedString::setLanguages (QStringList () << new_language_code);
}
