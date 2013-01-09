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

struct RKComponentDependency {
	RKComponentDependency (_type, _package, _min_version, _max_version) : type (_type), package (_package), min_version (_min_version), max_version (_max_version);
	enum DependencyType {
		RBaseInstallation,
		RPackage,
		RKWardPackage
	};
	DependencyType type;
	QString package;
	int min_version;
	int max_version;
};

class RKComponentMeta {
	static bool isRKWardVersionCompatible (const QDomElement &e, bool optional);
	static QList<RKComponentDependency> parseDependencies (const QDomElement &e);
	KAboutData parseAboutData (const QDomElement &e);
};

#endif
