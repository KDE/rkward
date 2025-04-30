/*
rksettingsmodule - This file is part of RKWard (https://rkward.kde.org). Created: Fri Apr 22 2005
SPDX-FileCopyrightText: 2005-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rksettingsmoduleobjectbrowser.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QInputDialog>
#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include "../debug.h"
#include "../misc/multistringselector.h"
#include "../misc/rkstandardicons.h"
#include "rksettings.h"
#include "rksettingsmodulegeneral.h"

// static
RKConfigValue<bool> RKSettingsModuleObjectBrowser::workspace_settings[RKObjectListViewSettings::SettingsCount]{{"show hidden vars", false}, {"show type field", true}, {"show class field", true}, {"show label field", true}};
RKConfigValue<bool> RKSettingsModuleObjectBrowser::varselector_settings[RKObjectListViewSettings::SettingsCount]{RKSettingsModuleObjectBrowser::workspace_settings[0], RKSettingsModuleObjectBrowser::workspace_settings[1], RKSettingsModuleObjectBrowser::workspace_settings[2], RKSettingsModuleObjectBrowser::workspace_settings[3]};
RKConfigValue<QStringList> RKSettingsModuleObjectBrowser::getstructure_blacklist{"package blacklist", QStringList(u"GO"_s)};

class RKSettingsPageObjectBrowser : public RKSettingsModuleWidget {
  public:
	RKSettingsPageObjectBrowser(QWidget *parent, RKSettingsModule *parent_module) : RKSettingsModuleWidget(parent, parent_module, RKSettingsModuleObjectBrowser::page_id) {
		RK_TRACE(SETTINGS);

		setWindowTitle(i18n("Workspace"));
		setWindowIcon(RKStandardIcons::getIcon(RKStandardIcons::WindowWorkspaceBrowser));
		help_url = QUrl(QStringLiteral("rkward://page/rkward_workspace_browser#settings"));

		QVBoxLayout *layout = new QVBoxLayout(this);

		// Note: Up to RKWard 0.6.3, this settings module had a lot of additional checkboxes. Since 0.6.4, most settings are stored, implicitly,
		//       i.e. the Workspace Browser tool window "remembers" its latest settings (and so does the Varselector, separately). This modules
		//       is still responsible to storing / loading settings.

		blacklist_choser = new MultiStringSelector(i18n("Never fetch the structure of these packages:"), this);
		blacklist_choser->setValues(RKSettingsModuleObjectBrowser::getstructure_blacklist);
		connect(blacklist_choser, &MultiStringSelector::listChanged, this, &RKSettingsPageObjectBrowser::change);
		connect(blacklist_choser, &MultiStringSelector::getNewStrings, this, [this](QStringList *string_list) {
			bool ok;
			QString new_string = QInputDialog::getText(this, i18n("Add exclusion"), i18n("Add the name of the package that no structure should be fetched for"), QLineEdit::Normal, QString(), &ok);
			if (ok) (*string_list).append(new_string);
		});
		layout->addWidget(blacklist_choser);
	}
	void applyChanges() override {
		RK_TRACE(SETTINGS);

		RKSettingsModuleObjectBrowser::getstructure_blacklist = blacklist_choser->getValues();
	}

  private:
	MultiStringSelector *blacklist_choser;
};

RKSettingsModuleObjectBrowser::RKSettingsModuleObjectBrowser(QObject *parent) : RKSettingsModule(parent) {
	RK_TRACE(SETTINGS);
}

RKSettingsModuleObjectBrowser::~RKSettingsModuleObjectBrowser() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleObjectBrowser::createPages(RKSettings *parent) {
	parent->addSettingsPage(new RKSettingsPageObjectBrowser(parent, this));
	;
}

// static
void RKSettingsModuleObjectBrowser::setDefaultForWorkspace(RKObjectListViewSettings::PersistentSettings setting, bool state) {
	RK_TRACE(SETTINGS);
	workspace_settings[setting] = state;
}

// static
void RKSettingsModuleObjectBrowser::setDefaultForVarselector(RKObjectListViewSettings::PersistentSettings setting, bool state) {
	RK_TRACE(SETTINGS);
	varselector_settings[setting] = state;
}

// static
bool RKSettingsModuleObjectBrowser::isPackageBlacklisted(const QString &package_name) {
	RK_TRACE(SETTINGS);
	return getstructure_blacklist.get().contains(package_name);
}

void writeSettings(KConfigGroup &cg, bool *settings) {
	cg.writeEntry("show hidden vars", settings[RKObjectListViewSettings::ShowObjectsHidden]);
	cg.writeEntry("show label field", settings[RKObjectListViewSettings::ShowFieldsLabel]);
	cg.writeEntry("show type field", settings[RKObjectListViewSettings::ShowFieldsType]);
	cg.writeEntry("show class field", settings[RKObjectListViewSettings::ShowFieldsClass]);
}

void syncSettings(KConfigGroup cg, RKConfigBase::ConfigSyncAction a, RKConfigValue<bool> *settings) {
	for (int i = 0; i < RKObjectListViewSettings::SettingsCount; ++i) {
		settings[i].syncConfig(cg, a);
	}
}

// static
void RKSettingsModuleObjectBrowser::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group(QStringLiteral("Object Browser"));
	getstructure_blacklist.syncConfig(cg, a);

	syncSettings(cg.group(QStringLiteral("Tool window")), a, workspace_settings);
	syncSettings(cg.group(QStringLiteral("Varselector")), a, varselector_settings);
}
