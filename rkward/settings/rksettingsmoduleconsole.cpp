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
void RKSettingsModuleConsole::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Console Settings");
	config->writeEntry ("save history", save_history);
	config->writeEntry ("max history length", max_history_length);
	config->writeEntry ("max console lines", max_console_lines);
}

//static
void RKSettingsModuleConsole::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Console Settings");
	save_history = config->readBoolEntry ("save history", true);
	max_history_length = config->readNumEntry ("max history length", 100);
	max_console_lines = config->readNumEntry ("max console lines", 500);
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
