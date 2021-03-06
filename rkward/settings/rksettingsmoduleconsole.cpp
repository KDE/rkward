/***************************************************************************
                          rksettingsmoduleconsole  -  description
                             -------------------
    begin                : Sun Oct 16 2005
    copyright            : (C) 2005-2020 by Thomas Friedrichsmeier
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
#include <qcheckbox.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSpinBox>

#include "../rbackend/rcommand.h"
#include "../rkglobals.h"

#include "../debug.h"

// static

RKCodeCompletionSettings RKSettingsModuleConsole::completion_settings;
bool RKSettingsModuleConsole::save_history;
uint RKSettingsModuleConsole::max_history_length;
uint RKSettingsModuleConsole::max_console_lines;
bool RKSettingsModuleConsole::pipe_user_commands_through_console;
RKSettingsModuleConsole::PipedCommandsHistoryMode RKSettingsModuleConsole::add_piped_commands_to_history;
bool RKSettingsModuleConsole::context_sensitive_history_by_default;

RKSettingsModuleConsole::RKSettingsModuleConsole (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *vbox = new QVBoxLayout (this);

	vbox->addWidget (completion_settings_widget = new RKCodeCompletionSettingsWidget (this, this, &completion_settings, false));

	save_history_box = new QCheckBox (i18n ("Load/Save command history"), this);
	save_history_box->setChecked (save_history);
	connect (save_history_box, &QCheckBox::stateChanged, this, &RKSettingsModuleConsole::changedSetting);
	vbox->addWidget (save_history_box);

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

	pipe_user_commands_through_console_box = new QCheckBox (i18n ("Run commands from script editor through console"), this);
	pipe_user_commands_through_console_box->setChecked (pipe_user_commands_through_console);
	connect (pipe_user_commands_through_console_box, &QCheckBox::stateChanged, this, &RKSettingsModuleConsole::changedSetting);
	vbox->addWidget (pipe_user_commands_through_console_box);

	vbox->addWidget (new QLabel (i18n ("Also add those commands to console history"), this));
	add_piped_commands_to_history_box = new QComboBox (this);
	add_piped_commands_to_history_box->insertItem ((int) DontAdd, i18n ("Do not add"));
	add_piped_commands_to_history_box->insertItem ((int) AddSingleLine, i18n ("Add only if single line"));
	add_piped_commands_to_history_box->insertItem ((int) AlwaysAdd, i18n ("Add all commands"));
	add_piped_commands_to_history_box->setCurrentIndex ((int) add_piped_commands_to_history);
	connect (add_piped_commands_to_history_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RKSettingsModuleConsole::changedSetting);
	add_piped_commands_to_history_box->setEnabled (pipe_user_commands_through_console_box->isChecked ());
	vbox->addWidget (add_piped_commands_to_history_box);

	vbox->addSpacing (2*RKGlobals::spacingHint ());

	reverse_context_mode_box = new QCheckBox (i18n ("Command history is context sensitive by default"), this);
	connect (reverse_context_mode_box, &QCheckBox::stateChanged, this, &RKSettingsModuleConsole::changedSetting);
	reverse_context_mode_box->setChecked (context_sensitive_history_by_default);
	vbox->addWidget (reverse_context_mode_box);

	vbox->addStretch ();
}

RKSettingsModuleConsole::~RKSettingsModuleConsole () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleConsole::changedSetting (int) {
	RK_TRACE (SETTINGS);
	change ();

	add_piped_commands_to_history_box->setEnabled (pipe_user_commands_through_console_box->isChecked ());
}

//static
bool RKSettingsModuleConsole::shouldDoHistoryContextSensitive (Qt::KeyboardModifiers current_state) {
	RK_TRACE (SETTINGS);

	if (current_state & Qt::ShiftModifier) return (!context_sensitive_history_by_default);
	return context_sensitive_history_by_default;
}

//static
void RKSettingsModuleConsole::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Console Settings");
	cg.writeEntry ("save history", save_history);
	cg.writeEntry ("max history length", max_history_length);
	cg.writeEntry ("max console lines", max_console_lines);
	cg.writeEntry ("pipe user commands through console", pipe_user_commands_through_console);
	cg.writeEntry ("add piped commands to history", (int) add_piped_commands_to_history);
	cg.writeEntry ("command history defaults to context sensitive", context_sensitive_history_by_default);
	completion_settings.saveSettings(cg);
}

//static
void RKSettingsModuleConsole::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Console Settings");
	save_history = cg.readEntry ("save history", true);
	max_history_length = cg.readEntry ("max history length", 100);
	max_console_lines = cg.readEntry ("max console lines", 500);
	pipe_user_commands_through_console = cg.readEntry ("pipe user commands through console", true);
	add_piped_commands_to_history = (PipedCommandsHistoryMode) cg.readEntry ("add piped commands to history", (int) AddSingleLine);
	context_sensitive_history_by_default = cg.readEntry ("command history defaults to context sensitive", false);
	completion_settings.tabkey_invokes_completion = true;
	completion_settings.loadSettings(cg);
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
	save_history = save_history_box->isChecked ();
	max_history_length = max_history_length_spinner->value ();
	max_console_lines = max_console_lines_spinner->value ();
	pipe_user_commands_through_console = pipe_user_commands_through_console_box->isChecked ();
	add_piped_commands_to_history = (PipedCommandsHistoryMode) add_piped_commands_to_history_box->currentIndex ();
	context_sensitive_history_by_default = reverse_context_mode_box->isChecked ();
}

void RKSettingsModuleConsole::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}
	
QString RKSettingsModuleConsole::caption () {
	RK_TRACE (SETTINGS);

	return (i18n ("Console"));
}

