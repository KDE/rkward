/*
qtscripti18n - This file is part of RKWard (https://rkward.kde.org). Created: Wed Oct 29 2014
SPDX-FileCopyrightText: 2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtscripti18n.h"

#include "../debug.h"

void RKMessageCatalogObject::addI18nToScriptEngine (QJSEngine* engine, const RKMessageCatalog* catalog) {
	auto handle = engine->newQObject (new RKMessageCatalogObject (catalog, engine));
	engine->globalObject ().setProperty ("_i18n", handle);
}

