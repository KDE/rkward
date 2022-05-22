/*
rkloadagent - This file is part of the RKWard project. Created: Sun Sep 5 2004
SPDX-FileCopyrightText: 2004 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKLOADAGENT_H
#define RKLOADAGENT_H

#include <qobject.h>

#include <qstring.h>
#include <QUrl>

class QTemporaryFile;

/** The RKLoadAgent is really a rather simple agent. All it needs to do is display an error message, if loading fails. No further action is required. Like all
agents, the RKLoadAgent self-destructs when done.
@author Thomas Friedrichsmeier
*/
class RKLoadAgent : public QObject {
	Q_OBJECT
public:
	explicit RKLoadAgent (const QUrl &url, bool merge=false);

	~RKLoadAgent ();
private:
/// needed if file to be loaded is remote
	QTemporaryFile* tmpfile;
	bool _merge;
};

#endif
