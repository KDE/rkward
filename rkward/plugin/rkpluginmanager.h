/***************************************************************************
                          rkpluginmanager  -  description
                             -------------------
    begin                : Mon Oct 25 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef RKPLUGINMANAGER_H
#define RKPLUGINMANAGER_H

/**
This class manages plugins and plugin components. I.e., it keeps a list of them, takes care of making them available in the menus etc.

@author Thomas Friedrichsmeier
*/
class RKPluginManager{
public:
	RKPluginManager();

	~RKPluginManager();
	
	void parseDirectory (const QString &dir);
private:
	void registerComponent (const QString &id, const QString &file);
	void registerImportFilter (const QString &format, const QString &file);
};

#endif
