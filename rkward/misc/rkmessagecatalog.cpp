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

#include "rkmessagecatalog.h"

#include <libintl.h>
#include <QFile>

#include "../debug.h"

RKMessageCatalog::RKMessageCatalog (const QString &name, const QString& path) {
	RK_TRACE (MISC);

	bound = false;
	catalog_path = path;
	catalog_name = QFile::encodeName (name);
}

RKMessageCatalog::~RKMessageCatalog () {
	RK_TRACE (MISC);
}

// Adopted from KDE's gettext.h
/* The separator between msgctxt and msgid in a .mo file. */
#define GETTEXT_CONTEXT_GLUE "\004"

QString RKMessageCatalog::translate (const QString &msgctxt, const QString &msgid) const {
	RK_TRACE (MISC);
	if (!bound) const_cast<RKMessageCatalog *> (this)->setup ();

	QByteArray key = (msgctxt + GETTEXT_CONTEXT_GLUE + msgid).toUtf8 ();
	const char *trans = dgettext (catalog_name, key);
	if (trans == key) return msgid;
	return QString::fromUtf8 (trans);
}

QString RKMessageCatalog::translate (const QString &msgid) const {
	RK_TRACE (MISC);
	if (!bound) const_cast<RKMessageCatalog *> (this)->setup ();

	return QString::fromUtf8 (dgettext (catalog_name, msgid.toUtf8 ()));
}

void RKMessageCatalog::setup () {
	RK_TRACE (MISC);

	setup_mutex.lock ();
	if (!bound) {
		bindtextdomain (catalog_name, QFile::encodeName (catalog_path));
		bind_textdomain_codeset (catalog_name, "UTF-8");
		bound = true;
	}
	setup_mutex.unlock ();
}
