/***************************************************************************
                          rkmessagecatalog  -  description
                             -------------------
    begin                : Mon Jun 24 2013
    copyright            : (C) 2013, 2014 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
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

#include "../debug.h"

// statics
QHash<QString, RKMessageCatalog*> RKMessageCatalog::catalogs;
QMutex RKMessageCatalog::setup_mutex;
RKMessageCatalog* RKMessageCatalog::null_catalog = 0;

RKMessageCatalog::RKMessageCatalog (const QString &name, const QString& path) {
	RK_TRACE (MISC);

	catalog_name = QFile::encodeName (name);
	char *res = bindtextdomain (catalog_name, QFile::encodeName (path));
	RK_DO (qDebug ("Opening catalog %s, expected at %s, found at %s", qPrintable (name), qPrintable (path), res), MISC, DL_WARNING);
	bind_textdomain_codeset (catalog_name, "UTF-8");
}

RKMessageCatalog::~RKMessageCatalog () {
	RK_TRACE (MISC);
}

// Adopted from KDE's gettext.h
/* The separator between msgctxt and msgid in a .mo file. */
#define GETTEXT_CONTEXT_GLUE "\004"

QString RKMessageCatalog::translate (const QString &msgctxt, const QString &msgid) const {
	RK_TRACE (MISC);

	QByteArray key = (msgctxt + GETTEXT_CONTEXT_GLUE + msgid).toUtf8 ();
	const char *trans = dgettext (catalog_name, key);
	if (trans == key) return msgid;
	return QString::fromUtf8 (trans);
}

QString RKMessageCatalog::translate (const QString &msgctxt, const QString &msgid_singular, const QString &msgid_plural, unsigned long int count) const {
	RK_TRACE (MISC);

	QByteArray key = (msgctxt + GETTEXT_CONTEXT_GLUE + msgid_singular).toUtf8 ();
	QByteArray pkey = msgid_plural.toUtf8 ();
	const char *trans = dngettext (catalog_name, key, pkey, count);
	if ((trans == key) || (trans == pkey)) {
		if (count == 1) return msgid_singular.arg (count);
		return msgid_plural.arg (count);
	}
	return QString::fromUtf8 (trans).arg (count);
}

QString RKMessageCatalog::translate (const QString &msgid) const {
	RK_TRACE (MISC);

	return QString::fromUtf8 (dgettext (catalog_name, msgid.toUtf8 ()));
}

QString RKMessageCatalog::translate (const QString &msgid_singular, const QString &msgid_plural, unsigned long int count) const {
	RK_TRACE (MISC);

	return QString::fromUtf8 (dngettext (catalog_name, msgid_singular.toUtf8 (), msgid_plural.toUtf8 (), count)).arg (count);
}

// static
RKMessageCatalog* RKMessageCatalog::getCatalog (const QString& name, const QString& pathhint) {
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

RKMessageCatalog* RKMessageCatalog::nullCatalog () {
	// ok, not thread-safe, here, but the worst that can happen is creating more than one dummy catalog.
	if (!null_catalog) null_catalog = getCatalog  ("rkward_dummy", QString ());
	return null_catalog;
}
