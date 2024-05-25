/*
rkdbusapi - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 20 2014
SPDX-FileCopyrightText: 2014-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkdbusapi.h"

#include <QDBusConnection>
#include <KWindowSystem>
#if __has_include(<KStartupInfo>)
#include <KStartupInfo>
#endif
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

void RKDBusAPI::openAnyUrl (const QStringList& urls, bool warn_external) {
	RK_TRACE (APP);

	// ok, raising the app window is totally hard to do, reliably. This solution copied from kate.
	QWidget *main = RKWardMainWindow::getMain ();
	main->show();
	main->activateWindow();
	main->raise();

	// omitting activation token

	KWindowSystem::activateWindow(main->windowHandle());
	// end

	RKWardMainWindow::getMain ()->openUrlsFromCommandLineOrDBus (warn_external, urls);
}

