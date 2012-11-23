/***************************************************************************
                          rkcommonfunctions  -  description
                             -------------------
    begin                : Mon Oct 17 2005
    copyright            : (C) 2005, 2006, 2007, 2009, 2010, 2011 by Thomas Friedrichsmeier
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
#include "rkcommonfunctions.h"

#include <qstringlist.h>
#include <qdom.h>
#include <qregexp.h>
#include <QDir>

#include <klocale.h>
#include <kxmlguiclient.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "../settings/rksettingsmodulegeneral.h"

namespace RKCommonFunctions {
	void removeNamedElementsRecursive (const QStringList &names, QDomNode &parent) {
		QDomNode nchild;

		for (QDomNode child = parent.firstChild (); !child.isNull (); child = nchild) {
			removeNamedElementsRecursive (names, child);

			nchild = child.nextSibling ();		// need to fetch next sibling here, as we might remove the child below
			if (child.isElement ()) {
				QDomElement e = child.toElement ();
				if (names.contains (e.attribute ("name"))) {
					parent.removeChild (child);
				}
			}
		}
	}
	
	void removeContainers (KXMLGUIClient *from, const QStringList &names, bool recursive) {
		QDomDocument doc = from->xmlguiBuildDocument ();
		if  (doc.documentElement ().isNull ()) doc = from->domDocument ();
	
		QDomElement e = doc.documentElement ();
		removeNamedElementsRecursive (names, e);
		from->setXMLGUIBuildDocument (doc);
	
		if (recursive) {
			QList<KXMLGUIClient*> children = from->childClients ();
			QList<KXMLGUIClient*>::const_iterator it;
			for (it = children.constBegin (); it != children.constEnd (); ++it) {
				removeContainers ((*it), names, true);
			}
		}
	}

	void moveContainer (KXMLGUIClient *client, const QString &tagname, const QString &name, const QString &to_name, bool recursive) {
		QDomDocument doc = client->xmlguiBuildDocument ();
		if  (doc.documentElement ().isNull ()) doc = client->domDocument ();
	
		// find the given elements
		QDomElement e = doc.documentElement ();

		QDomElement from_elem;
		QDomElement to_elem;

		QDomNodeList list = e.elementsByTagName (tagname);
		int count = list.count ();
		for (int i = 0; i < count; ++i) {
			QDomElement elem = list.item (i).toElement ();
			if (elem.isNull ()) continue;
			if (elem.attribute ("name") == name) {
				from_elem = elem;
			} else if (elem.attribute ("name") == to_name) {
				to_elem = elem;
			}
		}

		// move
		from_elem.parentNode ().removeChild (from_elem);
		to_elem.appendChild (from_elem);

		// set result
		client->setXMLGUIBuildDocument (doc);

		// recurse
		if (recursive) {
			QList<KXMLGUIClient*> children = client->childClients ();
			QList<KXMLGUIClient*>::const_iterator it;
			for (it = children.constBegin (); it != children.constEnd (); ++it) {
				moveContainer (*it, tagname, name, to_name, true);
			}
		}
	}

	QString getCurrentSymbol (const QString &context_line, int cursor_pos, bool strict) {
		if (context_line.isEmpty ()) return (QString::null);

		int current_word_start;
		int current_word_end;
		getCurrentSymbolOffset (context_line, cursor_pos, strict, &current_word_start, &current_word_end);
	
		// if both return the same position, we're on a non-word.
		if (current_word_start == current_word_end) return (QString ());

		return (context_line.mid (current_word_start, current_word_end - current_word_start));
	}

	void getCurrentSymbolOffset (const QString &context_line, int cursor_pos, bool strict, int *start, int *end) {
		*start = 0;

		int line_end = context_line.length () - 1;
		*end = line_end + 1;
		if (cursor_pos > line_end) cursor_pos = line_end;

		QChar quote_char;
		for (int i=0; i <= line_end; ++i) {
			QChar c = context_line.at (i);
			if (!quote_char.isNull ()) {
				if (c == '\\') ++i;
				if (c == quote_char) quote_char = QChar ();
				continue;
			} else {
				if (c == '\'' || c == '\"' || c == '`') {
					quote_char = c;
					continue;
				} else if (c.isLetterOrNumber () || c == '.' || c == '_') {
					continue;
				} else if (!strict) {
					if (c == '$' || c == ':' || c == '[' || c == ']' || c == '@') continue;
				}
			}

			// if we did not hit a continue, yet, that means we are on a potential symbol boundary
			if (i < cursor_pos) *start = i+1;
			else {
				*end = i;
				break;
			}
		}
	}

	QString getRKWardDataDir () {
		return (KGlobal::dirs ()->findResourceDir ("data", "rkward/resource.ver") + "rkward/");
	}

	QString getUseableRKWardSavefileName (const QString &prefix, const QString &postfix) {
		QDir dir (RKSettingsModuleGeneral::filesPath ());

		int i=0;
		while (true) {
			QString candidate = prefix + QString::number (i) + postfix;
			if (!dir.exists (candidate)) {
				return dir.filePath (candidate);
			}
			i++;
		}
	}

	QString escape (const QString &in) {
		QString out;

		for (int i = 0; i < in.size (); ++i) {
			QChar c = in[i];
			if (c == '\\') out.append ("\\\\");
			else if (c == '\n') out.append ("\\n");
			else if (c == '\t') out.append ("\\t");
			else if (c == '"') out.append ("\\\"");
			else out.append (c);
		}

		return out;
	}

	QString unescape (const QString &in) {
		QString out;
		out.reserve (in.size ());

		for (int i = 0; i < in.size (); ++i) {
			QChar c = in[i];
			if (c == '\\') {
				++i;
				if (i >= in.size ()) break;
				c = in[i];
				if (c == 'n') c = '\n';
				else if (c == 't') c = '\t';
				// NOTE: Quote (") and backslash (\) are escaped by the same symbol, i.e. c = in[i] is good enough
			}
			out.append (c);
		}

		return out;
	}

	void setTips (const QString tip, QWidget *first, QWidget *second, QWidget *third) {
		for (int i=0; i < 3; ++i) {
			QWidget *w = first;
			if (i == 1) w = second;
			if (i == 2) w = third;
			if (!w) return;

			w->setToolTip (tip);
			w->setWhatsThis (tip);
		}
	}

	QString noteSettingsTakesEffectAfterRestart () {
		return (i18n ("<p><em>Note:</em> This setting does not take effect until you restart RKWard.</p>"));
	}

}	// namespace
