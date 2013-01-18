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

quint32 RKSessionVars::parseVersionString (const QString &version, QString *suffix) {
	quint32 ret = 0;
	int pos = -1;
	int opos = 0;
	for (int i = 3; i > 0; --i) {
		++pos;
		if (!version[pos].isDigit ()) {
			int val = version.mid (pos, pos - opos).toInt ();
			if ((val < 0) || (val > 255) || (pos == opos)) {
				RK_DEBUG (MISC, DL_ERROR, "Invalid version specification '%s'", qPrintable (version));
				if (val > 255) val = 255;
				else val = 0;
			}
			ret += val << (8 * i);
			if (version[pos] == '.') {
				opos = pos + 1;
				continue;
			}
			opos = pos;
			break;
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
		rkward_version = parseVersionString (version, &rkward_version_suffix);
	}

	QString suffix;
	quint32 ver = parseVersionString (version, &suffix);
	if (ver < rkward_version) return -1;
	if (ver > rkward_version) return 1;
	return (suffix.compare (rkward_version_suffix));
}

#include "rksessionvars.moc"
