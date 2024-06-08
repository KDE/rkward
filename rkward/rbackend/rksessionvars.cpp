/*
rksessionvars - This file is part of RKWard (https://rkward.kde.org). Created: Thu Sep 08 2011
SPDX-FileCopyrightText: 2011-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rksessionvars.h"

#include "rkrinterface.h"
#include "../settings/rksettingsmoduledebug.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettingsmoduler.h"
#include "../version.h"

#include <kcoreaddons_version.h>
#include <KCoreAddons>

#include <QTemporaryFile>
#include <QStandardPaths>
#include <QSysInfo>
#include <QFileInfo>
#include <QVersionNumber>
#include <QDir>

#include "../debug.h"

RKSessionVars* RKSessionVars::_instance = nullptr;
RKParsedVersion RKSessionVars::rkward_version(RKWARD_VERSION);
RKParsedVersion RKSessionVars::r_version;
QString RKSessionVars::r_version_string;
QString RKSessionVars::r_binary;
QString RKSessionVars::appimagedir;

RKSessionVars::RKSessionVars (RInterface *parent) : QObject (parent) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (!_instance);

	_instance = this;
	auto appdir = qgetenv("APPDIR");
	if (!appdir.isEmpty()) appimagedir = QFileInfo(QString::fromLocal8Bit(appdir)).canonicalFilePath();
}

RKSessionVars::~RKSessionVars () {
	RK_TRACE (RBACKEND);
	RK_ASSERT(_instance == this);
	_instance = nullptr;
}

bool RKSessionVars::isPathInAppImage(const QString &path) {
	if (!runningInAppImage()) return false;
	return QFileInfo(path).canonicalFilePath().startsWith(appimagedir);
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
	// NOTE: No translation on purpose. This is mostly meant for pasting to the bug tracker
	QStringList lines;
	lines.append ("RKWard version: " RKWARD_VERSION);
	lines.append ("KDE Frameworks version (runtime): " + QString (KCoreAddons::versionString ()));
	lines.append ("KDE Frameworks version (compile time): " KCOREADDONS_VERSION_STRING);
	lines.append (QString ("Qt version (runtime): ") + qVersion ());
	lines.append ("Qt version (compile time): " QT_VERSION_STR);
	lines.append ("Using QWebEngine for HTML rendering");
	lines.append(QStringLiteral("Running on: ") + QSysInfo::prettyProductName());
	lines.append ("Local config directory: " + QStandardPaths::writableLocation (QStandardPaths::GenericConfigLocation));
	lines.append ("RKWard storage directory: " + RKSettingsModuleGeneral::filesPath ());
	lines.append ("Backend version (as known to the frontend): " + r_version_string);
	lines.append (QString());
	lines.append ("Debug message file (this may contain relevant diagnostic output in case of trouble):");
	lines.append (RK_Debug::debug_file->fileName ());
	return lines;
}

#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
static QString findExeAtPath (const QString &appname, const QString &path) {
	QDir dir (path);
	dir.makeAbsolute ();
	if (QFileInfo (dir.filePath (appname)).isExecutable ()) return dir.filePath (appname);
#ifdef Q_OS_WIN
	if (QFileInfo (dir.filePath (appname + ".exe")).isExecutable ()) return dir.filePath (appname + ".exe");
	if (QFileInfo (dir.filePath (appname + ".com")).isExecutable ()) return dir.filePath (appname + ".com");
#endif
	return QString ();
}

/** glob dirs instroot/prefix-* /rpath, sorted by version number represented in "*" */
static QStringList globVersionedDirs(const QString &instroot, const QString &prefix, const QString &rpath) {
	QStringList ret;
	if (QFileInfo(instroot).isReadable()) {
		QDir dir(instroot);
		QStringList candidates = dir.entryList(QStringList(prefix + "*"), QDir::Dirs);
		std::sort(candidates.begin(), candidates.end(), [prefix](const QString&a, const QString& b) -> bool {
			return QVersionNumber::fromString(a.mid(prefix.length())) > QVersionNumber::fromString(b.mid(prefix.length()));
		});
		for (int i = 0; i < candidates.count(); ++i) {
			QString found = findExeAtPath(rpath, dir.absoluteFilePath(candidates[i]));
			if (!found.isNull()) ret.append(found);
		}
	}
	return ret;
}
#endif

QStringList RKSessionVars::findRInstallations() {
	QStringList ret;
#if defined(Q_OS_MACOS)
	ret = globVersionedDirs("/Library/Frameworks/R.framework/Versions", QString(), "Resources/bin/R");
#elif defined(Q_OS_WIN)
	QString instroot = QString(getenv("PROGRAMFILES")) + "/R";
	if (!QFileInfo(instroot).isReadable()) instroot = QString(getenv("PROGRAMFILES(x86)")) + "/R";
	ret = globVersionedDirs(instroot, "R-", "bin/R");
#else
	const QStringList candidates{"/usr/bin/R", "/usr/local/bin/R"};
	for(const QString &p : candidates) {
		if (!ret.contains(p) && QFileInfo(p).isExecutable()) ret.append(p);
	}
#endif
	// On Unix, but also, if R was not found in the default locations try to find R in the system path.
	QString r = QStandardPaths::findExecutable("R");
	if (!(r.isEmpty() || ret.contains(r))) ret.append(r);
	// NOTE: in a default start, the configured R binary takes precedence over all the others, *but*
	//       --r-executable auto was specified, we want to detect as if from scratch. Thus appending, rather than prepending
	r = RKSettingsModuleR::userConfiguredRBinary();
	if (!(r.isEmpty() || ret.contains(r))) ret.append(r);
	return ret;
}
