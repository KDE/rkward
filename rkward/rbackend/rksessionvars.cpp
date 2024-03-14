/*
rksessionvars - This file is part of RKWard (https://rkward.kde.org). Created: Thu Sep 08 2011
SPDX-FileCopyrightText: 2011-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
RKParsedVersion RKSessionVars::rkward_version(RKWARD_VERSION);
RKParsedVersion RKSessionVars::r_version;
QString RKSessionVars::r_version_string;
QString RKSessionVars::r_binary;

RKSessionVars::RKSessionVars (RInterface *parent) : QObject (parent) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (!_instance);

	_instance = this;
}

RKSessionVars::~RKSessionVars () {
	RK_TRACE (RBACKEND);
	RK_ASSERT(_instance == this);
	_instance = nullptr;
}

void RKSessionVars::setInstalledPackages (const QStringList &new_list) {
	RK_TRACE (RBACKEND);

	installed_packages = new_list;
	Q_EMIT installedPackagesChanged();
}

void RKSessionVars::setRVersion (const QString& version_string) {
	RK_TRACE (RBACKEND);

	if (!r_version_string.isEmpty ()) {
		RK_DEBUG (RBACKEND, DL_WARNING, "R version has changed during runtime, from %s to %s", qPrintable (r_version_string), qPrintable (version_string));
	}
	r_version_string = version_string;
	r_version = RKParsedVersion(version_string);
}

QString RKSessionVars::RVersion(bool abbridged) {
	if (!abbridged) return r_version_string;
	return r_version_string.section ('.', 0, 1);
}

int RKSessionVars::compareRKWardVersion (const QString& version) {
	auto ver = RKParsedVersion(version);
	if (rkward_version > ver) return -1;
	if (ver > rkward_version) return 1;
	return 0;
}

int RKSessionVars::compareRVersion (const QString& version) {
	if (r_version_string.isEmpty()) return 0;

	auto ver = RKParsedVersion(version);
	if (r_version > ver) return -1;
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
	lines.append ("Using QWebEngine for HTML rendering");
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

