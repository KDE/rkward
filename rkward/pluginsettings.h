/***************************************************************************
                          pluginsettings.h  -  description
                             -------------------
    begin                : Sun Nov 10 2002
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

#ifndef PLUGINSETTINGS_H
#define PLUGINSETTINGS_H

#include <pluginsettingsui.h>

class RKwardApp;

/**
  *@author Thomas Friedrichsmeier
  */

class PluginSettings : public PluginSettingsUi  {
   Q_OBJECT
public: 
	PluginSettings(RKwardApp *parent, const char *name=0);
	~PluginSettings();
public slots:
	void slotCancel ();
	void slotReLoad ();
	void slotBrowse ();
private:
	RKwardApp *_parent;
};

#endif
