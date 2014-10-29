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

	const int n_funs = 4;
	// NOTE that we exploit the fact that these are ordered by number of arguments from 1 to 4, below.
	QLatin1String funs [n_funs] = { 
		QLatin1String ("i18n(msgid)"), 
		QLatin1String ("i18nc(msgctxt, msgid)"), 
		QLatin1String ("i18np(msgid, msgid_plural, count)"), 
		QLatin1String ("i18ncp(msgctxt, msgid, msgid_plural, count)")
	};
	QString eval;
	for (int i = 0; i < n_funs; ++i) {
		eval += QLatin1String ("function ") + funs[i] + QLatin1String ("{ ret = _i18n.") + funs[i] + ";\n"
		      + QLatin1String ("argn=") + ((i < 2) ? QLatin1String ("1") : QLatin1String ("2")) + ";\n"
		      + QLatin1String ("for (i=") + QString::number (i+1) + QLatin1String ("; i<arguments.length; i++) ")
			  + QLatin1String ("ret = ret.replace(new RegExp(\"%\"+argn++, 'g'), arguments[i]);\n")
			  + QLatin1String ("return(ret); }\n");
	}
	RK_DEBUG (PHP, DL_DEBUG, "Evaluating i18n glue code:\n%s", qPrintable (eval));
	engine->evaluate (eval);
}

#include "qtscripti18n.moc"
