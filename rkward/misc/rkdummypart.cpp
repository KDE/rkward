/***************************************************************************
                          rkdummypart  -  description
                             -------------------
    begin                : Wed Feb 28 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kapplication.h>

#include "rkdummypart.h"

#include "../debug.h"

RKDummyPart::RKDummyPart (QObject *parent, QWidget *widget) : KParts::Part (parent) {
	RK_TRACE (MISC);
	setWidget (widget);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);
}

RKDummyPart::~RKDummyPart () {
	RK_TRACE (MISC);
}

