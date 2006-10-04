/***************************************************************************
                          rksettingsmodule  -  description
                             -------------------
    begin                : Fri Apr 22 2005
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

#include "rksettingsmoduleobjectbrowser.h"

#include <klocale.h>
#include <kconfig.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include "../rkglobals.h"
#include "rksettings.h"
#include "../debug.h"

// static
bool RKSettingsModuleObjectBrowser::settings[RKObjectListViewSettings::SettingsCount];

RKSettingsModuleObjectBrowser::RKSettingsModuleObjectBrowser (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	checkboxes = new QCheckBox*[RKObjectListViewSettings::SettingsCount];

	QVBoxLayout *layout = new QVBoxLayout (this, RKGlobals::marginHint ());

	layout->addWidget (new QLabel (i18n ("Which objects should be shown by default?"), this));

	checkboxes[RKObjectListViewSettings::ShowObjectsHidden] = new QCheckBox (i18n ("Show hidden objects"), this);
	layout->addWidget (checkboxes[RKObjectListViewSettings::ShowObjectsHidden]);
	checkboxes[RKObjectListViewSettings::ShowObjectsAllEnvironments] = new QCheckBox (i18n ("Show all environments"), this);
	layout->addWidget (checkboxes[RKObjectListViewSettings::ShowObjectsAllEnvironments]);
	layout->addSpacing (2*RKGlobals::spacingHint ());
	checkboxes[RKObjectListViewSettings::ShowObjectsContainer] = new QCheckBox (i18n ("Show objects with children"), this);
	layout->addWidget (checkboxes[RKObjectListViewSettings::ShowObjectsContainer]);
	checkboxes[RKObjectListViewSettings::ShowObjectsFunction] = new QCheckBox (i18n ("Show functions"), this);
	layout->addWidget (checkboxes[RKObjectListViewSettings::ShowObjectsFunction]);
	checkboxes[RKObjectListViewSettings::ShowObjectsVariable] = new QCheckBox (i18n ("Show variables"), this);
	layout->addWidget (checkboxes[RKObjectListViewSettings::ShowObjectsVariable]);

	layout->addSpacing (2*RKGlobals::spacingHint ());

	layout->addWidget (new QLabel (i18n ("Which columns should be shown by default?"), this));

	checkboxes[RKObjectListViewSettings::ShowFieldsLabel] = new QCheckBox (i18n ("Label field"), this);
	layout->addWidget (checkboxes[RKObjectListViewSettings::ShowFieldsLabel]);
	checkboxes[RKObjectListViewSettings::ShowFieldsType] = new QCheckBox (i18n ("Type field"), this);
	layout->addWidget (checkboxes[RKObjectListViewSettings::ShowFieldsType]);
	checkboxes[RKObjectListViewSettings::ShowFieldsClass] = new QCheckBox (i18n ("Class field"), this);
	layout->addWidget (checkboxes[RKObjectListViewSettings::ShowFieldsClass]);

	layout->addStretch ();

	for (int i = 0; i < RKObjectListViewSettings::SettingsCount; ++i) {
		checkboxes[i]->setChecked (settings[i]);
		connect (checkboxes[i], SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	}
}

RKSettingsModuleObjectBrowser::~RKSettingsModuleObjectBrowser () {
	RK_TRACE (SETTINGS);
}

//static
bool RKSettingsModuleObjectBrowser::isSettingActive (RKObjectListViewSettings::Settings setting) {
	RK_TRACE (SETTINGS);
	return settings[setting];
}

bool RKSettingsModuleObjectBrowser::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleObjectBrowser::applyChanges () {
	RK_TRACE (SETTINGS);

	for (int i = 0; i < RKObjectListViewSettings::SettingsCount; ++i) {
		settings[i] = checkboxes[i]->isChecked ();
	}

	RKSettings::tracker ()->settingsChangedObjectBrowser ();
}

void RKSettingsModuleObjectBrowser::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

QString RKSettingsModuleObjectBrowser::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Workspace"));
}

//static
void RKSettingsModuleObjectBrowser::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Object Browser");
	config->writeEntry ("show hidden vars", settings[RKObjectListViewSettings::ShowObjectsHidden]);
	config->writeEntry ("show all environments", settings[RKObjectListViewSettings::ShowObjectsAllEnvironments]);
	config->writeEntry ("show container objects", settings[RKObjectListViewSettings::ShowObjectsContainer]);
	config->writeEntry ("show function objects", settings[RKObjectListViewSettings::ShowObjectsFunction]);
	config->writeEntry ("show variable objects", settings[RKObjectListViewSettings::ShowObjectsVariable]);
	config->writeEntry ("show label field", settings[RKObjectListViewSettings::ShowFieldsLabel]);
	config->writeEntry ("show type field", settings[RKObjectListViewSettings::ShowFieldsType]);
	config->writeEntry ("show class field", settings[RKObjectListViewSettings::ShowFieldsClass]);
}

//static
void RKSettingsModuleObjectBrowser::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Object Browser");
	settings[RKObjectListViewSettings::ShowObjectsHidden] = config->readBoolEntry ("show hidden vars", false);
	settings[RKObjectListViewSettings::ShowObjectsAllEnvironments] = config->readBoolEntry ("show all environments", true);
	settings[RKObjectListViewSettings::ShowObjectsContainer] = config->readBoolEntry ("show container objects", true);
	settings[RKObjectListViewSettings::ShowObjectsFunction] = config->readBoolEntry ("show function objects", true);
	settings[RKObjectListViewSettings::ShowObjectsVariable] = config->readBoolEntry ("show variable objects", true);
	settings[RKObjectListViewSettings::ShowFieldsLabel] = config->readBoolEntry ("show label field", true);
	settings[RKObjectListViewSettings::ShowFieldsType] = config->readBoolEntry ("show type field", true);
	settings[RKObjectListViewSettings::ShowFieldsClass] = config->readBoolEntry ("show class field", true);
}

void RKSettingsModuleObjectBrowser::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

#include "rksettingsmoduleobjectbrowser.moc"
