/***************************************************************************
                          rksettingsmodulewatch  -  description
                             -------------------
    begin                : Thu Aug 26 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rksettingsmodulewatch.h"

#include <klocale.h>
#include <kconfig.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include "../rbackend/rcommand.h"
#include "../rkglobals.h"

#include "../debug.h"

//static
int RKSettingsModuleWatch::plugin_filter;
//static
int RKSettingsModuleWatch::app_filter;
//static
int RKSettingsModuleWatch::sync_filter;
//static
int RKSettingsModuleWatch::user_filter;

//static
bool RKSettingsModuleWatch::shouldShowInput (RCommand *command) {
	RK_TRACE (SETTINGS);

	if (command->type () & RCommand::EmptyCommand) return false;
	
	if (command->type () & RCommand::Sync) {
		return (sync_filter & ShowInput);
	} else if (command->type () & RCommand::User) {
		return (user_filter & ShowInput);
	} else if (command->type () & RCommand::Plugin) {
		return (plugin_filter & ShowInput);
	} else if (command->type () & RCommand::App) {
		return (app_filter & ShowInput);
	}
	
	RK_ASSERT (false);
	return true;
}

//static
bool RKSettingsModuleWatch::shouldShowOutput (RCommand *command) {
	RK_TRACE (SETTINGS);

	if (command->type () & RCommand::EmptyCommand) return false;
	
	if (command->type () & RCommand::Sync) {
		return (sync_filter & ShowOutput);
	} else if (command->type () & RCommand::User) {
		return (user_filter & ShowOutput);
	} else if (command->type () & RCommand::Plugin) {
		return (plugin_filter & ShowOutput);
	} else if (command->type () & RCommand::App) {
		return (app_filter & ShowOutput);
	}
	
	RK_ASSERT (false);
	return true;
}

//static 
bool RKSettingsModuleWatch::shouldShowError (RCommand *command) {
	RK_TRACE (SETTINGS);

	if (command->type () & RCommand::Sync) {
		return (sync_filter & ShowError);
	} else if (command->type () & RCommand::User) {
		return (user_filter & ShowError);
	} else if (command->type () & RCommand::Plugin) {
		return (plugin_filter & ShowError);
	} else if (command->type () & RCommand::App) {
		return (app_filter & ShowError);
	}
	
	RK_ASSERT (false);
	return true;
}

bool RKSettingsModuleWatch::shouldRaiseWindow (RCommand *command) {
	RK_TRACE (SETTINGS);

	if (command->type () & RCommand::Sync) {
		return (sync_filter & RaiseWindow);
	} else if (command->type () & RCommand::User) {
		return (user_filter & RaiseWindow);
	} else if (command->type () & RCommand::Plugin) {
		return (plugin_filter & RaiseWindow);
	} else if (command->type () & RCommand::App) {
		return (app_filter & RaiseWindow);
	}
	
	RK_ASSERT (false);
	return true;
}

RKSettingsModuleWatch::RKSettingsModuleWatch (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::marginHint ());

	QLabel *label = new QLabel (i18n ("For now, settings only apply to new commands. All previous commands remain visible/invisible."), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	vbox->addWidget (label);
	vbox->addStretch ();
	
	QGridLayout *grid = new QGridLayout (vbox, 5, 5);

	label = new QLabel (i18n ("always show command"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	grid->addWidget (label, 0, 1);
	label = new QLabel (i18n ("always show result"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	grid->addWidget (label, 0, 2);
	label = new QLabel (i18n ("show errors"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	grid->addWidget (label, 0, 3);
	label = new QLabel (i18n ("show/raise window"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	grid->addWidget (label, 0, 4);
	
	user_filter_boxes = addFilterSettings (this, grid, 1, i18n ("User commands"), user_filter);
	plugin_filter_boxes = addFilterSettings (this, grid, 2, i18n ("Plugin generated commands"), plugin_filter);
	app_filter_boxes = addFilterSettings (this, grid, 3, i18n ("Application commands"), app_filter);
	sync_filter_boxes = addFilterSettings (this, grid, 4, i18n ("Synchronisation commands"), sync_filter);
}

RKSettingsModuleWatch::~RKSettingsModuleWatch () {
	RK_TRACE (SETTINGS);

	delete user_filter_boxes;
	delete plugin_filter_boxes;
	delete app_filter_boxes;
	delete sync_filter_boxes;
}

int RKSettingsModuleWatch::getFilterSettings (FilterBoxes *boxes) {
	RK_TRACE (SETTINGS);

	int ret=0;
	if (boxes->input->isChecked ()) ret |= ShowInput;
	if (boxes->output->isChecked ()) ret |= ShowOutput;
	if (boxes->error->isChecked ()) ret |= ShowError;
	if (boxes->raise->isChecked ()) ret |= RaiseWindow;
	return ret;
}

RKSettingsModuleWatch::FilterBoxes *RKSettingsModuleWatch::addFilterSettings (QWidget *parent, QGridLayout *layout, int row, const QString &label, int state) {
	RK_TRACE (SETTINGS);

	FilterBoxes *filter_boxes = new FilterBoxes;
	
	layout->addWidget (new QLabel (label, parent), row, 0);
	
	filter_boxes->input = new QCheckBox (parent);
	filter_boxes->input->setChecked (state & ShowInput);
	connect (filter_boxes->input, SIGNAL (stateChanged (int)), this, SLOT (changedSetting (int)));
	layout->addWidget (filter_boxes->input, row, 1);
	
	filter_boxes->output = new QCheckBox (parent);
	filter_boxes->output->setChecked (state & ShowOutput);
	connect (filter_boxes->output, SIGNAL (stateChanged (int)), this, SLOT (changedSetting (int)));
	layout->addWidget (filter_boxes->output, row, 2);
	
	filter_boxes->error = new QCheckBox (parent);
	filter_boxes->error->setChecked (state & ShowError);
	connect (filter_boxes->error, SIGNAL (stateChanged (int)), this, SLOT (changedSetting (int)));
	layout->addWidget (filter_boxes->error, row, 3);
	
	filter_boxes->raise = new QCheckBox (parent);
	filter_boxes->raise->setChecked (state & RaiseWindow);
	connect (filter_boxes->raise, SIGNAL (stateChanged (int)), this, SLOT (changedSetting (int)));
	layout->addWidget (filter_boxes->raise, row, 4);
	
	return filter_boxes;
}

void RKSettingsModuleWatch::changedSetting (int) {
	RK_TRACE (SETTINGS);
	change ();
}

//static
void RKSettingsModuleWatch::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("RInterface Watch Settings");
	config->writeEntry ("user command filter", user_filter);
	config->writeEntry ("plugin command filter", plugin_filter);
	config->writeEntry ("app command filter", app_filter);
	config->writeEntry ("sync command filter", sync_filter);
}

//static
void RKSettingsModuleWatch::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("RInterface Watch Settings");
	user_filter = config->readNumEntry ("user command filter", ShowInput | ShowOutput | ShowError | RaiseWindow);
	plugin_filter = config->readNumEntry ("plugin command filter", ShowInput | ShowError);
	app_filter = config->readNumEntry ("app command filter", ShowInput | ShowError);
	sync_filter = config->readNumEntry ("sync command filter", (int) ShowError);
}

bool RKSettingsModuleWatch::hasChanges () {
// TODO: move to RKSettingsModule -baseclass?
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleWatch::applyChanges () {
	RK_TRACE (SETTINGS);

	user_filter = getFilterSettings (user_filter_boxes);
	plugin_filter = getFilterSettings (plugin_filter_boxes);
	app_filter = getFilterSettings (app_filter_boxes);
	sync_filter = getFilterSettings (sync_filter_boxes);
}

void RKSettingsModuleWatch::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}
	
QString RKSettingsModuleWatch::caption () {
	RK_TRACE (SETTINGS);

	return (i18n ("Console"));
}

#include "rksettingsmodulewatch.moc"
