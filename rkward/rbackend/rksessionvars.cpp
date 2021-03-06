/***************************************************************************
                          rksessionvars  -  description
                             -------------------
    begin                : Thu Sep 08 2011
    copyright            : (C) 2011-2020 by Thomas Friedrichsmeier
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

#include "rksessionvars.h"

#include "rkrinterface.h"
#include "../settings/rksettingsmoduledebug.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../version.h"

#include <kcoreaddons_version.h>
#if KCOREADDONS_VERSION >= QT_VERSION_CHECK(5,20,0)
#include <kcoreaddons.h>
#endif

#include <QTemporaryFile>
#include <QStandardPaths>
#include <QSysInfo>

#include "../debug.h"

RKSessionVars* RKSessionVars::_instance = 0;
quint32 RKSessionVars::rkward_version = 0;
QString RKSessionVars::rkward_version_suffix;
quint32 RKSessionVars::r_version = 0;
QString RKSessionVars::r_version_string;

RKSessionVars::RKSessionVars (RInterface *parent) : QObject (parent) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (!_instance);

	_instance = this;
}

RKSessionVars::~RKSessionVars () {
	RK_TRACE (RBACKEND);
}

void RKSessionVars::setInstalledPackages (const QStringList &new_list) {
	RK_TRACE (RBACKEND);

	installed_packages = new_list;
	emit (installedPackagesChanged ());
}

void RKSessionVars::setRVersion (const QString& version_string) {
	RK_TRACE (RBACKEND);

	if (!r_version_string.isEmpty ()) {
		RK_DEBUG (RBACKEND, DL_WARNING, "R version has changed during runtime, from %s to %s", qPrintable (r_version_string), qPrintable (version_string));
	}
	r_version_string = version_string;
	r_version = parseVersionString (version_string, 0);
}

QString RKSessionVars::RVersion(bool abbridged) {
	if (!abbridged) return r_version_string;
	return r_version_string.section ('.', 0, 1);
}

quint32 RKSessionVars::parseVersionString (const QString &version, QString *suffix) {
	quint32 ret = 0;
	int pos = -1;
	int opos = 0;
	for (int i = 3; i >= 0; --i) {
		while (1) {
			++pos;
			if (!(pos < version.size () && version[pos].isDigit ())) {
				int val = version.mid (opos, pos - opos).toInt ();
				if ((val < 0) || (val > 255) || (pos == opos)) {
					RK_DEBUG (MISC, DL_ERROR, "Invalid version specification '%s'", qPrintable (version));
					if (val > 255) val = 255;
					else val = 0;
				}
				ret += val << (8 * i);
				if ((pos < version.size ()) && (version[pos] == '.')) {
					opos = pos + 1;
					break;
				}
				opos = pos;
				i = -1;
				break;
			}
		}
	}
	if (opos <= (version.size () - 1)) {
		if (suffix) *suffix = version.mid (opos);
		else RK_DEBUG (MISC, DL_WARNING, "Non numeric portion ('%s') of version specification '%s' will be ignored.", qPrintable (version.mid (opos)), qPrintable (version));
	}

	return ret;
}

int RKSessionVars::compareRKWardVersion (const QString& version) {
	if (!rkward_version) {
		rkward_version = parseVersionString (RKWARD_VERSION, &rkward_version_suffix);
	}

	QString suffix;
	quint32 ver = parseVersionString (version, &suffix);
	if (ver < rkward_version) return -1;
	if (ver > rkward_version) return 1;
	return (suffix.compare (rkward_version_suffix));
}

int RKSessionVars::compareRVersion (const QString& version) {
	if (r_version_string.isEmpty()) return 0;

	quint32 ver = parseVersionString (version, 0);
	if (ver < r_version) return -1;
	if (ver > r_version) return 1;
	return 0;
}

QStringList RKSessionVars::frontendSessionInfo () {
	QStringList lines;
	lines.append ("RKWard version: " RKWARD_VERSION);
	// KF5 TODO: find replacement for line below
#if KCOREADDONS_VERSION >= QT_VERSION_CHECK(5,20,0)
	lines.append ("KDE Frameworks version (runtime): " + QString (KCoreAddons::versionString ()));
#endif
	lines.append ("KDE Frameworks version (compile time): " KCOREADDONS_VERSION_STRING);
	lines.append (QString ("Qt version (runtime): ") + qVersion ());
	lines.append ("Qt version (compile time): " QT_VERSION_STR);
#ifdef NO_QT_WEBENGINE
	lines.append ("Using QtWebKit for HTML rendering");
#else
	lines.append ("Using QWebEngine for HTML rendering");
#endif
#if defined Q_OS_WIN
	lines.append ("Windows runtime version (refer to QSysInfo documentation to translate code into human readable form): 0x" + QString::number (QSysInfo::windowsVersion (), 16));
#elif defined Q_OS_MACOS
	lines.append ("MacOS runtime version (refer to QSysInfo documentation to translate code into human readable form): 0x" + QString::number (QSysInfo::MacintoshVersion, 16));
#endif
	lines.append ("Local config directory: " + QStandardPaths::writableLocation (QStandardPaths::GenericConfigLocation));
	lines.append ("RKWard storage directory: " + RKSettingsModuleGeneral::filesPath ());
	lines.append ("Backend version (as known to the frontend): " + r_version_string);
	lines.append (QString());
	lines.append ("Debug message file (this may contain relevant diagnostic output in case of trouble):");
	lines.append (RK_Debug::debug_file->fileName ());
	return lines;
}

