/*
rkglobals - This file is part of RKWard (https://rkward.kde.org). Created: Wed Aug 18 2004
SPDX-FileCopyrightText: 2004 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkglobals.h"

#include <qstring.h>

RInterface *RKGlobals::rinter;
RKModificationTracker *RKGlobals::mtracker;
QVariantMap RKGlobals::startup_options;

#include <QApplication>
#include <QStyle>

int RKGlobals::marginHint () {
	return QApplication::style ()->pixelMetric (QStyle::PM_DefaultChildMargin);
}

int RKGlobals::spacingHint () {
	return QApplication::style ()->pixelMetric (QStyle::PM_DefaultLayoutSpacing);
}

