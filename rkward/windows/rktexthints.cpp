/*
rktexthints - This file is part of the RKWard project. Created: Sat Jun 17 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rktexthints.h"

#include <KLocalizedString>
#include <KTextEditor/Document>

#include "../misc/rkcommonfunctions.h"
#include "../core/robjectlist.h"
#include "../settings/rksettingsmodulecommandeditor.h"

#include "../debug.h"

RKTextHints::RKTextHints(KTextEditor::View *view, const RKCodeCompletionSettings *settings) : QObject(view), KTextEditor::TextHintProvider(), settings(settings) {
	RK_TRACE(COMMANDEDITOR);
	view->registerTextHintProvider(this);
}

RKTextHints::~RKTextHints() {
	RK_TRACE(COMMANDEDITOR);
}

QString RKTextHints::textHint(KTextEditor::View *view, const KTextEditor::Cursor &position) {
	if (!settings->isEnabled(RKCodeCompletionSettings::MouseOver)) return QString();
	RK_TRACE(COMMANDEDITOR);
	QString line = view->document()->line(position.line()) + ' ';
	QString symbol = RKCommonFunctions::getCurrentSymbol(line, position.column(), false);
	auto obj = RObjectList::getObjectList()->findObject(symbol);
	if (obj) {
		return i18n("The name <i>%1</i> might refer to:<br>", symbol) + obj->getObjectDescription();
	}
	return QString();
}

