/*
rkparsedversion - This file is part of the RKWard project. Created: Sat May 07 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkparsedversion.h"
#include "../debug.h"

RKParsedVersion::RKParsedVersion(const QString& version) {
	quint32 ret = 0;
	int pos = -1;
	int opos = 0;
	for (int i = 3; i >= 0; --i) {
		while (true) {
			++pos;
			if (!(pos < version.size () && version[pos].isDigit ())) {
				int val = QStringView{version}.mid(opos, pos - opos).toInt();
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
	if (opos <= (version.size() - 1)) {
		version_suffix = version.mid(opos);
	}

	version_numeric = ret;
}

QString RKParsedVersion::toString() const {
	QString ret;
	for (int i = 3; i >= 0; --i) {
		int ver_part = (version_numeric >> (i * 8)) & 0x000000FF;
		ret.append(QString::number(ver_part));
		if (i > 0) ret.append('.');
	}
	if (ret.endsWith(QLatin1String(".0"))) ret.chop(2);	// HACK: Don't print more than three version parts, unless the fourth is non-zero
	ret.append(version_suffix);
	return ret;
}
