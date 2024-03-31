/*
rkcommonfunctions - This file is part of the RKWard project. Created: Mon Oct 17 2005
SPDX-FileCopyrightText: 2005-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKCOMMONFUNCTIONS_H
#define RKCOMMONFUNCTIONS_H

#include <QChar>
#include <QRect>
#include <QString>

class QDomNode;
class KXMLGUIClient;
class QWidget;
class QLabel;

/** Some common static helper functions that don't really belong to any class in particular. If ever we have more than a dozen or so functions in here,
we should probably split this file up. Until then, there's no real need.

@author Thomas Friedrichsmeier
*/
namespace RKCommonFunctions {
/** remove QDomElements with attribute 'name="..."' from QDomNode parent, where "..." is any of the strings in names */
	void removeNamedElementsRecursive (const QStringList &names, QDomNode &parent);
/** remove containers (actions, menus, etc.) with attribute 'name="..."' from KXMLGUIClient from s XML gui, where "..." is any of the strings in names. If recursive, also removes those containers from child clients. */
	void removeContainers (KXMLGUIClient *from, const QStringList &names, bool recursive);
/** move container (action, menu, etc.) with tagname "tagname" and attribute 'name="..."' to be a child node of the tag with tagname=tagname and attribute name=to_name. Can be used to make a top-level menu a sub-menu of another menu instead */
	void moveContainer (KXMLGUIClient *client, const QString &tagname, const QString &name, const QString &to_name, bool recursive, bool flatten=false);

/** Get the base directory where RKWard data files are stored */
	QString getRKWardDataDir ();

/** given a context line, find the end of a quote started by quote_char. @returns -1 if not end of quote was found. */
	int quoteEndPosition (const QChar& quote_char, const QString& haystack, int from = 0);

/** given the context line, find what looks like an R symbol */
	QString getCurrentSymbol (const QString &context_line, int cursor_pos, bool strict=true);
/** like get current symbol, but merely returns the start and end position of the current symbol */
	void getCurrentSymbolOffset (const QString &context_line, int cursor_pos, bool strict, int *start, int *end);

/** escape special chars in a QString */
	QString escape (const QString &in);
/** reverse of escape () */
	QString unescape (const QString &in);

/** simultaneously sets tool tips and what's this tips on up to three QWidgets */
	void setTips (const QString &tip, QWidget *first, QWidget *second=nullptr, QWidget *third=nullptr);
	QString noteSettingsTakesEffectAfterRestart ();

/** create a QLabel that has wordwarp enabled, in a single line of code. */
	QLabel* wordWrappedLabel (const QString &text);
/** create a QLabel that has wordwarp enabled, *and* clickable links (opened inside RKWard), in a single line of code. */
	QLabel* linkedWrappedLabel (const QString &text);
};

#endif
