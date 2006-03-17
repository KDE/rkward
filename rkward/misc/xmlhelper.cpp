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
	RK_TRACE (XML);

	highest_error = 0;
}

XMLHelper::~XMLHelper () {
	RK_TRACE (XML);
}

//static 
XMLHelper *XMLHelper::getStaticHelper () {
	RK_TRACE (XML);

	if (!static_xml_helper) {
		static_xml_helper = new XMLHelper ();
	}
	return static_xml_helper;
}

QDomElement XMLHelper::openXMLFile (const QString &filename, int debug_level) {
	RK_TRACE (XML);

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

XMLChildList XMLHelper::getChildElements (const QDomElement &parent, const QString &name, int debug_level) {
	RK_TRACE (XML);

	XMLChildList list;

	if (!parent.isNull()) {
		QDomNode n = parent.firstChild ();
		while (!n.isNull ()) {
			QDomElement e = n.toElement ();
			if (!e.isNull ()) {
				if ((name.isEmpty ()) || (e.tagName () == name)) {
					list.append (e);
				}
			}
			n = n.nextSibling ();
		}
	} else {
		displayError (&parent, i18n ("Trying to retrieve children of invalid element"), debug_level);
	}

	return (list);
}

QDomElement XMLHelper::getChildElement (const QDomElement &parent, const QString &name, int debug_level) {
	RK_TRACE (XML);

	XMLChildList list = getChildElements (parent, name, debug_level);
	if (list.count () != 1) {
		displayError (&parent, i18n ("Expected exactly one element '%1' but found %2").arg (name).arg (QString::number (list.count ())), debug_level);
		QDomElement dummy;
		return dummy;
	}

	return list.first ();
}

QString XMLHelper::getStringAttribute (const QDomElement &element, const QString &name, const QString &def, int debug_level) {
	RK_TRACE (XML);

	if (!element.hasAttribute (name)) {
		displayError (&element, i18n ("'%1'-attribute not given. Assuming '%2'").arg (name).arg (def), debug_level);
		return (def);
	}

	return (element.attribute (name));
}

int XMLHelper::getMultiChoiceAttribute (const QDomElement &element, const QString &name, const QString &values, int def, int debug_level) {
	RK_TRACE (XML);

	QStringList value_list = QStringList::split (';', values);

	QString plain_value = getStringAttribute (element, name, value_list[def], debug_level);
	
	int index;
	if ((index = value_list.findIndex (plain_value)) >= 0) {
		return (index);
	} else {
		displayError (&element, i18n ("Illegal attribute value. Allowed values are one of '%1', only.").arg (values), debug_level, DL_ERROR);
		return def;
	}
}

int XMLHelper::getIntAttribute (const QDomElement &element, const QString &name, int def, int debug_level) {
	RK_TRACE (XML);

	QString res = getStringAttribute (element, name, QString::number (def), debug_level);

	bool valid_number;;
	int ret = res.toInt (&valid_number);

	if (!valid_number) {
		displayError (&element, i18n ("Illegal attribute value. Only integer numbers are allowed."), debug_level, DL_ERROR);
		return def;
	}

	return ret;
}

double XMLHelper::getDoubleAttribute (const QDomElement &element, const QString &name, double def, int debug_level) {
	RK_TRACE (XML);

	QString res = getStringAttribute (element, name, QString::number (def), debug_level);

	bool valid_number;;
	double ret = res.toDouble (&valid_number);

	if (!valid_number) {
		displayError (&element, i18n ("Illegal attribute value. Only real numbers are allowed."), debug_level, DL_ERROR);
		return def;
	}

	return ret;
}

bool XMLHelper::getBoolAttribute (const QDomElement &element, const QString &name, bool def, int debug_level) {
	RK_TRACE (XML);

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
	RK_TRACE (XML);

	if (message_level < debug_level) message_level = debug_level;
	if (highest_error < debug_level) highest_error = debug_level;

	if ((RK_Debug_Flags & XML) && (message_level >= RK_Debug_Level)) {
		QString backtrace = i18n ("XML-parsing '%1' ").arg (filename);
		// create a "backtrace"
		QStringList list;

		if (in_node) {
			QDomNode node_copy = *in_node;
			while (!((node_copy.isNull ()) || (node_copy.isDocument ()))) {
				list.prepend (node_copy.nodeName ());
				node_copy = node_copy.parentNode ();
			}
		}

		backtrace += list.join ("->");

		RK_DO (qDebug ("%s: %s", backtrace.latin1 (), message.latin1 ()), XML, message_level);
	}
}

