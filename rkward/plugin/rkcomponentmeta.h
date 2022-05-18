/*
rkcomponentmeta - This file is part of the RKWard project. Created: Wed Jan 09 2013
SPDX-FileCopyrightText: 2013-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKCOMPONENTMETA_H
#define RKCOMPONENTMETA_H

#include <QDomElement>
#include <QList>

#include "../misc/rkparsedversion.h"

class XMLHelper;
struct RKComponentDependency {
	RKComponentDependency () : type(RBaseInstallation), min_version(RKParsedVersion()), max_version(RKParsedVersion::maxVersion()) {};
	QString toHtml () const;
	static QString depsToHtml (const QList<RKComponentDependency> &deps);
	enum DependencyType {
		RBaseInstallation,
		RPackage,
		RKWardPluginmap,
		RKWardVersion
	};
	DependencyType type;
	QString package;
	QString source_info;
	RKParsedVersion min_version;
	RKParsedVersion max_version;

	static QList<RKComponentDependency> parseDependencies (const QDomElement &e, XMLHelper &xml);
	static bool isRKWardVersionCompatible (const QDomElement &e);
	static bool isRVersionCompatible (const QDomElement &e);
};

struct RKComponentAuthor {
	QString name;
	QString email;
	QString url;
	QString roles;
};

class RKComponentAboutData {
public:
	RKComponentAboutData () { valid = false; };
	RKComponentAboutData (const QDomElement &e, XMLHelper &xml);
	~RKComponentAboutData ();
	QString toHtml () const;

	QString name;
	QString version;
	QString releasedate;
	QString shortinfo;
	QString copyright;
	QString license;
	QString url;
	QString category;
	QList<RKComponentAuthor> authors;
	QString translator_names;
	QString translator_emails;
	bool valid;
};

#endif
