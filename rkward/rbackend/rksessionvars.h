/*
rksessionvars - This file is part of the RKWard project. Created: Thu Sep 08 2011
SPDX-FileCopyrightText: 2011-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSESSIONVARS_H
#define RKSESSIONVARS_H

#include <QObject>
#include <QStringList>

#include "../misc/rkparsedversion.h"

class RInterface;

/** Singleton for storing information about the running R session, and - for some of the info - notifying about changes. */
class RKSessionVars : public QObject {
	Q_OBJECT
public:
	static RKSessionVars* instance () { return _instance; };
	QStringList installedPackages () const { return installed_packages; };
	void setInstalledPackages (const QStringList &new_list);
	static void setRVersion (const QString &version_string);
/** Return version number of currently running R.
@param abbridged If true, return Major.Minor, only (e.g. 3.5), otherwise, return full version string. */
	static QString RVersion (bool abbridged);
/** compare given version string against the running version of RKWard. Returns -1 for earlier than current, 0 for equal, 1 for later than current version */
	static int compareRKWardVersion (const QString &version);
/** compare given version string against the running version of R. Returns -1 for earlier than current, 0 for equal, 1 for later than current version. NOTE: The version of R is not known until the R backend has been started. In this case, 0 is always returned */
	static int compareRVersion (const QString &version);
/** Split "a.b.c.d.e-fghi" into up to four numeric portions (returned as four bytes in a single 32bit unsigned int).
Anything else (everything after the fourth dot, or after the first character that is neither dot, nor digit)
is returned as suffix (via the suffix pointer; if that is 0, an error is reported, instead). */
	static QStringList frontendSessionInfo ();
	static QString RBinary() { return r_binary; }
	static bool runningInAppImage() { return !appimagedir.isNull(); }
	static bool isPathInAppImage(const QString &path);
	static QStringList findRInstallations();
Q_SIGNALS:
	void installedPackagesChanged ();
protected:
friend class RInterface;
	explicit RKSessionVars(RInterface *parent);
	~RKSessionVars ();
private:
	static RKSessionVars* _instance;

	QStringList installed_packages;
	static RKParsedVersion rkward_version;
	static RKParsedVersion r_version;
	static QString r_version_string;
	static QString appimagedir;
friend class RKFrontendTransmitter;
friend class RKWardCoreTest;
friend class RKSetupWizard;
	static QString r_binary;
};

#endif
