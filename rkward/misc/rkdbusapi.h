/***************************************************************************
                          rkdbusapi  -  description
                             -------------------
    begin                : Thu Nov 20 2014
    copyright            : (C) 2014 by Thomas Friedrichsmeier
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
	Q_SCRIPTABLE void openAnyUrl (const QStringList &urls);
};

#endif
