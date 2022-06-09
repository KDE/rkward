/*
rkeditobjectagent - This file is part of the RKWard project. Created: Fri Feb 16 2007
SPDX-FileCopyrightText: 2007-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKEDITOBJECTAGENT_H
#define RKEDITOBJECTAGENT_H

#include <qobject.h>

#include <qstring.h>
#include <qstringlist.h>

class RCommandChain;

/** This agent gets called, when an rk.edit() command was run in the backend. The purpose is to first update the structure information for the object(s), and then try to open it/them.

@author Thomas Friedrichsmeier
*/
class RKEditObjectAgent : public QObject {
	Q_OBJECT
public:
	RKEditObjectAgent (const QStringList &object_names, RCommandChain *chain);

	~RKEditObjectAgent ();
private:
	QStringList object_names;
};

#endif
