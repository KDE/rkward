/*
rkdbusapi - This file is part of the RKWard project. Created: Thu Nov 20 2014
SPDX-FileCopyrightText: 2014-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKDBUSAPI_H
#define RKDBUSAPI_H

#define RKDBUS_SERVICENAME "org.kde.rkward.api"

#include <QObject>

class RKDBusAPI : public QObject {
	Q_OBJECT
public:
/** Creates an object (should be a singleton) to relay incoming DBus calls, and registers it on the session bus. */
	explicit RKDBusAPI (QObject *parent);
	~RKDBusAPI () {};
public slots:
	Q_SCRIPTABLE void openAnyUrl (const QStringList &urls, bool warn_external=true);
};

#endif
