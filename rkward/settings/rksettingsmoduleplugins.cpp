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
#include "rksettingsmoduleplugins.h"

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include "../rkward.h"
#include "../rkglobals.h"
#include "../misc/multistringselector.h"

// static members
QStringList RKSettingsModulePlugins::plugin_maps;
RKSettingsModulePlugins::PluginPrefs RKSettingsModulePlugins::interface_pref;

RKSettingsModulePlugins::RKSettingsModulePlugins (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	QVBoxLayout *main_vbox = new QVBoxLayout (this, RKGlobals::marginHint ());
	
	main_vbox->addStretch ();
	
	QLabel *label = new QLabel (i18n ("Some plugins are available with both, a wizard-like interface and a traditional dialog interface. If both are available, which mode of presentation do you prefer?"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	
	button_group = new QButtonGroup (this);
	button_group->setColumnLayout (0, Qt::Vertical);
	button_group->layout()->setSpacing (6);
	button_group->layout()->setMargin (11);
	QVBoxLayout *group_layout = new QVBoxLayout(button_group->layout());
	group_layout->addWidget (new QRadioButton (i18n ("Always prefer dialogs"), button_group));
	group_layout->addWidget (new QRadioButton (i18n ("Prefer recommended interface"), button_group));
	group_layout->addWidget (new QRadioButton (i18n ("Always prefer wizards"), button_group));
	button_group->setButton (static_cast<int> (interface_pref));
	connect (button_group, SIGNAL (clicked (int)), this, SLOT (buttonClicked (int)));
	main_vbox->addWidget (button_group);
	
	main_vbox->addStretch ();
	
	map_choser = new MultiStringSelector (i18n ("Select .pluginmap file(s)"), this);
	map_choser->setValues (plugin_maps);
	connect (map_choser, SIGNAL (getNewStrings (QStringList*)), this, SLOT (browseRequest (QStringList*)));
	connect (map_choser, SIGNAL (listChanged ()), this, SLOT (pathsChanged ()));
	main_vbox->addWidget (map_choser);
}

RKSettingsModulePlugins::~RKSettingsModulePlugins() {
}

void RKSettingsModulePlugins::pathsChanged () {
	change ();
}

void RKSettingsModulePlugins::buttonClicked (int) {
	change ();
}

void RKSettingsModulePlugins::browseRequest (QStringList* strings) {
	(*strings) = KFileDialog::getOpenFileNames (KGlobal::dirs()->findResourceDir("plugins", "standard_plugins.pluginmap"), "*.pluginmap", this, i18n ("Select .pluginmap-file"));
}

QString RKSettingsModulePlugins::caption () {
	return (i18n ("Plugins"));
}

bool RKSettingsModulePlugins::hasChanges () {
	return changed;
}

void RKSettingsModulePlugins::applyChanges () {
	plugin_maps = map_choser->getValues ();
#if QT_VERSION < 0x030200
	interface_pref = static_cast<PluginPrefs> (button_group->id (button_group->selected ()));
#else
	interface_pref = static_cast<PluginPrefs> (button_group->selectedId ());
#endif
	RKGlobals::rkApp ()->initPlugins();
}

void RKSettingsModulePlugins::save (KConfig *config) {
	saveSettings (config);
}

void RKSettingsModulePlugins::saveSettings (KConfig *config) {
	config->setGroup ("Plugin Settings");
	config->writeEntry ("Plugin Maps", plugin_maps);
	config->writeEntry ("Interface Preferences", static_cast<int> (interface_pref));
}

void RKSettingsModulePlugins::loadSettings (KConfig *config) {
	config->setGroup ("Plugin Settings");
	plugin_maps = config->readListEntry ("Plugin Maps");
	if (!plugin_maps.count ()) {
		QString dummy = KGlobal::dirs()->findResourceDir("plugins", "standard_plugins.pluginmap");
		if (dummy == "") {
			// try our luck with a relative path
			dummy = "plugins";
		}
		plugin_maps.append (dummy + "/standard_plugins.pluginmap");
	}
	interface_pref = static_cast<PluginPrefs> (config->readNumEntry ("Interface Preferences", static_cast<int> (PreferWizard)));
}

#include "rksettingsmoduleplugins.moc"
