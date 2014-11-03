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

#include "qtscripti18n.h"

#include <QScriptEngine>

#include "../debug.h"

void RKMessageCatalogObject::addI18nToScriptEngine (QScriptEngine* engine, const RKMessageCatalog* catalog) {
	QScriptValue handle = engine->newQObject (new RKMessageCatalogObject (catalog, engine));
	engine->globalObject ().setProperty ("_i18n", handle);
}

#include "qtscripti18n.moc"
