/*
rkcommonfunctions - This file is part of RKWard (https://rkward.kde.org). Created: Mon Oct 17 2005
SPDX-FileCopyrightText: 2005-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkcommonfunctions.h"

#include <qstringlist.h>
#include <qdom.h>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QLabel>

#include <KLocalizedString>
#include <kxmlguiclient.h>

#include "../windows/rkworkplace.h"
#include "../version.h"
#include "../debug.h"

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

	void moveContainer (KXMLGUIClient *client, const QString &tagname, const QString &name, const QString &to_name, bool recursive, bool flatten) {
		// recurse first
		if (recursive) {
			QList<KXMLGUIClient*> children = client->childClients ();
			QList<KXMLGUIClient*>::const_iterator it;
			for (it = children.constBegin (); it != children.constEnd (); ++it) {
				moveContainer (*it, tagname, name, to_name, true);
			}
		}

		QDomDocument doc = client->xmlguiBuildDocument ();
		if  (doc.documentElement ().isNull ()) doc = client->domDocument ();
	
		// find the given elements
		QDomElement e = doc.documentElement ();

		QDomElement from_elem;
		QDomElement to_elem;

		const QString name_attr ("name");
		const QDomNodeList list = e.elementsByTagName (tagname);
		int count = list.count ();
		for (int i = 0; i < count; ++i) {
			const QDomElement elem = list.item (i).toElement ();
			if (elem.isNull ()) continue;
			if (elem.attribute (name_attr) == name) {
				from_elem = elem;
			} else if (elem.attribute (name_attr) == to_name) {
				to_elem = elem;
			}
		}

		if (from_elem.isNull ()) return;
		if (to_elem.isNull ()) {  // if no place to move to, just rename (Note: children will be moved, below)
			to_elem = from_elem.cloneNode (false).toElement ();
			to_elem.setAttribute (name_attr, to_name);
			from_elem.parentNode().appendChild(to_elem);
		}
		// move
		from_elem.parentNode ().removeChild (from_elem);
		if (flatten) {
			while (from_elem.hasChildNodes()) {
				to_elem.appendChild (from_elem.firstChild());
			}
		} else {
			to_elem.appendChild (from_elem);
		}

		// set result
		client->setXMLGUIBuildDocument (doc);
	}

	QString getCurrentSymbol (const QString &context_line, int cursor_pos, bool strict) {
		if (context_line.isEmpty ()) return (QString ());

		int current_word_start;
		int current_word_end;
		getCurrentSymbolOffset (context_line, cursor_pos, strict, &current_word_start, &current_word_end);
	
		// if both return the same position, we're on a non-word.
		if (current_word_start == current_word_end) return (QString ());

		return (context_line.mid (current_word_start, current_word_end - current_word_start));
	}

	int quoteEndPosition (const QChar& quote_char, const QString& haystack, int from) {
		int line_end = haystack.length () - 1;
		for (int i=from; i <= line_end; ++i) {
			QChar c = haystack.at (i);
			if (c == quote_char) return i;
			if (c == '\\') {
				++i;
				continue;
			}
		}
		return -1; // quote end not found
	}

	void getCurrentSymbolOffset (const QString &context_line, int cursor_pos, bool strict, int *start, int *end) {
		*start = 0;

		int line_end = context_line.length () - 1;
		*end = line_end + 1;
		if (cursor_pos > line_end) cursor_pos = line_end;

		for (int i=0; i <= line_end; ++i) {
			QChar c = context_line.at (i);
			if (c == '\'' || c == '\"' || c == '`') {
				i = quoteEndPosition (c, context_line, i+1);
				if (i < 0) break;
				continue;
			} else if (c.isLetterOrNumber () || c == '.' || c == '_') {
				continue;
			} else if (!strict) {
				if (c == '$' || c == ':' || c == '[' || c == ']' || c == '@') continue;
			}

			// if we did not hit a continue, yet, that means we are on a potential symbol boundary
			if (i <= cursor_pos) *start = i+1;
			else {
				*end = i;
				break;
			}
		}
	}

	static QString findPathFromAppDir(const QStringList &candidates) {
		for (int i = 0;  i < candidates.size(); ++i) {
			QString candidate = QCoreApplication::applicationDirPath() + '/' + candidates[i] + '/';
			if (QFile::exists(candidate)) return candidate;
		}
		return QString();
	}

	QString getRKWardDataDir () {
		static QString rkward_data_dir;
		if (rkward_data_dir.isNull ()) {
			QString inside_build_tree = findPathFromAppDir(QStringList() << "rkwardinstall" << "../rkwardinstall" << "../rkward/rkwardinstall");
			if (!inside_build_tree.isEmpty()) {
				RK_DEBUG(APP, DL_INFO, "Running from inside build tree");
				rkward_data_dir = inside_build_tree;
				return rkward_data_dir;
			}
			QStringList candidates = QStandardPaths::locateAll (QStandardPaths::AppDataLocation, "resource.ver");
			candidates += QStandardPaths::locateAll (QStandardPaths::AppDataLocation, "rkward/resource.ver");  // Well, isn't this just silly? AppDataLocation may or may not contain the application name (on Mac)
			for (int i = 0; i < candidates.size (); ++i) {
				QFile resource_ver (candidates[i]);
				if (resource_ver.open (QIODevice::ReadOnly) && (resource_ver.read (100).trimmed () == RKWARD_VERSION)) {
					rkward_data_dir = candidates[i].replace ("resource.ver", QString ());
					return rkward_data_dir;
				}
			}
			rkward_data_dir = "";   // prevents checking again
			RK_DEBUG(APP, DL_ERROR, "resource.ver not found. Data path(s): %s", qPrintable (QStandardPaths::standardLocations (QStandardPaths::AppDataLocation).join (':')));
		}
		return rkward_data_dir;
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

	void setTips (const QString &tip, QWidget *first, QWidget *second, QWidget *third) {
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

	QLabel* wordWrappedLabel (const QString& text) {
		QLabel* ret = new QLabel (text);
		ret->setWordWrap (true);
		return ret;
	}

	QLabel* linkedWrappedLabel (const QString& text) {
		QLabel* ret = wordWrappedLabel (text);
		QObject::connect (ret, &QLabel::linkActivated, RKWorkplace::mainWorkplace (), &RKWorkplace::openAnyUrlString);
		return ret;
	}
}	// namespace
