/*
rkcommonfunctions - This file is part of the RKWard project. Created: Sat May 14 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcompatibility.h"

#include <QApplication>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#	include <QDesktopWidget>
#	include <QWidget>
#else
#	include <QScreen>
#endif

namespace RKCompatibility {
	QRect availableGeometry(QWidget* for_widget) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
		auto screen = for_widget->screen();
		if (screen) return screen->availableGeometry();
		return QApplication::primaryScreen()->availableGeometry();
#else
		return ::QApplication::desktop()->availableGeometry(for_widget);
#endif
	}
};

