/***************************************************************************
                          rkmessagecatalog  -  description
                             -------------------
    begin                : Mon Jun 24 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier
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

#ifndef RKMESSAGECATALOG_H
#define RKMESSAGECATALOG_H

#include <QString>
#include <QMutex>

/** This class - heavily inspired by KCatalog - wraps a gettext message catalog. Contrary to KCatalog, this does not currently support using a language other than
 * the system language, or switching languages at runtime. It allows (the base directory of) message catalogs to be at an arbitrary location.
 * 
 * Also, msgids are passed as QStrings, since in our use cases, that's the source data format, anyway.
 */
class RKMessageCatalog {
public:
	RKMessageCatalog (const QString &name, const QString &path);
	~RKMessageCatalog ();

	QString translate (const QString &msgctxt, const QString &msgid) const;
	QString translate (const QString &msgid) const;
private:
	void setup ();
	QString catalog_path;
	QByteArray catalog_name;
	bool bound;
	QMutex setup_mutex;
};

#endif
