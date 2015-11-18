/***************************************************************************
                          rkdummypart  -  description
                             -------------------
    begin                : Wed Feb 28 2007
    copyright            : (C) 2007, 2009 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

