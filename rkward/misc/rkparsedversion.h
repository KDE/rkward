/*
rkparsedversion - This file is part of the RKWard project. Created: Sat May 07 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKPARSEDVERSION_H
#define RKPARSEDVERSION_H

#include <QString>

/** Helper class to compare versions. A version number of "a.b.c.d.e-fghi" is split into up to four numeric portions (stored as four bytes in a single 32bit unsigned int).
Anything else (everything after the fourth dot, or after the first character that is neither dot, nor digit) is stored as a string, and compared lexically. */
class RKParsedVersion {
public:
	explicit RKParsedVersion(const QString& from_string);
	RKParsedVersion() : version_numeric(0) {};
/** Create a null version that will always compare as higher than other (non-null) versions */
	static RKParsedVersion maxVersion() {
		RKParsedVersion ret;
		ret.version_numeric = 0xFFFFFFFF;
		return ret;
	}
	bool operator >(const RKParsedVersion &other) const {
		return (version_numeric > other.version_numeric || (version_numeric == other.version_numeric && version_suffix > other.version_suffix));
	}
	bool operator <(const RKParsedVersion &other) const {
		return (version_numeric < other.version_numeric || (version_numeric == other.version_numeric && version_suffix < other.version_suffix));
	}
	bool operator ==(const RKParsedVersion &other) const {
		return ((version_numeric == other.version_numeric) && (version_suffix == other.version_suffix));
	}
	bool isNull() const { return (version_suffix.isNull() && (version_numeric == 0 || version_numeric == 0xFFFFFFFF)); }
	QString toString() const;
private:
	quint32 version_numeric;
	QString version_suffix;
};

#endif
