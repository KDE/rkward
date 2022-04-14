/*
rksettingsmodule - This file is part of RKWard (https://rkward.kde.org). Created: Fri Apr 22 2005
SPDX-FileCopyrightText: 2005-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rksettingsmoduleobjectbrowser.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QInputDialog>

#include "../misc/multistringselector.h"
#include "../misc/rkstandardicons.h"
#include "rksettings.h"
#include "rksettingsmodulegeneral.h"
#include "../debug.h"

// static
RKConfigValue<bool> RKSettingsModuleObjectBrowser::workspace_settings[RKObjectListViewSettings::SettingsCount] {{"show hidden vars", false},{"show type field", true},{"show class field", true},{"show label field", true}};
RKConfigValue<bool> RKSettingsModuleObjectBrowser::varselector_settings[RKObjectListViewSettings::SettingsCount] { RKSettingsModuleObjectBrowser::workspace_settings[0],RKSettingsModuleObjectBrowser::workspace_settings[1],RKSettingsModuleObjectBrowser::workspace_settings[2],RKSettingsModuleObjectBrowser::workspace_settings[3]};
RKConfigValue<QStringList> RKSettingsModuleObjectBrowser::getstructure_blacklist {"package blacklist", QStringList("GO")};

RKSettingsModuleObjectBrowser::RKSettingsModuleObjectBrowser (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *layout = new QVBoxLayout (this);

	// Note: Up to RKWard 0.6.3, this settings module had a lot of additional checkboxes. Since 0.6.4, most settings are stored, implicitly,
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
	return getstructure_blacklist.get().contains (package_name);
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

QString RKSettingsModuleObjectBrowser::caption() const {
	RK_TRACE(SETTINGS);
	return(i18n("Workspace"));
}

QIcon RKSettingsModuleObjectBrowser::icon() const {
	RK_TRACE(SETTINGS);
	return RKStandardIcons::getIcon(RKStandardIcons::WindowWorkspaceBrowser);
}

void writeSettings (KConfigGroup &cg, bool *settings) {
	cg.writeEntry ("show hidden vars", settings[RKObjectListViewSettings::ShowObjectsHidden]);
	cg.writeEntry ("show label field", settings[RKObjectListViewSettings::ShowFieldsLabel]);
	cg.writeEntry ("show type field", settings[RKObjectListViewSettings::ShowFieldsType]);
	cg.writeEntry ("show class field", settings[RKObjectListViewSettings::ShowFieldsClass]);
}

void syncSettings(KConfigGroup cg, RKConfigBase::ConfigSyncAction a, RKConfigValue<bool> *settings) {
	for (int i = 0; i < RKObjectListViewSettings::SettingsCount; ++i) {
		settings[i].syncConfig(cg, a);
	}
}

//static
void RKSettingsModuleObjectBrowser::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group("Object Browser");
	getstructure_blacklist.syncConfig(cg, a);

	syncSettings (cg.group("Tool window"), a, workspace_settings);
	syncSettings (cg.group("Varselector"), a, varselector_settings);
}

void RKSettingsModuleObjectBrowser::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleObjectBrowser::listChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

