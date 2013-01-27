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
#include <kconfiggroup.h>
#include <kinputdialog.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <QVBoxLayout>

#include "../rkglobals.h"
#include "../misc/multistringselector.h"
#include "rksettings.h"
#include "../debug.h"

// static
bool RKSettingsModuleObjectBrowser::settings[RKObjectListViewSettings::SettingsCount];
QStringList RKSettingsModuleObjectBrowser::getstructure_blacklist;

RKSettingsModuleObjectBrowser::RKSettingsModuleObjectBrowser (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	checkboxes = new QCheckBox*[RKObjectListViewSettings::SettingsCount];

	QVBoxLayout *layout = new QVBoxLayout (this);

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

	blacklist_choser = new MultiStringSelector (i18n ("Never fetch the structure of these packages:"), this);
	blacklist_choser->setValues (getstructure_blacklist);
	connect (blacklist_choser, SIGNAL (listChanged ()), this, SLOT (listChanged ()));
	connect (blacklist_choser, SIGNAL (getNewStrings (QStringList*)), this, SLOT (addBlackList (QStringList*)));
	layout->addWidget (blacklist_choser);
}

RKSettingsModuleObjectBrowser::~RKSettingsModuleObjectBrowser () {
	RK_TRACE (SETTINGS);
}

//static
bool RKSettingsModuleObjectBrowser::isSettingActive (RKObjectListViewSettings::Settings setting) {
	RK_TRACE (SETTINGS);
	return settings[setting];
}

//static
bool RKSettingsModuleObjectBrowser::isPackageBlacklisted (const QString &package_name) {
	RK_TRACE (SETTINGS);
	return getstructure_blacklist.contains (package_name);
}

void RKSettingsModuleObjectBrowser::addBlackList (QStringList *string_list) {
	RK_TRACE (SETTINGS);
	QString new_string = KInputDialog::getText (i18n ("Add exclusion"), i18n ("Add the name of the package that no structure should be fetched for"), QString::null, 0, this);
	(*string_list).append (new_string);
}

void RKSettingsModuleObjectBrowser::applyChanges () {
	RK_TRACE (SETTINGS);

	for (int i = 0; i < RKObjectListViewSettings::SettingsCount; ++i) {
		settings[i] = checkboxes[i]->isChecked ();
	}
	getstructure_blacklist = blacklist_choser->getValues();
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

	KConfigGroup cg = config->group ("Object Browser");
	cg.writeEntry ("show hidden vars", settings[RKObjectListViewSettings::ShowObjectsHidden]);
	cg.writeEntry ("show all environments", settings[RKObjectListViewSettings::ShowObjectsAllEnvironments]);
	cg.writeEntry ("show container objects", settings[RKObjectListViewSettings::ShowObjectsContainer]);
	cg.writeEntry ("show function objects", settings[RKObjectListViewSettings::ShowObjectsFunction]);
	cg.writeEntry ("show variable objects", settings[RKObjectListViewSettings::ShowObjectsVariable]);
	cg.writeEntry ("show label field", settings[RKObjectListViewSettings::ShowFieldsLabel]);
	cg.writeEntry ("show type field", settings[RKObjectListViewSettings::ShowFieldsType]);
	cg.writeEntry ("show class field", settings[RKObjectListViewSettings::ShowFieldsClass]);

	cg.writeEntry ("package blacklist", getstructure_blacklist);
}

//static
void RKSettingsModuleObjectBrowser::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Object Browser");
	settings[RKObjectListViewSettings::ShowObjectsHidden] = cg.readEntry ("show hidden vars", false);
	settings[RKObjectListViewSettings::ShowObjectsAllEnvironments] = cg.readEntry ("show all environments", true);
	settings[RKObjectListViewSettings::ShowObjectsContainer] = cg.readEntry ("show container objects", true);
	settings[RKObjectListViewSettings::ShowObjectsFunction] = cg.readEntry ("show function objects", true);
	settings[RKObjectListViewSettings::ShowObjectsVariable] = cg.readEntry ("show variable objects", true);
	settings[RKObjectListViewSettings::ShowFieldsLabel] = cg.readEntry ("show label field", true);
	settings[RKObjectListViewSettings::ShowFieldsType] = cg.readEntry ("show type field", true);
	settings[RKObjectListViewSettings::ShowFieldsClass] = cg.readEntry ("show class field", true);

	getstructure_blacklist = cg.readEntry ("package blacklist", QStringList ("GO"));
}

void RKSettingsModuleObjectBrowser::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleObjectBrowser::listChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

#include "rksettingsmoduleobjectbrowser.moc"
