/*
rksettingsmoduleoutput - This file is part of the RKWard project. Created: Fri Jul 30 2004
SPDX-FileCopyrightText: 2004-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULEOUTPUT_H
#define RKSETTINGSMODULEOUTPUT_H

#include "rksettingsmodule.h"

#include <QStringList>

class QCheckBox;
class QGroupBox;
class QComboBox;
class QSpinBox;
class RCommand;
class GetFileNameWidget;

/**
Allows to configure which types of commands should be "carbon copied" to the output window. Like the RKSettingsModules classes, this class encapsulates both, the setting itself,
and a widget to configure the settings.
@author Thomas Friedrichsmeier
*/
class RKCarbonCopySettings : public RKSettingsModuleWidget {
	Q_OBJECT
  public:
	explicit RKCarbonCopySettings(QWidget *parent, RKSettingsModuleWidget *page);
	~RKCarbonCopySettings();

	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);

	static bool shouldCarbonCopyCommand(const RCommand *command);
	static bool includeOutputInCarbonCopy() { return (cc_globally_enabled && cc_command_output); };
  public Q_SLOTS:
	void applyChanges() override;

  private:
	// There can be multiple instances of this widget, which need to be kept in sync.
	static QList<RKCarbonCopySettings *> instances;

	static RKConfigValue<bool> cc_globally_enabled;
	static RKConfigValue<bool> cc_console_commands;
	static RKConfigValue<bool> cc_script_commands;
	static RKConfigValue<bool> cc_app_plugin_commands;
	static RKConfigValue<bool> cc_command_output;
};

/**
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleOutput : public RKSettingsModule {
	Q_OBJECT
  public:
	explicit RKSettingsModuleOutput(QObject *parent);
	~RKSettingsModuleOutput() override;

	void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction) override;
	void createPages(RKSettings *parent) override;
	static constexpr PageId page_id = QLatin1String("output");

	/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands();

	static bool autoShow() { return auto_show; };
	static bool autoRaise() { return auto_raise; };
	static bool sharedDefaultOutput() { return shared_default_output; };

  private:
	friend class RKSettingsPageOutput;
	static RKConfigValue<bool> auto_show;
	static RKConfigValue<bool> auto_raise;
	static RKConfigValue<QString> graphics_type;
	static RKConfigValue<int> graphics_width;
	static RKConfigValue<int> graphics_height;
	static RKConfigValue<int> graphics_jpg_quality;
	static RKConfigValue<QString> custom_css_file;
	static RKConfigValue<bool> shared_default_output;
};

#endif
