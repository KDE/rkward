/***************************************************************************
                          rksettingsmoduleoutput  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004-2022 by Thomas Friedrichsmeier
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
	explicit RKCarbonCopySettings (QWidget* parent, RKSettingsModule* module);
	~RKCarbonCopySettings ();

	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);

	static bool shouldCarbonCopyCommand (const RCommand *command);
	static bool includeOutputInCarbonCopy () { return (cc_globally_enabled && cc_command_output); };
public slots:
	void applyChanges() override;
private:
	// There can be multiple instances of this widget, which need to be kept in sync.
	static QList<RKCarbonCopySettings*> instances;
	void update ();

	QGroupBox *cc_globally_enabled_box;
	QCheckBox *cc_console_commands_box;
	QCheckBox *cc_script_commands_box;
	QCheckBox *cc_app_plugin_commands_box;
	QCheckBox *cc_command_output_box;

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
	RKSettingsModuleOutput (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleOutput ();

	void applyChanges () override;
	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);

/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();

	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	QString caption () override;

	static bool autoShow () { return auto_show; };
	static bool autoRaise () { return auto_raise; };
	static bool sharedDefaultOutput() { return shared_default_output; };
public slots:
	void boxChanged ();
private:
	QComboBox *graphics_type_box;
	QSpinBox *graphics_width_box;
	QSpinBox *graphics_height_box;
	QSpinBox *graphics_jpg_quality_box;
	RKCarbonCopySettings *cc_settings;
	GetFileNameWidget *custom_css_file_box;

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
