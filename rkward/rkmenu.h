/***************************************************************************
                          rkmenu.h  -  description
                             -------------------
    begin                : Wed Nov 6 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#ifndef RKMENU_H
#define RKMENU_H

#include <qpopupmenu.h>

#include <qmap.h>
#include <qstring.h>

class QMenuBar;
class RKPlugin;
class RKwardApp;
class QDomElement;

/**
  *@author Thomas Friedrichsmeier
  */

class RKMenu : public QPopupMenu  {
public: 
	RKMenu(RKMenu *parent, QString tag, QString label, RKwardApp *app);
	RKMenu(QMenuBar *parent, QString tag, QString label, RKwardApp *app);
	~RKMenu();
	void addSubMenu (const QString &id, RKMenu *submenu);
	void addEntry (const QString &id, RKPlugin *plugin);
	QString label ();
private:
/** TODO: Probably we neither need to keep the tag, the parent, nor is_top_level.
	And probably, we don't ever have to propagate RKPlugin *s backwards.
	But - we'll clean up later... */
	QMap<QString, RKMenu*> submenus;
	QMap<QString, RKPlugin*> entries;
	bool is_top_level;
	QString _tag;
	QString _label;
	RKwardApp *app;
};

#endif
