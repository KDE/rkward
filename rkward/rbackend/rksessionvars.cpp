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

#include "rksessionvars.h"

#include "rinterface.h"
#include "../version.h"

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
	if (opos < (version.size () - 1)) {
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

#include "rksessionvars.moc"
