/*
rksettingsmoduleconsole - This file is part of the RKWard project. Created: Sun Oct 16 2005
SPDX-FileCopyrightText: 2005-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULECONSOLE_H
#define RKSETTINGSMODULECONSOLE_H

#include "rksettingsmodule.h"

#include "rksettingsmodulecommandeditor.h"  // For RKCodeCompletionSettings
#include <qnamespace.h>

class QComboBox;
class QSpinBox;

/**
Settings module for the console. Allows you to configure whether to store command history, command history length. Future extensions: color for warnings/errors, etc.

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleConsole : public RKSettingsModule {
Q_OBJECT
public:
	RKSettingsModuleConsole (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleConsole ();

	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};
	void applyChanges () override;

	static bool saveHistory () { return save_history; };
	static uint maxHistoryLength () { return max_history_length; };
	static uint maxConsoleLines () { return max_console_lines; };
	static bool pipeUserCommandsThroughConsole () { return pipe_user_commands_through_console; };
	enum PipedCommandsHistoryMode {
		DontAdd = 0,
		AddSingleLine = 1,
		AlwaysAdd = 2
	};
	static PipedCommandsHistoryMode addPipedCommandsToHistory () { return add_piped_commands_to_history; };
	/** Given the button state, return whether the command history should be navigated context sensitive or insensitive
	@param current_state the current button state
	@returns true, if a the search should be context sensitive, false for a normal search */
	static bool shouldDoHistoryContextSensitive (Qt::KeyboardModifiers current_state);
	static RKConfigValue<bool> *showMinimap() { return &show_minimap; };
	static RKConfigValue<bool> *wordWrap() { return &word_wrap; };
	static const RKCodeCompletionSettings* completionSettings() { return &completion_settings; }

	static QStringList loadCommandHistory ();
	static void saveCommandHistory (const QStringList &list);

	QString caption() const override;
	QIcon icon() const override;

	QUrl helpURL () override { return QUrl ("rkward://page/rkward_console#settings"); };
private:
	static RKCodeCompletionSettings completion_settings;
	static RKConfigValue<bool> save_history;
	static RKConfigValue<uint> max_history_length;
	static RKConfigValue<uint> max_console_lines;
	static RKConfigValue<bool> pipe_user_commands_through_console;
	static RKConfigValue<PipedCommandsHistoryMode, int> add_piped_commands_to_history;
	static RKConfigValue<bool> context_sensitive_history_by_default;
	static RKConfigValue<bool> show_minimap;
	static RKConfigValue<bool> word_wrap;

	RKCodeCompletionSettingsWidget *completion_settings_widget;
};

#endif
