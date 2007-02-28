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
#ifndef RKDUMMYPART_H
#define RKDUMMYPART_H

#include <kparts/part.h>

class RKDummyPart : public KParts::Part {
public:
	RKDummyPart (QObject *parent, QWidget *widget);
	~RKDummyPart ();
};

#endif
