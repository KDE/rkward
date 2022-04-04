/*
rkdummypart - This file is part of the RKWard project. Created: Wed Feb 28 2007
SPDX-FileCopyrightText: 2007 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKDUMMYPART_H
#define RKDUMMYPART_H

#include <kparts/part.h>

class RKDummyPart : public KParts::Part {
public:
	RKDummyPart (QObject *parent, QWidget *widget);
	~RKDummyPart ();
};

#endif
