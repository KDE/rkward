/*
rkstyle - This file is part of the RKWard project. Created: Wed Apr 13 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkstyle.h"

#include <QApplication>
#include <QStyle>

int RKStyle::marginHint() {
	return QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin);
}

int RKStyle::spacingHint() {
	return QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
}
