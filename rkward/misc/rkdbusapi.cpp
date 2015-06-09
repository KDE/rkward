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

#include "rkdbusapi.h"

#include <QDBusConnection>
#include <kwindowsystem.h>
#include "../windows/rkworkplace.h"
#include "../rkward.h"

#include "../debug.h"

RKDBusAPI::RKDBusAPI (QObject* parent): QObject (parent) {
	RK_TRACE (APP);

	if (!QDBusConnection::sessionBus ().isConnected ()) {
		RK_DEBUG (DL_ERROR, APP, "D-Bus session bus in not connected");
		return;
	}

	if (!QDBusConnection::sessionBus ().registerService (RKDBUS_SERVICENAME)) {
		RK_DEBUG (DL_ERROR, APP, "Could not register org.kde.rkward on session bus");
		return;
	}

	QDBusConnection::sessionBus ().registerObject ("/", this, QDBusConnection::ExportScriptableSlots);
}

void RKDBusAPI::openAnyUrl(const QStringList& urls) {
	RK_TRACE (APP);

	// ok, raising the app window is totally hard to do, reliably. This solution copied from kate.
	QWidget *main = RKWardMainWindow::getMain ();
	main->show();
	main->activateWindow();
	main->raise();
#ifdef Q_WS_X11
	KWindowSystem::forceActiveWindow (main->winId ());
	KWindowSystem::raiseWindow (main->winId ());
	KWindowSystem::demandAttention (main->winId ());
#endif
	// end

	RKWardMainWindow::getMain ()->setMergeLoads (true);
	for (int i = 0; i < urls.size (); ++i) {
		RKWorkplace::mainWorkplace ()->openAnyUrl (urls[i]);
	}
	RKWardMainWindow::getMain ()->setMergeLoads (false);
}

