/*
rkmessagecatalog - This file is part of the RKWard project. Created: Mon Jun 24 2013
SPDX-FileCopyrightText: 2013-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKMESSAGECATALOG_H
#define RKMESSAGECATALOG_H

#include <QString>
#include <QMutex>
#include <QHash>

/** This class - heavily inspired by KCatalog - wraps a gettext message catalog. Contrary to KCatalog, this does not currently support using a language other than
 * the system language, or switching languages at runtime. It allows (the base directory of) message catalogs to be at an arbitrary location, however, which is
 * important for plugins (installed at runtime and outside of system path)
 * 
 * Also, msgids are passed as QStrings, since in our use cases, that's the source data format, anyway.
 */
class RKMessageCatalog {
public:
	QString translate(const QString &msgid, const QStringList &args=QStringList()) const;
	QString translate(const QString &msgctxt, const QString &msgid, const QStringList &args=QStringList()) const;
	QString translate(const QString &msgid_singular, const QString &msgid_plural, unsigned long int count, const QStringList &args=QStringList()) const;
	QString translate(const QString &msgctxt, const QString &msgid_singular, const QString &msgid_plural, unsigned long int count, const QStringList &args=QStringList()) const;

/** Get the catalog identified by name. This could be an already open catalog, or a new one. In the latter case, the catalog is expected at pathhint. In the former case, pathhint is ignored. This function is guaranteed to return a non-null RKMessageCatalog, although that does not imply the catalog could actually be loaded. */
	static RKMessageCatalog *getCatalog (const QString &name, const QString &pathhint);
/** Returns a dummy null-catalog */
	static RKMessageCatalog *nullCatalog ();
/** Switch language to use for any coming translations */
	static void switchLanguage (const QString &new_language_code);
private:
	RKMessageCatalog (const QString &name, const QString &path);
	~RKMessageCatalog ();

	QByteArray catalog_name;

	class CatalogHash {
		QHash<QString, RKMessageCatalog*> catalogs;
		QMutex setup_mutex;
	public:
		CatalogHash () {};
		~CatalogHash ();
		RKMessageCatalog* getCatalog (const QString& name, const QString& pathhint);
	};
	static CatalogHash catalogs;
};

#endif
