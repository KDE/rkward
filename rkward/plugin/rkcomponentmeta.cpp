/*
rkcomponentmeta - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jan 09 2013
SPDX-FileCopyrightText: 2013-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcomponentmeta.h"

#include "../misc/xmlhelper.h"
#include "../misc/rkmessagecatalog.h"
#include "../rbackend/rksessionvars.h"

#include <KLocalizedString>

#include "../debug.h"

using namespace Qt::Literals::StringLiterals;

static QLatin1String rkward_min_version_tag("rkward_min_version");
static QLatin1String rkward_max_version_tag("rkward_max_version");
static QLatin1String R_min_version_tag("R_min_version");
static QLatin1String R_max_version_tag("R_max_version");
static QLatin1String any_min_version_tag("min_version");
static QLatin1String any_max_version_tag("max_version");
static QLatin1String platforms_tag("platforms");

RKComponentAboutData::RKComponentAboutData (const QDomElement& e, XMLHelper &xml) {
	RK_TRACE (PLUGIN);
	if (e.isNull ()) {
		valid = false;
		return;
	}

	valid = true;
	name = xml.i18nStringAttribute (e, QStringLiteral("name"), QString (), DL_INFO);
	version = xml.getStringAttribute (e, QStringLiteral("version"), QString (), DL_INFO);
	releasedate = xml.getStringAttribute (e, QStringLiteral("releasedate"), QString (), DL_INFO);
	shortinfo = xml.i18nStringAttribute (e, QStringLiteral("shortinfo"), QString (), DL_INFO);
	copyright = xml.getStringAttribute (e, QStringLiteral("copyright"), QString (), DL_INFO);
	license = xml.getStringAttribute (e, QStringLiteral("license"), QString (), DL_INFO);
	url = xml.getStringAttribute (e, QStringLiteral("url"), QString (), DL_INFO);
	category = xml.i18nStringAttribute (e, QStringLiteral("category"), i18n ("Unspecified"), DL_INFO);

	XMLChildList aes = xml.getChildElements (e, QStringLiteral("author"), DL_INFO);
	for (int i = 0; i < aes.size (); ++i) {
		QDomElement ae = aes[i];
		RKComponentAuthor author;
		author.name = xml.i18nStringAttribute (ae, QStringLiteral("name"), QString (), DL_INFO);
		if (author.name.isEmpty ()) {
			author.name = xml.i18nStringAttribute (ae, QStringLiteral("given"), QString (), DL_WARNING) + u' ' + xml.i18nStringAttribute (ae, QStringLiteral("family"), QString (), DL_WARNING);
			
		}
		if (author.name.isEmpty ()) xml.displayError (&ae, QStringLiteral("No author name specified"), DL_WARNING);
		author.roles = xml.getStringAttribute (ae, QStringLiteral("role"), QString (), DL_INFO);
		author.email = xml.getStringAttribute (ae, QStringLiteral("email"), QString (), DL_WARNING);
		author.url = xml.getStringAttribute (ae, QStringLiteral("url"), QString (), DL_INFO);
		authors.append (author);
	}

	const QString translator_names_id (QStringLiteral("Your names"));
	const QString translator_emails_id (QStringLiteral("Your emails"));
	translator_names = xml.messageCatalog ()->translate (QStringLiteral("NAME OF TRANSLATORS"), translator_names_id);
	translator_emails = xml.messageCatalog ()->translate (QStringLiteral("EMAIL OF TRANSLATORS"), translator_emails_id);
	if (translator_names == translator_names_id) translator_names.clear ();
	if (translator_emails == translator_emails_id) translator_emails.clear ();
}

RKComponentAboutData::~RKComponentAboutData () {
	RK_TRACE (PLUGIN);
}

QString RKComponentAboutData::toHtml() const {
	RK_TRACE(PLUGIN);

	QString ret = u"<p><b>"_s + name + u"</b>"_s;
	if (!version.isEmpty()) ret.append(u' ' + version);
	if (!releasedate.isEmpty()) ret.append(u" ("_s + releasedate + u')');
	if (!shortinfo.isEmpty()) ret.append(u":</p>\n<p>"_s + shortinfo);
	ret.append(u"</p>\n"_s);
	if (!url.isEmpty()) ret.append(u"URL: <a href=\""_s + url + u"\">"_s + url + u"</a></p>\n<p>"_s);
	if (!copyright.isEmpty()) ret.append(i18n("Copyright (c): %1", copyright) + u"</p>\n<p>"_s);
	if (!license.isEmpty()) ret.append(i18n("License: %1", license) + u"</p>"_s);

	if (!authors.isEmpty()) {
		ret.append(u"\n<p><b>"_s + i18n("Authors:") + u"</b></p>\n<p><ul>"_s);
		for (int i = 0; i < authors.size(); ++i) {
			RKComponentAuthor a = authors[i];
			ret.append(u"<li>"_s + a.name);
			if (!a.email.isEmpty()) ret.append(u" ("_s + a.email + u')');
			if (!a.url.isEmpty()) ret.append(u" ("_s + a.url + u')');
			if (!a.roles.isEmpty()) ret.append(u"<br/><i>"_s + i18nc("Author roles (contributor, etc.)", "Roles") + u"</i>: "_s + a.roles);
		}
		ret.append(u"</ul></p>"_s);
	}

	if (!translator_names.isNull()) {
		QStringList tns = translator_names.split(QLatin1Char(','), Qt::KeepEmptyParts);
		QStringList tes = translator_emails.split(QLatin1Char(','), Qt::KeepEmptyParts);
		ret.append(u"\n<p><b>"_s + i18n("Translators:") + u"</b></p>\n<p><ul>"_s);
		for (int i = 0; i < tns.size(); ++i) {
			QString tn = tns.value(i);
			QString te = tes.value(i);
			if (tn.isEmpty() && te.isEmpty()) continue;
			ret.append(u"<li>"_s + tn + u" <"_s + te + u"></li>\n"_s);
		}
		ret.append(u"</ul></p>"_s);
	}

	return ret;
}


bool RKComponentDependency::isRKWardVersionCompatible(const QDomElement& e) {
	RK_TRACE(PLUGIN);

	if (e.hasAttribute(rkward_min_version_tag)) {
		if (RKSessionVars::compareRKWardVersion(e.attribute(rkward_min_version_tag)) > 0) return false;
	}
	if (e.hasAttribute(rkward_max_version_tag)) {
		if (RKSessionVars::compareRKWardVersion(e.attribute(rkward_max_version_tag)) < 0) return false;
	}
	if (e.hasAttribute(platforms_tag)) {
		auto platforms = e.attribute(platforms_tag).split(u':');
#if defined(Q_OS_WIN)
		if (platforms.contains(QLatin1String("windows"))) return true;
#elif defined(Q_OS_MACOS)
		if (platforms.contains(QLatin1String("macos"))) return true;
#elif defined(Q_OS_UNIX)  // NOTE: order matters. Q_OS_UNIX is also defined on mac, but we do not want to count that as "unix", here.
		if (platforms.contains(QLatin1String("unix"))) return true;
#else
		static_assert(false, "Undefined platform");
#endif
		return false;
	}

	return true;
}

bool RKComponentDependency::isRVersionCompatible (const QDomElement& e) {
	RK_TRACE (PLUGIN);

	if (e.hasAttribute (R_min_version_tag)) {
		if (RKSessionVars::compareRVersion (e.attribute (R_min_version_tag)) > 0) return false;
	}
	if (e.hasAttribute (R_max_version_tag)) {
		if (RKSessionVars::compareRVersion (e.attribute (R_max_version_tag)) < 0) return false;
	}

	return true;
}

QList <RKComponentDependency> RKComponentDependency::parseDependencies (const QDomElement& e, XMLHelper &xml) {
	RK_TRACE (PLUGIN);

	QList<RKComponentDependency> ret;
	if (e.isNull ()) return ret;
	RKComponentDependency dep;

	// Check for R dependency, first.
	dep.type = RKComponentDependency::RBaseInstallation;
	if (e.hasAttribute(R_min_version_tag)) dep.min_version = RKParsedVersion(e.attribute(R_min_version_tag));
	if (e.hasAttribute(R_max_version_tag)) dep.max_version = RKParsedVersion(e.attribute(R_max_version_tag));
	if (!(dep.min_version.isNull() && dep.max_version.isNull())) ret.append(dep);

	XMLChildList deps = xml.getChildElements (e, QString (), DL_INFO);
	for (int i = 0; i < deps.size (); ++i) {
		QDomElement dep_e = deps[i];
		if (dep_e.tagName () == QLatin1String("package")) {
			dep.type = RKComponentDependency::RPackage;
			dep.source_info = xml.getStringAttribute (e, QStringLiteral("repository"), QString (), DL_INFO);
		} else if (dep_e.tagName () == QLatin1String("pluginmap")) {
			dep.type = RKComponentDependency::RKWardPluginmap;
			dep.source_info = xml.getStringAttribute (e, QStringLiteral("url"), QStringLiteral ("https://rkward.kde.org"), DL_WARNING);
		} else {
			RK_DEBUG (PLUGIN, DL_ERROR, "Tag <%s> is not allowed, here.", qPrintable (dep_e.tagName ()));
			continue;
		}
		dep.package = xml.getStringAttribute (dep_e, QStringLiteral("name"), QString (), DL_ERROR);

		if (e.hasAttribute(any_min_version_tag)) dep.min_version = RKParsedVersion(e.attribute(any_min_version_tag));
		if (e.hasAttribute(any_max_version_tag)) dep.max_version = RKParsedVersion(e.attribute(any_max_version_tag));

		ret.append (dep);
	}

	// Add RKWard dependency, last
	dep.type = RKComponentDependency::RKWardVersion;
	dep.source_info.clear ();
	if (e.hasAttribute(rkward_min_version_tag)) dep.min_version = RKParsedVersion(e.attribute(rkward_min_version_tag));
	if (e.hasAttribute(rkward_max_version_tag)) dep.max_version = RKParsedVersion(e.attribute(rkward_max_version_tag));
	if (!(dep.min_version.isNull() && dep.max_version.isNull())) ret.append(dep);

	return ret;
}

QString RKComponentDependency::depsToHtml(const QList<RKComponentDependency>& deps) {
	RK_TRACE(PLUGIN);

	QString ret;
	if (deps.isEmpty()) return ret;

	ret.append(u"<ul>"_s);
	for (int i = 0; i < deps.size(); ++i) {
		ret.append(u"<li>"_s);
		const RKComponentDependency& dep = deps[i];
		if (dep.type == RBaseInstallation) {
			ret.append(u"R"_s);
		} else if (dep.type == RKWardVersion) {
			ret.append(u"RKWard"_s);
		} else {
			if (dep.type == RKWardPluginmap) ret.append(i18n("RKWard plugin map"));
			else ret.append(i18n("R package"));
			ret.append(u" \""_s + dep.package + u"\""_s);
			if (!dep.source_info.isEmpty()) ret.append(u" ("_s + dep.source_info + u')');
		}
		if (!dep.min_version.isNull()) ret.append(u" &gt;= "_s + dep.min_version.toString());
		if (!dep.max_version.isNull()) ret.append(u" &lt;= "_s + dep.max_version.toString());
		ret.append(u"</li>"_s);
	}
	ret.append(u"</ul>"_s);
	return ret;
}
