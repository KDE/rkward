/*
rkstyle - This file is part of the RKWard project. Created: Wed Apr 13 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkstyle.h"

#include <QApplication>
#include <QStyle>

#include <KColorScheme>

#include "../debug.h"

KColorScheme* RKStyle::_view_scheme = nullptr;

int RKStyle::marginHint() {
	return QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin);
}

int RKStyle::spacingHint() {
	return QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
}

KColorScheme* RKStyle::viewScheme() {
	if (!_view_scheme) {
		RK_TRACE(MISC);
		_view_scheme = new KColorScheme(QPalette::Normal);
		QObject::connect(qApp, &QGuiApplication::paletteChanged, cleanResources);  // will be re-initialized when needed, again; NOTE: Not emitted before Qt 5.13.0, so quirky, before
	}
	return _view_scheme;
}

void RKStyle::cleanResources() {
	RK_TRACE(MISC);
	delete _view_scheme;
	_view_scheme = nullptr;
}
