/***************************************************************************
                          rksettingsmoduleconsole  -  description
                             -------------------
    begin                : Sun Oct 16 2005
    copyright            : (C) 2005-2022 by Thomas Friedrichsmeier
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
#include "rksettingsmoduleconsole.h"

#include <KLocalizedString>
#include <kconfiggroup.h>
#include <KSharedConfig>

#include <qlayout.h>
#include <QCheckBox>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSpinBox>

#include "../rbackend/rcommand.h"
#include "../rkglobals.h"

#include "../debug.h"

// static

RKCodeCompletionSettings RKSettingsModuleConsole::completion_settings;
RKConfigValue<bool> RKSettingsModuleConsole::save_history {"save history", true};
RKConfigValue<uint> RKSettingsModuleConsole::max_history_length {"max history length", 100};
RKConfigValue<uint> RKSettingsModuleConsole::max_console_lines {"max console lines", 500};
RKConfigValue<bool> RKSettingsModuleConsole::pipe_user_commands_through_console {"pipe user commands through console", true};
RKConfigValue<RKSettingsModuleConsole::PipedCommandsHistoryMode, int> RKSettingsModuleConsole::add_piped_commands_to_history {"add piped commands to history", RKSettingsModuleConsole::AddSingleLine };
RKConfigValue<bool> RKSettingsModuleConsole::context_sensitive_history_by_default {"command history defaults to context sensitive", false};

RKSettingsModuleConsole::RKSettingsModuleConsole (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *vbox = new QVBoxLayout (this);

	vbox->addWidget (completion_settings_widget = new RKCodeCompletionSettingsWidget (this, this, &completion_settings, false));

	vbox->addWidget (save_history.makeCheckbox(i18n("Load/Save command history"), this));

	vbox->addWidget (new QLabel (i18n ("Maximum length of command history"), this));
	max_history_length_spinner = new QSpinBox(this);
	max_history_length_spinner->setMaximum(10000);
	max_history_length_spinner->setMinimum(0);
	max_history_length_spinner->setSingleStep(10);
	max_history_length_spinner->setValue(max_history_length);
	max_history_length_spinner->setSpecialValueText (i18n ("Unlimited"));
	connect (max_history_length_spinner, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleConsole::changedSetting);
	vbox->addWidget (max_history_length_spinner);

	vbox->addWidget (new QLabel (i18n ("Maximum number of paragraphs/lines to display in the console"), this));
	max_console_lines_spinner = new QSpinBox(this);
	max_console_lines_spinner->setMaximum(10000);
	max_console_lines_spinner->setMinimum(0);
	max_console_lines_spinner->setSingleStep(10);
	max_console_lines_spinner->setValue(max_console_lines);
	max_console_lines_spinner->setSpecialValueText (i18n ("Unlimited"));
	connect (max_console_lines_spinner, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleConsole::changedSetting);
	vbox->addWidget (max_console_lines_spinner);

	vbox->addSpacing (2*RKGlobals::spacingHint ());

	auto pipe_user_commands_through_console_box = pipe_user_commands_through_console.makeCheckbox(i18n("Run commands from script editor through console"), this);
	vbox->addWidget(pipe_user_commands_through_console_box);

	vbox->addWidget (new QLabel (i18n ("Also add those commands to console history"), this));
	add_piped_commands_to_history_box = new QComboBox (this);
	add_piped_commands_to_history_box->insertItem ((int) DontAdd, i18n ("Do not add"));
	add_piped_commands_to_history_box->insertItem ((int) AddSingleLine, i18n ("Add only if single line"));
	add_piped_commands_to_history_box->insertItem ((int) AlwaysAdd, i18n ("Add all commands"));
	add_piped_commands_to_history_box->setCurrentIndex ((int) add_piped_commands_to_history);
	connect (add_piped_commands_to_history_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RKSettingsModuleConsole::changedSetting);
	add_piped_commands_to_history_box->setEnabled(pipe_user_commands_through_console_box->isChecked());
	connect(pipe_user_commands_through_console_box, &QCheckBox::stateChanged, add_piped_commands_to_history_box, &QCheckBox::setEnabled);
	vbox->addWidget (add_piped_commands_to_history_box);

	vbox->addSpacing (2*RKGlobals::spacingHint ());

	vbox->addWidget (context_sensitive_history_by_default.makeCheckbox(i18n("Command history is context sensitive by default"), this));

	vbox->addStretch ();
}

RKSettingsModuleConsole::~RKSettingsModuleConsole () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleConsole::changedSetting (int) {
	RK_TRACE (SETTINGS);
	change ();
}

//static
bool RKSettingsModuleConsole::shouldDoHistoryContextSensitive (Qt::KeyboardModifiers current_state) {
	RK_TRACE (SETTINGS);

	if (current_state & Qt::ShiftModifier) return (!context_sensitive_history_by_default);
	return context_sensitive_history_by_default;
}

//static
void RKSettingsModuleConsole::syncConfig(KConfig* config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Console Settings");
	save_history.syncConfig(cg, a);
	max_history_length.syncConfig(cg, a);
	max_console_lines.syncConfig(cg, a);
	pipe_user_commands_through_console.syncConfig(cg, a);
	add_piped_commands_to_history.syncConfig(cg, a);
	context_sensitive_history_by_default.syncConfig(cg, a);
	if (a == RKConfigBase::LoadConfig) {
		completion_settings.tabkey_invokes_completion = true;
	}
	completion_settings.syncConfig(cg, a);
}

//static
QStringList RKSettingsModuleConsole::loadCommandHistory () {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = KSharedConfig::openConfig ()->group ("Console Settings");
	return cg.readEntry ("history", QStringList ());
}

//static
void RKSettingsModuleConsole::saveCommandHistory (const QStringList &list) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = KSharedConfig::openConfig ()->group ("Console Settings");
	if (save_history) {
		cg.writeEntry ("history", list);
	}
	cg.sync ();
}

void RKSettingsModuleConsole::applyChanges () {
	RK_TRACE (SETTINGS);

	completion_settings_widget->applyChanges();
	max_history_length = max_history_length_spinner->value ();
	max_console_lines = max_console_lines_spinner->value ();
	add_piped_commands_to_history = (PipedCommandsHistoryMode) add_piped_commands_to_history_box->currentIndex ();
}
	
QString RKSettingsModuleConsole::caption () {
	RK_TRACE (SETTINGS);

	return (i18n ("Console"));
}

