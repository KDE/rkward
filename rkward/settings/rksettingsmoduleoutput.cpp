/***************************************************************************
                          rksettingsmoduleoutput  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004, 2010, 2011 by Thomas Friedrichsmeier
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
#include "rksettingsmoduleoutput.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <qlayout.h>
#include <qlabel.h>
#include <QGroupBox>
#include <qcheckbox.h>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSpinBox>

#include "../rkglobals.h"
#include "../misc/getfilenamewidget.h"
#include "../misc/rkcommonfunctions.h"
#include "../rbackend/rkrinterface.h"
#include "../debug.h"

// static members
bool RKCarbonCopySettings::cc_globally_enabled;
bool RKCarbonCopySettings::cc_console_commands;
bool RKCarbonCopySettings::cc_script_commands;
bool RKCarbonCopySettings::cc_app_plugin_commands;
bool RKCarbonCopySettings::cc_command_output;
QList<RKCarbonCopySettings*> RKCarbonCopySettings::instances;

RKCarbonCopySettings::RKCarbonCopySettings (QWidget* parent) : QWidget (parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	main_vbox->setContentsMargins (0, 0, 0, 0);
	cc_globally_enabled_box = new QGroupBox (i18n ("Carbon copy commands to output"), this);
	cc_globally_enabled_box->setCheckable (true);
	connect (cc_globally_enabled_box, &QGroupBox::clicked, this, &RKCarbonCopySettings::settingChanged);
	main_vbox->addWidget (cc_globally_enabled_box);

	QVBoxLayout *group_layout = new QVBoxLayout (cc_globally_enabled_box);
	cc_console_commands_box = new QCheckBox (i18n ("Commands entered in the console"), cc_globally_enabled_box);
	connect (cc_console_commands_box, &QCheckBox::clicked, this, &RKCarbonCopySettings::settingChanged);
	group_layout->addWidget (cc_console_commands_box);

	cc_script_commands_box = new QCheckBox (i18n ("Commands run via the 'Run' menu"), cc_globally_enabled_box);
	connect (cc_script_commands_box, &QCheckBox::clicked, this, &RKCarbonCopySettings::settingChanged);
	group_layout->addWidget (cc_script_commands_box);

	cc_app_plugin_commands_box = new QCheckBox (i18n ("Commands originating from dialogs and plugins"), cc_globally_enabled_box);
	connect (cc_app_plugin_commands_box, &QCheckBox::clicked, this, &RKCarbonCopySettings::settingChanged);
	group_layout->addWidget (cc_app_plugin_commands_box);

	cc_command_output_box = new QCheckBox (i18n ("Also carbon copy the command output"), cc_globally_enabled_box);
	connect (cc_command_output_box, &QCheckBox::clicked, this, &RKCarbonCopySettings::settingChanged);
	group_layout->addWidget (cc_command_output_box);

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

void RKCarbonCopySettings::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Carbon Copy Settings");
	cg.writeEntry ("CC enabled", cc_globally_enabled);
	cg.writeEntry ("CC console commands", cc_console_commands);
	cg.writeEntry ("CC script commands", cc_script_commands);
	cg.writeEntry ("CC app/plugin commands", cc_app_plugin_commands);
	cg.writeEntry ("CC command output", cc_command_output);
}

void RKCarbonCopySettings::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Carbon Copy Settings");
	cc_globally_enabled = cg.readEntry ("CC enabled", false);
	cc_console_commands = cg.readEntry ("CC console commands", true);
	cc_script_commands = cg.readEntry ("CC script commands", false);
	cc_app_plugin_commands = cg.readEntry ("CC app/plugin commands", false);
	cc_command_output = cg.readEntry ("CC command output", true);
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

void RKCarbonCopySettings::settingChanged () {
	RK_TRACE (SETTINGS);

	emit (changed ());
}

void RKCarbonCopySettings::applyChanges() {
	RK_TRACE (SETTINGS);

	cc_globally_enabled = cc_globally_enabled_box->isChecked ();
	cc_console_commands = cc_console_commands_box->isChecked ();
	cc_script_commands = cc_script_commands_box->isChecked ();
	cc_app_plugin_commands = cc_app_plugin_commands_box->isChecked ();
	cc_command_output = cc_command_output_box->isChecked ();

	foreach (RKCarbonCopySettings *sibling, instances) {
		if (sibling != this) sibling->update ();
	}
}

// static members
bool RKSettingsModuleOutput::auto_show;
bool RKSettingsModuleOutput::auto_raise;
QString RKSettingsModuleOutput::graphics_type;
int RKSettingsModuleOutput::graphics_width;
int RKSettingsModuleOutput::graphics_height;
int RKSettingsModuleOutput::graphics_jpg_quality;
QString RKSettingsModuleOutput::custom_css_file;

RKSettingsModuleOutput::RKSettingsModuleOutput (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	
	QGroupBox *group = new QGroupBox (i18n ("Output Window options"), this);
	QVBoxLayout* group_layout = new QVBoxLayout (group);
	group_layout->addWidget (auto_show_box = new QCheckBox (i18n ("show window on new output"), group));
	auto_show_box->setChecked (auto_show);
	connect (auto_show_box, &QCheckBox::stateChanged, this, &RKSettingsModuleOutput::boxChanged);
	group_layout->addWidget (auto_raise_box = new QCheckBox (i18n ("raise window on new output"), group));
	auto_raise_box->setChecked (auto_raise);
	auto_raise_box->setEnabled (auto_show);
	connect (auto_raise_box, &QCheckBox::stateChanged, this, &RKSettingsModuleOutput::boxChanged);

	main_vbox->addWidget (group);

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
	graphics_type_box->setCurrentIndex (graphics_type_box->findData (graphics_type));
	graphics_type_box->setEditable (false);
	connect (graphics_type_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RKSettingsModuleOutput::boxChanged);
	h_layout->addSpacing (2*RKGlobals::spacingHint ());
	h_layout->addWidget (new QLabel (i18n ("JPG quality"), group));
	h_layout->addWidget (graphics_jpg_quality_box = new QSpinBox(group));
	graphics_jpg_quality_box->setMaximum(100);
	graphics_jpg_quality_box->setMinimum(1);
	graphics_jpg_quality_box->setSingleStep(1);
	graphics_jpg_quality_box->setValue(graphics_jpg_quality);
	graphics_jpg_quality_box->setEnabled (graphics_type == "\"JPG\"");
	connect (graphics_jpg_quality_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleOutput::boxChanged);
	h_layout->addStretch ();

	h_layout = new QHBoxLayout ();
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("Width:"), group));
	h_layout->addWidget (graphics_width_box = new QSpinBox(group));
	graphics_width_box->setMaximum(INT_MAX);
	graphics_width_box->setMinimum(1);
	graphics_width_box->setSingleStep(1);
	graphics_width_box->setValue(graphics_width);
	h_layout->addSpacing (2*RKGlobals::spacingHint ());
	h_layout->addWidget (new QLabel (i18n ("Height:"), group));
	h_layout->addWidget (graphics_height_box = new QSpinBox(group));
	graphics_height_box->setMaximum(INT_MAX);
	graphics_height_box->setMinimum(1);
	graphics_height_box->setSingleStep(1);
	graphics_height_box->setValue(graphics_height);
	h_layout->addStretch ();
	connect (graphics_width_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleOutput::boxChanged);
	connect (graphics_height_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleOutput::boxChanged);

	main_vbox->addWidget (group);

	cc_settings = new RKCarbonCopySettings (this);
	connect (cc_settings, &RKCarbonCopySettings::changed, this, &RKSettingsModuleOutput::boxChanged);
	main_vbox->addWidget (cc_settings);

	main_vbox->addStretch ();
}

RKSettingsModuleOutput::~RKSettingsModuleOutput() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleOutput::boxChanged () {
	RK_TRACE (SETTINGS);
	change ();
	auto_raise_box->setEnabled (auto_show_box->isChecked ());
	graphics_jpg_quality_box->setEnabled (graphics_type_box->itemData (graphics_type_box->currentIndex ()).toString () == "\"JPG\"");
}

QString RKSettingsModuleOutput::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Output"));
}

void RKSettingsModuleOutput::applyChanges () {
	RK_TRACE (SETTINGS);

	auto_show = auto_show_box->isChecked ();
	auto_raise = auto_raise_box->isChecked ();

	custom_css_file = custom_css_file_box->getLocation ();

	graphics_type = graphics_type_box->itemData (graphics_type_box->currentIndex ()).toString ();
	graphics_width = graphics_width_box->value ();
	graphics_height = graphics_height_box->value ();
	graphics_jpg_quality = graphics_jpg_quality_box->value ();

	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString (), 0, 0, commandChain ());
	}

	cc_settings->applyChanges ();
}

void RKSettingsModuleOutput::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}

void RKSettingsModuleOutput::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Output Window");
	cg.writeEntry ("auto_show", auto_show);
	cg.writeEntry ("auto_raise", auto_raise);
	cg.writeEntry ("graphics_type", graphics_type);
	cg.writeEntry ("graphics_width", graphics_width);
	cg.writeEntry ("graphics_height", graphics_height);
	cg.writeEntry ("graphics_jpg_quality", graphics_jpg_quality);
	cg.writeEntry ("custom css file", custom_css_file);

	RKCarbonCopySettings::saveSettings (config);
}

void RKSettingsModuleOutput::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Output Window");
	auto_show = cg.readEntry ("auto_show", true);
	auto_raise = cg.readEntry ("auto_raise", true);
	graphics_type = cg.readEntry ("graphics_type", "NULL");
	graphics_width = cg.readEntry ("graphics_width", 480);
	graphics_height = cg.readEntry ("graphics_height", 480);
	graphics_jpg_quality = cg.readEntry ("graphics_jpg_quality", 75);
	custom_css_file = cg.readEntry ("custom css file", QString ());

	RKCarbonCopySettings::loadSettings (config);
}

//static
QStringList RKSettingsModuleOutput::makeRRunTimeOptionCommands () {
	RK_TRACE (SETTINGS);
	QStringList list;

// output format options
	QString command = "options (\"rk.graphics.type\"=" + graphics_type;
	command.append (", \"rk.graphics.width\"=" + QString::number (graphics_width));
	command.append (", \"rk.graphics.height\"=" + QString::number (graphics_height));
	if (graphics_type == "\"JPG\"") command.append (", \"rk.graphics.jpg.quality\"=" + QString::number (graphics_jpg_quality));
	command.append (", \"rk.output.css.file\"=\"" + (custom_css_file.isEmpty () ? RKCommonFunctions::getRKWardDataDir () + "pages/rkward_output.css" : custom_css_file) + '\"');
	list.append (command + ")\n");

	return (list);
}

