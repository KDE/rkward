/***************************************************************************
                          rkcomponentmeta  -  description
                             -------------------
    begin                : Wed Jan 09 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier
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

#ifndef RKCOMPONENTMETA_H
#define RKCOMPONENTMETA_H

#include <QDomElement>
#include <QList>

struct RKComponentDependency {
	RKComponentDependency () : type (RBaseInstallation), min_version (0), max_version (0xFFFFFFFF) {};
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
	quint32 min_version;
	quint32 max_version;

	static QList<RKComponentDependency> parseDependencies (const QDomElement &e);
	static bool isRKWardVersionCompatible (const QDomElement &e);
};

struct RKComponentAuthor {
	QString name;
	QString email;
	QString url;
	QString roles;
};

class RKComponentAboutData {
public:
	RKComponentAboutData (const QDomElement &e);
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
};

#endif
