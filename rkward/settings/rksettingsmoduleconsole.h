/***************************************************************************
                          rksettingsmoduleconsole  -  description
                             -------------------
    begin                : Sun Oct 16 2005
    copyright            : (C) 2005, 2006, 2007, 2009 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RKSETTINGSMODULECONSOLE_H
#define RKSETTINGSMODULECONSOLE_H

#include "rksettingsmodule.h"

#include <qnamespace.h>

class QCheckBox;
class QComboBox;
class KIntSpinBox;

/**
Settings module for the console. Allows you to configure whether to store command history, command history length. Future extensions: color for warnings/errors, etc.

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleConsole : public RKSettingsModule {
Q_OBJECT
public:
	RKSettingsModuleConsole (RKSettings *gui, QWidget *parent);

	~RKSettingsModuleConsole ();
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);

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

	static QStringList loadCommandHistory ();
	static void saveCommandHistory (const QStringList &list);

	QString caption ();

	QString helpURL () { return ("rkward://page/rkward_console#settings"); };
public slots:
	void changedSetting (int);
private:
	static bool save_history;
	static uint max_history_length;
	static uint max_console_lines;
	static bool pipe_user_commands_through_console;
	static PipedCommandsHistoryMode add_piped_commands_to_history;
	static bool context_sensitive_history_by_default;

	QCheckBox *save_history_box;
	QCheckBox *reverse_context_mode_box;
	QCheckBox *pipe_user_commands_through_console_box;
	QComboBox *add_piped_commands_to_history_box;
	KIntSpinBox *max_history_length_spinner;
	KIntSpinBox *max_console_lines_spinner;
};

#endif
