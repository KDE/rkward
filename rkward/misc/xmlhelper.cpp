/***************************************************************************
                          xmlhelper.cpp  -  description
                             -------------------
    begin                : Fri May 6 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

#include "xmlhelper.h"

#include <klocale.h>

#include <qstringlist.h>
#include <qfile.h>

#include "../debug.h"

//static
XMLHelper *XMLHelper::static_xml_helper=0;

XMLHelper::XMLHelper () {
	highest_error = 0;
}

XMLHelper::~XMLHelper () {
}

//static 
XMLHelper *XMLHelper::getStaticHelper () {
	if (!static_xml_helper) {
		static_xml_helper = new XMLHelper ();
	}
	return static_xml_helper;
}

QDomElement XMLHelper::openXMLFile (const QString &filename, int debug_level) {
	int error_line, error_column;
	QString error_message;
	QDomDocument doc;

	XMLHelper::filename = filename;
	highest_error = 0;
	
	QFile f (filename);
	if (!f.open (IO_ReadOnly)) displayError (0, i18n("Could not open file for reading"), debug_level, DL_ERROR);
	if (!doc.setContent(&f, false, &error_message, &error_line, &error_column)) {
		displayError (0, i18n ("Error parsing XML-file. Error-message was: '%1' in line '%2', column '%3'. Expect further errors to be reported below").arg (error_message).arg (QString::number (error_line)).arg (QString::number (error_column)), debug_level, DL_ERROR);
	}
	f.close();

	return doc.documentElement ();
}

QDomNodeList XMLHelper::getChildElements (const QDomElement &parent, const QString &name, int debug_level) {
	QString convert;

	if (!parent.isNull()) {
		if (name != "") {
			return (parent.elementsByTagName (name));
		} else {
			return (parent.childNodes ());
		}
	} else {
		displayError (&parent, i18n ("Trying to retrieve children of invalid element"), debug_level);
	}

	QDomNodeList list;
	return (list);
}

QString XMLHelper::getStringAttribute (const QDomElement &element, const QString &name, const QString &def, int debug_level) {
	if (!element.hasAttribute (name)) {
		displayError (&element, i18n ("'%1'-attribute not given. Assuming '%2'").arg (name).arg (def), debug_level);
		return (def);
	}

	return (element.attribute (name));
}

bool XMLHelper::getBoolAttribute (const QDomElement &element, const QString &name, bool def, int debug_level) {
	QString defstring, res;
	if (def) defstring = "true";
	else defstring = "false";

	res = getStringAttribute (element, name, defstring, debug_level);
	if (res == "true") return true;
	if (res == "false") return false;

	displayError (&element, i18n ("Illegal attribute value. Allowed values are 'true' or 'false', only."), debug_level, DL_ERROR);
	return def;
}

void XMLHelper::displayError (const QDomNode *in_node, const QString &message, int debug_level, int message_level) {
	if (message_level < debug_level) message_level = debug_level;
	if (highest_error < debug_level) highest_error = debug_level;

	if ((RK_Debug_Flags & XML) && (message_level >= RK_Debug_Level)) {
		QString backtrace = i18n ("XML-parsing '%1' ").arg (filename);
		// create a "backtrace"
		QStringList list;

		QDomNode node_copy = *in_node;
		while (!((node_copy.isDocument ()) || (node_copy.isNull ()))) {
			list.prepend (node_copy.nodeName ());
			node_copy = node_copy.parentNode ();
		}

		backtrace += list.join ("->");

		RK_DO (qDebug ("%s: %s", backtrace.latin1 (), message.latin1 ()), XML, message_level);
	}
}

