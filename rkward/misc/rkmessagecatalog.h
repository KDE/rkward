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
	QString translate (const QString &msgid) const;
	QString translate (const QString &msgctxt, const QString &msgid) const;
	QString translate (const QString &msgid_singular, const QString &msgid_plural, unsigned long int count) const;
	QString translate (const QString &msgctxt, const QString &msgid_singular, const QString &msgid_plural, unsigned long int count) const;

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
