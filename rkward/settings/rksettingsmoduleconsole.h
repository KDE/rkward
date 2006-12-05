/***************************************************************************
                          rksettingsmoduleconsole  -  description
                             -------------------
    begin                : Sun Oct 16 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

class QCheckBox;
class KIntSpinBox;
class RKConsole;

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

	static QStringList loadCommandHistory ();
	static void saveCommandHistory (const QStringList &list);

	QString caption ();
public slots:
	void changedSetting (int);
private:
	static bool save_history;
	static uint max_history_length;
	static uint max_console_lines;
	static bool pipe_user_commands_through_console;

	QCheckBox *save_history_box;
	QCheckBox *pipe_user_commands_through_console_box;
	KIntSpinBox *max_history_length_spinner;
	KIntSpinBox *max_console_lines_spinner;
};

#endif
