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
#include <kaboutdata.h>

struct RKComponentDependency {
	RKComponentDependency () : type (RBaseInstallation), min_version (0), max_version (0xFFFFFFFF);
	enum DependencyType {
		RBaseInstallation,
		RPackage,
		RKWardPluginmap,
	};
	DependencyType type;
	QString package;
	QString source_info;
	quint32 min_version;
	quint32 max_version;
};

class RKComponentMeta {
	static bool isRKWardVersionCompatible (const QDomElement &e);
	static QList<RKComponentDependency> parseDependencies (const QDomElement &e);
	static KAboutData parseAboutData (const QDomElement &e);
};

#endif
