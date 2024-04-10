/*
rksettingsmoduleoutput - This file is part of RKWard (https://rkward.kde.org). Created: Fri Jul 30 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmoduleoutput.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <qlayout.h>
#include <qlabel.h>
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QComboBox>

#include "../misc/rkstyle.h"
#include "../misc/getfilenamewidget.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkspinbox.h"
#include "../misc/rkstandardicons.h"
#include "../rbackend/rkrinterface.h"
#include "../debug.h"

// static members
RKConfigValue<bool> RKCarbonCopySettings::cc_globally_enabled {"CC enabled", false};
RKConfigValue<bool> RKCarbonCopySettings::cc_console_commands {"CC console commands", true};
RKConfigValue<bool> RKCarbonCopySettings::cc_script_commands {"CC script commands", false};
RKConfigValue<bool> RKCarbonCopySettings::cc_app_plugin_commands {"CC app/plugin commands", false};
RKConfigValue<bool> RKCarbonCopySettings::cc_command_output {"CC command output", true};
QList<RKCarbonCopySettings*> RKCarbonCopySettings::instances;

RKCarbonCopySettings::RKCarbonCopySettings(QWidget* parent, RKSettingsModule* module) : RKSettingsModuleWidget(parent, module) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	main_vbox->setContentsMargins (0, 0, 0, 0);
	cc_globally_enabled_box = new QGroupBox (i18n ("Carbon copy commands to output"), this);
	cc_globally_enabled_box->setCheckable (true);
	connect (cc_globally_enabled_box, &QGroupBox::clicked, this, &RKCarbonCopySettings::change);
	main_vbox->addWidget (cc_globally_enabled_box);

	QVBoxLayout *group_layout = new QVBoxLayout (cc_globally_enabled_box);
	group_layout->addWidget(cc_console_commands_box = cc_console_commands.makeCheckbox(i18n("Commands entered in the console"), this));
	group_layout->addWidget(cc_script_commands_box = cc_script_commands.makeCheckbox(i18n("Commands run via the 'Run' menu"), this));
	group_layout->addWidget(cc_app_plugin_commands_box = cc_app_plugin_commands.makeCheckbox(i18n("Commands originating from dialogs and plugins"), this));
	group_layout->addWidget(cc_command_output_box = cc_command_output.makeCheckbox(i18n("Also carbon copy the command output"), this));

	update ();
	instances.append (this);
}

RKCarbonCopySettings::~RKCarbonCopySettings () {
	RK_TRACE (SETTINGS);

	instances.removeAll (this);
}

void RKCarbonCopySettings::update() {
	RK_TRACE (SETTINGS);

	cc_globally_enabled_box->setChecked (cc_globally_enabled);
	cc_console_commands_box->setChecked (cc_console_commands);
	cc_script_commands_box->setChecked (cc_script_commands);
	cc_app_plugin_commands_box->setChecked (cc_app_plugin_commands);
	cc_command_output_box->setChecked (cc_command_output);
}

void RKCarbonCopySettings::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group("Carbon Copy Settings");
	cc_globally_enabled.syncConfig(cg, a);
	cc_console_commands.syncConfig(cg, a);
	cc_script_commands.syncConfig(cg, a);
	cc_app_plugin_commands.syncConfig(cg, a);
	cc_command_output.syncConfig(cg, a);
}

bool RKCarbonCopySettings::shouldCarbonCopyCommand (const RCommand *command) {
	RK_TRACE (SETTINGS);

	if (!cc_globally_enabled) return false;
	if (command->type () & (RCommand::Silent | RCommand::Sync)) return false;
	if (command->type () & RCommand::Console) return cc_console_commands;
	if (command->type () & RCommand::User) return cc_script_commands;
	if (command->type () & (RCommand::App | RCommand::Plugin)) return cc_app_plugin_commands;
	return false;
}

void RKCarbonCopySettings::applyChanges() {
	RK_TRACE (SETTINGS);

	cc_globally_enabled = cc_globally_enabled_box->isChecked ();

	for (RKCarbonCopySettings *sibling : std::as_const(instances)) {
		if (sibling != this) sibling->update ();
	}
}

// static members
RKConfigValue<bool> RKSettingsModuleOutput::auto_show {"auto_show", true};
RKConfigValue<bool> RKSettingsModuleOutput::auto_raise {"auto_raise", true};
RKConfigValue<QString> RKSettingsModuleOutput::graphics_type {"graphics_type", "NULL"};
RKConfigValue<int> RKSettingsModuleOutput::graphics_width {"graphics_width", 480};
RKConfigValue<int> RKSettingsModuleOutput::graphics_height {"graphics_height", 480};
RKConfigValue<int> RKSettingsModuleOutput::graphics_jpg_quality {"graphics_jpg_quality", 75};
RKConfigValue<QString> RKSettingsModuleOutput::custom_css_file {"custom css file", QString()};
RKConfigValue<bool> RKSettingsModuleOutput::shared_default_output {"Shared default output", false};

RKSettingsModuleOutput::RKSettingsModuleOutput (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	
	QGroupBox *group = new QGroupBox (i18n ("Output Window options"), this);
	QVBoxLayout* group_layout = new QVBoxLayout (group);
	auto auto_show_box = auto_show.makeCheckbox(i18n("show window on new output"), this);
	group_layout->addWidget (auto_show_box);
	auto auto_raise_box = auto_raise.makeCheckbox(i18n("raise window on new output"), this);
	group_layout->addWidget (auto_raise_box);
	auto_raise_box->setEnabled (auto_show);
	connect(auto_show_box, &QCheckBox::stateChanged, auto_raise_box, [auto_raise_box](int state) {
		auto_raise_box->setEnabled(state);
	});

	main_vbox->addWidget (group);

	main_vbox->addWidget(shared_default_output.makeCheckbox(i18n("Default output (used, while no other output has been set, explicitly) is shared across workspaces(*)."), this));

	custom_css_file_box = new GetFileNameWidget (this, GetFileNameWidget::ExistingFile, true, i18n ("CSS file to use for output (leave empty for default)"), i18n ("Select CSS file"), custom_css_file);
	connect (custom_css_file_box, &GetFileNameWidget::locationChanged, this, &RKSettingsModuleOutput::boxChanged);
	RKCommonFunctions::setTips (i18n ("Select a CSS file for custom formatting of the output window. Leave empty to use the default CSS file shipped with RKWard. Note that this setting takes effect, when initializing an output file (e.g. after flushing the output), only."), custom_css_file_box);
	main_vbox->addWidget (custom_css_file_box);

	group = new QGroupBox (i18n ("Graphics"), this);
	group_layout = new QVBoxLayout (group);
	QHBoxLayout *h_layout = new QHBoxLayout ();
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("File format"), group));
	h_layout->addWidget (graphics_type_box = new QComboBox (group));
	graphics_type_box->addItem (i18n ("<Default>"), QString ("NULL"));
	graphics_type_box->addItem (i18n ("PNG"), QString ("\"PNG\""));
	graphics_type_box->addItem (i18n ("SVG"), QString ("\"SVG\""));
	graphics_type_box->addItem (i18n ("JPG"), QString ("\"JPG\""));
	graphics_type_box->setCurrentIndex (graphics_type_box->findData (graphics_type.get()));
	graphics_type_box->setEditable (false);
	connect (graphics_type_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RKSettingsModuleOutput::boxChanged);
	h_layout->addSpacing (2*RKStyle::spacingHint ());
	h_layout->addWidget (new QLabel (i18n ("JPG quality"), group));
	h_layout->addWidget (graphics_jpg_quality_box = graphics_jpg_quality.makeSpinBox(1, 100, this));
	graphics_jpg_quality_box->setEnabled (graphics_type == "\"JPG\"");
	h_layout->addStretch ();

	h_layout = new QHBoxLayout ();
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("Width:"), group));
	h_layout->addWidget (graphics_width.makeSpinBox(1, INT_MAX, this));
	h_layout->addSpacing (2*RKStyle::spacingHint ());
	h_layout->addWidget (new QLabel (i18n ("Height:"), group));
	h_layout->addWidget (graphics_height.makeSpinBox(1, INT_MAX, this));
	h_layout->addStretch ();

	main_vbox->addWidget (group);

	cc_settings = new RKCarbonCopySettings(this, this);
	main_vbox->addWidget (cc_settings);

	main_vbox->addStretch ();
}

RKSettingsModuleOutput::~RKSettingsModuleOutput() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleOutput::boxChanged () {
	RK_TRACE (SETTINGS);
	change ();
	graphics_jpg_quality_box->setEnabled (graphics_type_box->itemData (graphics_type_box->currentIndex ()).toString () == "\"JPG\"");
}

QString RKSettingsModuleOutput::caption() const {
	RK_TRACE(SETTINGS);
	return(i18n("Output"));
}

QIcon RKSettingsModuleOutput::icon() const{
	RK_TRACE(SETTINGS);
	return RKStandardIcons::getIcon(RKStandardIcons::WindowOutput);
}

void RKSettingsModuleOutput::applyChanges () {
	RK_TRACE (SETTINGS);

	custom_css_file = custom_css_file_box->getLocation ();

	graphics_type = graphics_type_box->itemData (graphics_type_box->currentIndex ()).toString ();

	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.cbegin (); it != commands.cend (); ++it) {
		RInterface::issueCommand(new RCommand(*it, RCommand::App), commandChain());
	}

	cc_settings->applyChanges ();
}

void RKSettingsModuleOutput::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Output Window");
	auto_show.syncConfig(cg, a);
	auto_raise.syncConfig(cg, a);
	graphics_type.syncConfig(cg, a);
	graphics_width.syncConfig(cg, a);
	graphics_height.syncConfig(cg, a);
	graphics_jpg_quality.syncConfig(cg, a);
	custom_css_file.syncConfig(cg, a);
	shared_default_output.syncConfig(cg, a);

	RKCarbonCopySettings::syncConfig(config, a);
}

//static
QStringList RKSettingsModuleOutput::makeRRunTimeOptionCommands () {
	RK_TRACE (SETTINGS);
	QStringList list;

// output format options
	QString command = "options (\"rk.graphics.type\"=" + graphics_type.get();
	command.append (", \"rk.graphics.width\"=" + QString::number (graphics_width));
	command.append (", \"rk.graphics.height\"=" + QString::number (graphics_height));
	if (graphics_type == "\"JPG\"") command.append (", \"rk.graphics.jpg.quality\"=" + QString::number (graphics_jpg_quality));
	command.append (", \"rk.output.css.file\"=\"" + (custom_css_file.get().isEmpty () ? RKCommonFunctions::getRKWardDataDir () + "pages/rkward_output.css" : custom_css_file.get()) + '\"');
	list.append (command + ")\n");

	return (list);
}

