/***************************************************************************
                          rksettingsmoduleplugins  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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
class QCheckBox;
class RKSpinBox;

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
	static bool showCodeByDefault () { return show_code; };
	static int defaultCodeHeight () { return code_size; };
public slots:
	void pathsChanged ();
	void settingChanged (int);
	void browseRequest (QStringList* strings);
private:
	MultiStringSelector *map_choser;
	QButtonGroup *button_group;
	QCheckBox *show_code_box;
	RKSpinBox *code_size_box;
	
	static QStringList plugin_maps;
	static PluginPrefs interface_pref;
	static bool show_code;
	static int code_size;
};

#endif
