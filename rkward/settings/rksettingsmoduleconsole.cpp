/*
rksettingsmoduleconsole - This file is part of RKWard (https://rkward.kde.org). Created: Sun Oct 16 2005
SPDX-FileCopyrightText: 2005-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmoduleconsole.h"

#include <KLocalizedString>
#include <kconfiggroup.h>
#include <KSharedConfig>

#include <qlayout.h>
#include <QCheckBox>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QComboBox>

#include "../rbackend/rcommand.h"
#include "../misc/rkspinbox.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkstyle.h"

#include "../debug.h"

// static

RKCodeCompletionSettings RKSettingsModuleConsole::completion_settings;
RKConfigValue<bool> RKSettingsModuleConsole::save_history {"save history", true};
RKConfigValue<uint> RKSettingsModuleConsole::max_history_length {"max history length", 100};
RKConfigValue<uint> RKSettingsModuleConsole::max_console_lines {"max console lines", 500};
RKConfigValue<bool> RKSettingsModuleConsole::pipe_user_commands_through_console {"pipe user commands through console", true};
RKConfigValue<RKSettingsModuleConsole::PipedCommandsHistoryMode, int> RKSettingsModuleConsole::add_piped_commands_to_history {"add piped commands to history", RKSettingsModuleConsole::AddSingleLine };
RKConfigValue<bool> RKSettingsModuleConsole::context_sensitive_history_by_default {"command history defaults to context sensitive", false};
RKConfigValue<bool> RKSettingsModuleConsole::show_minimap {"show minimap", true};
RKConfigValue<bool> RKSettingsModuleConsole::word_wrap {"dynamic word wrap", false};

RKSettingsModuleConsole::RKSettingsModuleConsole (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *vbox = new QVBoxLayout (this);

	vbox->addWidget (completion_settings_widget = new RKCodeCompletionSettingsWidget (this, this, &completion_settings, false));

	vbox->addWidget (save_history.makeCheckbox(i18n("Load/Save command history"), this));

	vbox->addWidget (new QLabel (i18n ("Maximum length of command history (0 for no limit)"), this));
	auto max_history_length_spinner = max_history_length.makeSpinBox(0, 10000, this);
	vbox->addWidget (max_history_length_spinner);

	vbox->addWidget (new QLabel (i18n ("Maximum number of paragraphs/lines to display in the console (0 for not limit)"), this));
	auto max_console_lines_spinner = max_console_lines.makeSpinBox(0, 10000, this);
	vbox->addWidget (max_console_lines_spinner);

	vbox->addSpacing (2*RKStyle::spacingHint ());

	auto pipe_user_commands_through_console_box = pipe_user_commands_through_console.makeCheckbox(i18n("Run commands from script editor through console"), this);
	vbox->addWidget(pipe_user_commands_through_console_box);

	vbox->addWidget (new QLabel (i18n ("Also add those commands to console history"), this));
	auto add_piped_commands_to_history_box = add_piped_commands_to_history.makeDropDown(RKConfigBase::LabelList(
		{{(int) DontAdd, i18n("Do not add")}, {(int) AddSingleLine, i18n("Add only if single line")}, {(int) AlwaysAdd, i18n("Add all commands")}}
	), this);
	add_piped_commands_to_history_box->setEnabled(pipe_user_commands_through_console_box->isChecked());
	connect(pipe_user_commands_through_console_box, &QCheckBox::stateChanged, add_piped_commands_to_history_box, [add_piped_commands_to_history_box](int state) {
		add_piped_commands_to_history_box->setEnabled(state);
	});
	vbox->addWidget (add_piped_commands_to_history_box);

	vbox->addSpacing (2*RKStyle::spacingHint ());

	vbox->addWidget (context_sensitive_history_by_default.makeCheckbox(i18n("Command history is context sensitive by default"), this));

	vbox->addStretch ();
}

RKSettingsModuleConsole::~RKSettingsModuleConsole () {
	RK_TRACE (SETTINGS);
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
	show_minimap.syncConfig(cg, a);
	word_wrap.syncConfig(cg, a);
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
}
	
QString RKSettingsModuleConsole::caption() const {
	RK_TRACE(SETTINGS);

	return(i18n("Console"));
}

QIcon RKSettingsModuleConsole::icon() const {
	RK_TRACE(SETTINGS);

	return RKStandardIcons::getIcon(RKStandardIcons::WindowConsole);
}

