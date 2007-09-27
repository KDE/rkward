/***************************************************************************
                          rksettingsmoduleconsole  -  description
                             -------------------
    begin                : Sun Oct 16 2005
    copyright            : (C) 2005, 2006, 2007 by Thomas Friedrichsmeier
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
#include "rksettingsmoduleconsole.h"

#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kapplication.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include "../rbackend/rcommand.h"
#include "../rkglobals.h"

#include "../debug.h"

// static
bool RKSettingsModuleConsole::save_history;
uint RKSettingsModuleConsole::max_history_length;
uint RKSettingsModuleConsole::max_console_lines;
bool RKSettingsModuleConsole::pipe_user_commands_through_console;
bool RKSettingsModuleConsole::add_piped_commands_to_history;
bool RKSettingsModuleConsole::context_sensitive_history_by_default;

RKSettingsModuleConsole::RKSettingsModuleConsole (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::marginHint ());

	save_history_box = new QCheckBox (i18n ("Load/Save command history"), this);
	save_history_box->setChecked (save_history);
	connect (save_history_box, SIGNAL (stateChanged (int)), this, SLOT (changedSetting (int)));
	vbox->addWidget (save_history_box);

	vbox->addWidget (new QLabel (i18n ("Maximum length of command history"), this));
	max_history_length_spinner = new KIntSpinBox (0, 10000, 10, max_history_length, 10, this);
	max_history_length_spinner->setSpecialValueText (i18n ("Unlimited"));
	connect (max_history_length_spinner, SIGNAL (valueChanged (int)), this, SLOT (changedSetting (int)));
	vbox->addWidget (max_history_length_spinner);

	vbox->addWidget (new QLabel (i18n ("Maximum number of paragraphs/lines to display in the console"), this));
	max_console_lines_spinner = new KIntSpinBox (0, 10000, 10, max_console_lines, 10, this);
	max_console_lines_spinner->setSpecialValueText (i18n ("Unlimited"));
	connect (max_console_lines_spinner, SIGNAL (valueChanged (int)), this, SLOT (changedSetting (int)));
	vbox->addWidget (max_console_lines_spinner);

	vbox->addSpacing (2*RKGlobals::spacingHint ());

	pipe_user_commands_through_console_box = new QCheckBox (i18n ("Run commands from script editor through console"), this);
	pipe_user_commands_through_console_box->setChecked (pipe_user_commands_through_console);
	connect (pipe_user_commands_through_console_box, SIGNAL (stateChanged (int)), this, SLOT (changedSetting (int)));
	vbox->addWidget (pipe_user_commands_through_console_box);

	add_piped_commands_to_history_box = new QCheckBox (i18n ("Also add those commands to console history"), this);
	add_piped_commands_to_history_box->setChecked (add_piped_commands_to_history);
	connect (add_piped_commands_to_history_box, SIGNAL (stateChanged (int)), this, SLOT (changedSetting (int)));
	add_piped_commands_to_history_box->setEnabled (pipe_user_commands_through_console_box->isChecked ());
	vbox->addWidget (add_piped_commands_to_history_box);

	vbox->addSpacing (2*RKGlobals::spacingHint ());

	reverse_context_mode_box = new QCheckBox (i18n ("Command history is context sensitive by default"), this);
	connect (reverse_context_mode_box, SIGNAL (stateChanged (int)), this, SLOT (changedSetting (int)));
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
bool RKSettingsModuleConsole::shouldDoHistoryContextSensitive (Qt::ButtonState current_state) {
	RK_TRACE (SETTINGS);

	if (current_state & ShiftButton) return (!context_sensitive_history_by_default);
	return context_sensitive_history_by_default;
}

//static
void RKSettingsModuleConsole::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Console Settings");
	config->writeEntry ("save history", save_history);
	config->writeEntry ("max history length", max_history_length);
	config->writeEntry ("max console lines", max_console_lines);
	config->writeEntry ("pipe user commands through console", pipe_user_commands_through_console);
	config->writeEntry ("add piped commands to history", add_piped_commands_to_history);
	config->writeEntry ("command history defaults to context sensitive", context_sensitive_history_by_default);
}

//static
void RKSettingsModuleConsole::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Console Settings");
	save_history = config->readBoolEntry ("save history", true);
	max_history_length = config->readNumEntry ("max history length", 100);
	max_console_lines = config->readNumEntry ("max console lines", 500);
	pipe_user_commands_through_console = config->readBoolEntry ("pipe user commands through console", true);
	add_piped_commands_to_history = config->readBoolEntry ("add piped commands to history", true);
	context_sensitive_history_by_default = config->readBoolEntry ("command history defaults to context sensitive", false);
}

//static
QStringList RKSettingsModuleConsole::loadCommandHistory () {
	RK_TRACE (SETTINGS);

	KConfig *config = kapp->config ();
	config->setGroup ("Console Settings");
	return config->readListEntry ("history");
}

//static
void RKSettingsModuleConsole::saveCommandHistory (const QStringList &list) {
	RK_TRACE (SETTINGS);

	KConfig *config = kapp->config ();
	config->setGroup ("Console Settings");
	if (save_history) {
		config->writeEntry ("history", list);
	}
	config->sync ();
}


bool RKSettingsModuleConsole::hasChanges () {
// TODO: move to RKSettingsModule -baseclass?
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleConsole::applyChanges () {
	RK_TRACE (SETTINGS);

	save_history = save_history_box->isChecked ();
	max_history_length = max_history_length_spinner->value ();
	max_console_lines = max_console_lines_spinner->value ();
	pipe_user_commands_through_console = pipe_user_commands_through_console_box->isChecked ();
	add_piped_commands_to_history = add_piped_commands_to_history_box->isChecked ();
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

#include "rksettingsmoduleconsole.moc"
