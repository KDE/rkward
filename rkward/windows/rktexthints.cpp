/*
rktexthints - This file is part of the RKWard project. Created: Sat Jun 17 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rktexthints.h"

#include <KLocalizedString>

#include "../misc/rkcommonfunctions.h"
#include "../core/robjectlist.h"

#include "../debug.h"

RKTextHints::RKTextHints(KTextEditor::View *view) : QObject(view), KTextEditor::TextHintProvider() {
	RK_TRACE(COMMANDEDITOR);
	auto iface = qobject_cast<KTextEditor::TextHintInterface*>(view);
	if (iface) {
		iface->registerTextHintProvider(this);
	} else {
		RK_ASSERT(iface);
	}
}

RKTextHints::~RKTextHints() {
	RK_TRACE(COMMANDEDITOR);
}

QString RKTextHints::textHint(KTextEditor::View *view, const KTextEditor::Cursor &position) {
	RK_TRACE(COMMANDEDITOR);
	QString line = view->document()->line(position.line()) + ' ';
	QString symbol = RKCommonFunctions::getCurrentSymbol(line, position.column());
	auto obj = RObjectList::getObjectList()->findObject(symbol);
	if (obj) {
		return i18n("Symbol <i>%1</i> might refer to: ", symbol) + obj->getObjectDescription();
	}
	return QString();
}

