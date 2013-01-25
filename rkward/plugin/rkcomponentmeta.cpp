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

#include "rkcomponentmeta.h"

#include "../misc/xmlhelper.h"
#include "../rbackend/rksessionvars.h"

#include "../debug.h"

QLatin1String rkward_min_version_tag ("rkward_min_version");
QLatin1String rkward_max_version_tag ("rkward_max_version");
QLatin1String R_min_version_tag ("R_min_version");
QLatin1String R_max_version_tag ("R_max_version");
QLatin1String any_min_version_tag ("min_version");
QLatin1String any_max_version_tag ("max_version");

RKComponentAboutData::RKComponentAboutData (const QDomElement& e) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = XMLHelper::getStaticHelper ();
	QString name = xml->getStringAttribute (e, "name", QString (), DL_ERROR);
	about = new KAboutData (name.toLocal8Bit (), QByteArray (), ki18n ("%1").subs (name), xml->getStringAttribute (e, "version", QString (), DL_WARNING).toLocal8Bit ());
	about->setShortDescription (ki18n ("%1").subs (xml->getStringAttribute (e, "shortinfo", QString (), DL_WARNING)));
	about->setCopyrightStatement (ki18n ("%1").subs (xml->getStringAttribute (e, "copyright", QString (), DL_WARNING)));
	about->setLicenseText (ki18n ("%1").subs (xml->getStringAttribute (e, "license", QString (), DL_WARNING)));
	about->setHomepage (xml->getStringAttribute (e, "url", QString (), DL_WARNING).toLocal8Bit ());
	// NOTE: Misusing catalog name for storing category.
	category = xml->getStringAttribute (e, "category", i18n ("Unspecified"), DL_INFO);
	// NOTE: ignoring releasedate for now.

	XMLChildList authors = xml->getChildElements (e, "author", DL_INFO);
	for (int i = 0; i < authors.size (); ++i) {
		QDomElement author = authors[i];
		about->addAuthor (ki18n ("%1").subs (xml->getStringAttribute (author, "given", QString (), DL_ERROR) + " " + xml->getStringAttribute (e, "family", QString (), DL_ERROR)),
						  ki18n ("%1").subs (xml->getStringAttribute (author, "role", QString (), DL_INFO)),	// TODO: translate into human readable
						  xml->getStringAttribute (author, "email", QString (), DL_WARNING).toLocal8Bit (),
						  xml->getStringAttribute (author, "url", QString (), DL_INFO).toLocal8Bit ()
			   );
	}
}

RKComponentAboutData::~RKComponentAboutData() {
	RK_TRACE (PLUGIN);

	delete about;
}


bool RKComponentDependency::isRKWardVersionCompatible (const QDomElement& e) {
	RK_TRACE (PLUGIN);

	if (e.hasAttribute (rkward_min_version_tag)) {
		if (RKSessionVars::compareRKWardVersion (e.attribute (rkward_min_version_tag)) > 0) return false;
	}
	if (e.hasAttribute (rkward_max_version_tag)) {
		if (RKSessionVars::compareRKWardVersion (e.attribute (rkward_max_version_tag)) < 0) return false;
	}

	return true;
}

QList <RKComponentDependency> RKComponentDependency::parseDependencies (const QDomElement& e) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = XMLHelper::getStaticHelper ();
	QList<RKComponentDependency> ret;
	RKComponentDependency dep;

	// Check for R dependency, first.
	dep.type = RKComponentDependency::RBaseInstallation;
	if (e.hasAttribute (R_min_version_tag)) dep.min_version = RKSessionVars::parseVersionString (e.attribute (R_min_version_tag), 0);
	if (e.hasAttribute (R_max_version_tag)) dep.max_version = RKSessionVars::parseVersionString (e.attribute (R_max_version_tag), 0);
	if ((dep.min_version > 0) || (dep.max_version < 0xFFFFFFFF)) ret.append (dep);

	XMLChildList deps = xml->getChildElements (e, QString (), DL_INFO);
	for (int i = 0; i < deps.size (); ++i) {
		QDomElement dep_e = deps[i];
		if (dep_e.tagName () == "package") {
			dep.type = RKComponentDependency::RPackage;
			dep.package = xml->getStringAttribute (e, "repository", QString (), DL_INFO);
		} else if (dep_e.tagName () == "pluginmap") {
			dep.type = RKComponentDependency::RKWardPluginmap;
			dep.package = xml->getStringAttribute (e, "url", QString ("http://rkward.sf.net"), DL_WARNING);
		} else {
			RK_DEBUG (PLUGIN, DL_ERROR, "Tag <%s> is not allowed, here.", qPrintable (dep_e.tagName ()));
			continue;
		}

		dep.min_version = 0;
		dep.max_version = 0xFFFFFFFF;
		if (e.hasAttribute (any_min_version_tag)) dep.min_version = RKSessionVars::parseVersionString (e.attribute (any_min_version_tag), 0);
		if (e.hasAttribute (any_max_version_tag)) dep.max_version = RKSessionVars::parseVersionString (e.attribute (any_max_version_tag), 0);

		ret.append (dep);
	}

	// Add RKWard dependency, last
	dep.type = RKComponentDependency::RKWardVersion;
	dep.min_version = 0;
	dep.max_version = 0xFFFFFFFF;
	// Although we ignore it, here, RKWard dependencies may come with a non-numeric suffix
	QString suffix_dummy;
	if (e.hasAttribute (rkward_min_version_tag)) dep.min_version = RKSessionVars::parseVersionString (e.attribute (rkward_min_version_tag), &suffix_dummy);
	if (e.hasAttribute (rkward_max_version_tag)) dep.max_version = RKSessionVars::parseVersionString (e.attribute (rkward_max_version_tag), &suffix_dummy);
	if ((dep.min_version > 0) || (dep.max_version < 0xFFFFFFFF)) ret.append (dep);

	return ret;
}
