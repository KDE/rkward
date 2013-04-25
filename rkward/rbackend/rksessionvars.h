/***************************************************************************
                          rksessionvars  -  description
                             -------------------
    begin                : Thu Sep 08 2011
    copyright            : (C) 2011, 2013 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RKSESSIONVARS_H
#define RKSESSIONVARS_H

#include <QObject>
#include <QStringList>

class RInterface;

/** Singleton for storing information about the running R session, and - for some of the info - notifying about changes. */
class RKSessionVars : public QObject {
	Q_OBJECT
public:
	static RKSessionVars* instance () { return _instance; };
	QStringList installedPackages () const { return installed_packages; };
	void setInstalledPackages (const QStringList &new_list);
	static void setRVersion (const QString &version_string);
/** compare given version string against the running version of RKWard. Returns -1 for earlier than current, 0 for equal, 1 for later than current version */
	static int compareRKWardVersion (const QString &version);
/** compare given version string against the running version of R. Returns -1 for earlier than current, 0 for equal, 1 for later than current version. NOTE: The version of R is not known until the R backend has been started. In this case, 0 is always returned */
	static int compareRVersion (const QString &version);
/** Split "a.b.c.d.e-fghi" into up to four numeric portions (returned as four bytes in a single 32bit unsigned int).
Anything else (everything after the fourth dot, or after the first character that is neither dot, nor digit)
is returned as suffix (via the suffix pointer; if that is 0, an error is reported, instead). */
	static quint32 parseVersionString (const QString &version, QString *suffix);
	static QStringList frontendSessionInfo ();
signals:
	void installedPackagesChanged ();
protected:
friend class RInterface;
	RKSessionVars (RInterface *parent);
	~RKSessionVars ();
private:
	static RKSessionVars* _instance;

	QStringList installed_packages;
	static quint32 rkward_version;
	static QString rkward_version_suffix;
	static quint32 r_version;
	static QString r_version_string;
};

#endif
