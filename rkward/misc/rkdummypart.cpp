/*
rkdummypart - This file is part of RKWard (https://rkward.kde.org). Created: Wed Feb 28 2007
SPDX-FileCopyrightText: 2007-2009 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkdummypart.h"

#include <QGuiApplication>

#include "../debug.h"

RKDummyPart::RKDummyPart (QObject *parent, QWidget *widget) : KParts::Part (parent) {
	RK_TRACE (MISC);
	setWidget (widget);
	setComponentName (QCoreApplication::applicationName (), QGuiApplication::applicationDisplayName ());
	setXMLFile ("rkdummypart.rc");
}

RKDummyPart::~RKDummyPart () {
	RK_TRACE (MISC);
}

