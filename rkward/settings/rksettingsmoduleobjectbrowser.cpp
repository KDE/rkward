/***************************************************************************
                          rksettingsmodule  -  description
                             -------------------
    begin                : Fri Apr 22 2005
    copyright            : (C) 2005, 2015 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QInputDialog>

#include "../rkglobals.h"
#include "../misc/multistringselector.h"
#include "rksettings.h"
#include "rksettingsmodulegeneral.h"
#include "../debug.h"

// static
bool RKSettingsModuleObjectBrowser::workspace_settings[RKObjectListViewSettings::SettingsCount];
bool RKSettingsModuleObjectBrowser::varselector_settings[RKObjectListViewSettings::SettingsCount];
QStringList RKSettingsModuleObjectBrowser::getstructure_blacklist;

RKSettingsModuleObjectBrowser::RKSettingsModuleObjectBrowser (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *layout = new QVBoxLayout (this);

	// Note: Up to RKWard 0.6.3, this settings module had a lot of additional checkboxes. Since 0.6.4, most settings are stored, implictily,
	//       i.e. the Workspace Browser tool window "remembers" its latest settings (and so does the Varselector, separately). This modules
	//       is still responsible to storing / loading settings.

	blacklist_choser = new MultiStringSelector (i18n ("Never fetch the structure of these packages:"), this);
	blacklist_choser->setValues (getstructure_blacklist);
	connect (blacklist_choser, &MultiStringSelector::listChanged, this, &RKSettingsModuleObjectBrowser::listChanged);
	connect (blacklist_choser, &MultiStringSelector::getNewStrings, this, &RKSettingsModuleObjectBrowser::addBlackList);
	layout->addWidget (blacklist_choser);
}

RKSettingsModuleObjectBrowser::~RKSettingsModuleObjectBrowser () {
	RK_TRACE (SETTINGS);
}

//static
void RKSettingsModuleObjectBrowser::setDefaultForWorkspace (RKObjectListViewSettings::PersistentSettings setting, bool state) {
	RK_TRACE (SETTINGS);
	workspace_settings[setting] = state;
}

//static
void RKSettingsModuleObjectBrowser::setDefaultForVarselector (RKObjectListViewSettings::PersistentSettings setting, bool state) {
	RK_TRACE (SETTINGS);
	varselector_settings[setting] = state;
}

//static
bool RKSettingsModuleObjectBrowser::isPackageBlacklisted (const QString &package_name) {
	RK_TRACE (SETTINGS);
	return getstructure_blacklist.contains (package_name);
}

void RKSettingsModuleObjectBrowser::addBlackList (QStringList *string_list) {
	RK_TRACE (SETTINGS);
	bool ok;
	QString new_string = QInputDialog::getText (this, i18n ("Add exclusion"), i18n ("Add the name of the package that no structure should be fetched for"), QLineEdit::Normal, QString (), &ok);
	if (ok) (*string_list).append (new_string);
}

void RKSettingsModuleObjectBrowser::applyChanges () {
	RK_TRACE (SETTINGS);

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

void writeSettings (KConfigGroup &cg, bool *settings) {
	cg.writeEntry ("show hidden vars", settings[RKObjectListViewSettings::ShowObjectsHidden]);
	cg.writeEntry ("show label field", settings[RKObjectListViewSettings::ShowFieldsLabel]);
	cg.writeEntry ("show type field", settings[RKObjectListViewSettings::ShowFieldsType]);
	cg.writeEntry ("show class field", settings[RKObjectListViewSettings::ShowFieldsClass]);
}

//static
void RKSettingsModuleObjectBrowser::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Object Browser");
	cg.writeEntry ("package blacklist", getstructure_blacklist);

	KConfigGroup toolgroup = cg.group ("Tool window");
	writeSettings (toolgroup, workspace_settings);
	KConfigGroup varselgroup = cg.group ("Varselector");
	writeSettings (varselgroup, varselector_settings);
}

void readSettings (const KConfigGroup &cg, bool *settings) {
	settings[RKObjectListViewSettings::ShowObjectsHidden] = cg.readEntry ("show hidden vars", false);
	settings[RKObjectListViewSettings::ShowFieldsLabel] = cg.readEntry ("show label field", true);
	settings[RKObjectListViewSettings::ShowFieldsType] = cg.readEntry ("show type field", true);
	settings[RKObjectListViewSettings::ShowFieldsClass] = cg.readEntry ("show class field", true);
}

//static
void RKSettingsModuleObjectBrowser::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Object Browser");
	getstructure_blacklist = cg.readEntry ("package blacklist", QStringList ("GO"));

	readSettings (cg.group ("Tool window"), workspace_settings);
	readSettings (cg.group ("Varselector"), varselector_settings);
}

void RKSettingsModuleObjectBrowser::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleObjectBrowser::listChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

