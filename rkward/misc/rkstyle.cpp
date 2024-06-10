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
	return QApplication::style()->pixelMetric(QStyle::PM_LayoutTopMargin);
}

int RKStyle::spacingHint() {
	return QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
}

KColorScheme* RKStyle::viewScheme() {
	if (!_view_scheme) {
		RK_TRACE(MISC);
		// Note: Will be updated on changes with RKWardMainWindow::event
		_view_scheme = new KColorScheme(QPalette::Normal);
	}
	return _view_scheme;
}
