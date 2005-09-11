/***************************************************************************
                          rksettingsmoduleplugins  -  description
                             -------------------
    begin                : Wed Jul 28 2004
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
#ifndef RKSETTINGSMODULEPLUGINS_H
#define RKSETTINGSMODULEPLUGINS_H

#include "rksettingsmodule.h"

#include <qstringlist.h>

class MultiStringSelector;
class QButtonGroup;

/** The settings-module for plugin specific settings
@author Thomas Friedrichsmeier
*/
class RKSettingsModulePlugins : public RKSettingsModule {
	Q_OBJECT
public:
    RKSettingsModulePlugins (RKSettings *gui, QWidget *parent);

    ~RKSettingsModulePlugins ();

	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);
	
	enum PluginPrefs { PreferDialog=0, PreferRecommended=1, PreferWizard=2 };
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();
	
	static QStringList &pluginMaps () { return plugin_maps; };
	static PluginPrefs getInterfacePreference () { return interface_pref; };
public slots:
	void pathsChanged ();
	void buttonClicked (int);
	void browseRequest (QStringList* strings);
private:
	MultiStringSelector *map_choser;
	QButtonGroup *button_group;
	
	static QStringList plugin_maps;
	static PluginPrefs interface_pref;
};

#endif
