/***************************************************************************
                          qtscripti18n  -  description
                             -------------------
    begin                : Wed Oct 29 2014
    copyright            : (C) 2014 by Thomas Friedrichsmeier
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

#ifndef QTSCRIPTI18N_H
#define QTSCRIPTI18N_H

#include "../misc/rkmessagecatalog.h"

class QScriptEngine;
/** A QObject wrapper around RKMessageCatalog. Meant for use in RKComponentScripting and QtScriptBackend */
class RKMessageCatalogObject : public QObject {
	Q_OBJECT
public:
	RKMessageCatalogObject (const RKMessageCatalog *_catalog, QObject *parent) : QObject (parent), catalog (_catalog) {};
	virtual ~RKMessageCatalogObject () {};

	Q_INVOKABLE QString i18n (const QString &msgid) const {
		return (catalog->translate (msgid));
	};
	Q_INVOKABLE QString i18nc (const QString &msgctxt, const QString &msgid) const {
		return (catalog->translate (msgctxt, msgid));
	};
	Q_INVOKABLE QString i18np (const QString &msgid_singular, const QString &msgid_plural, uint count) const {
		return (catalog->translate (msgid_singular, msgid_plural, count));
	};
	Q_INVOKABLE QString i18ncp (const QString &msgctxt, const QString &msgid_singular, const QString &msgid_plural, uint count) const {
		return (catalog->translate (msgctxt, msgid_singular, msgid_plural, count));
	};
/** Add an RKMessageCatalog, and the required glue code to the given QScriptEngine. */
	static void addI18nToScriptEngine (QScriptEngine *engine, const RKMessageCatalog *catalog);
private:
	const RKMessageCatalog *catalog;
};



#endif
