/***************************************************************************
                          rksettingsmoduledebug  description
                             -------------------
    begin                : Tue Oct 23 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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
#include "rksettingsmoduledebug.h"

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <qlayout.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QButtonGroup>

#include "../misc/rkspinbox.h"
#include "../rkglobals.h"
#include "../debug.h"

RKSettingsModuleDebug::RKSettingsModuleDebug (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout* main_vbox = new QVBoxLayout (this, RKGlobals::marginHint ());

	QLabel* label = new QLabel (i18n ("<b>These settings are for debugging purposes, only.</b> It is safe to leave the untouched. Also, these settings will only apply to the current session, and not be saved."), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);

	main_vbox->addSpacing (2 * RKGlobals::spacingHint ());

	label = new QLabel (i18n ("Debug level"), this);
	debug_level_box = new RKSpinBox (this);
	debug_level_box->setIntMode (DL_FATAL, DL_TRACE, DL_FATAL - RK_Debug_Level);
	connect (debug_level_box, SIGNAL (valueChanged(int)), this, SLOT (settingChanged(int)));
	main_vbox->addWidget (label);
	main_vbox->addWidget (debug_level_box);


	debug_flags_group = new QButtonGroup (this);
	debug_flags_group->setExclusive (false);
	QGroupBox* group = new QGroupBox (i18n ("Debug flags"), this);
	QVBoxLayout* box_layout = new QVBoxLayout (group);

	debug_flags_group->addButton (new QCheckBox ("APP", group), APP);
	debug_flags_group->addButton (new QCheckBox ("PLUGIN", group), PLUGIN);
	debug_flags_group->addButton (new QCheckBox ("OBJECTS", group), OBJECTS);
	debug_flags_group->addButton (new QCheckBox ("EDITOR", group), EDITOR);
	debug_flags_group->addButton (new QCheckBox ("SETTINGS", group), SETTINGS);
	debug_flags_group->addButton (new QCheckBox ("PHP", group), PHP);
	debug_flags_group->addButton (new QCheckBox ("RBACKEND", group), RBACKEND);
	debug_flags_group->addButton (new QCheckBox ("COMMANDEDITOR", group), COMMANDEDITOR);
	debug_flags_group->addButton (new QCheckBox ("MISC", group), MISC);
	debug_flags_group->addButton (new QCheckBox ("DIALOGS", group), DIALOGS);
	debug_flags_group->addButton (new QCheckBox ("OUTPUT", group), OUTPUT);
	debug_flags_group->addButton (new QCheckBox ("XML", group), XML);

	QList<QAbstractButton*> buttons = debug_flags_group->buttons ();
	for (QList<QAbstractButton*>::const_iterator it = buttons.constBegin (); it != buttons.constEnd (); ++it) {
		box_layout->addWidget (*it);
		(*it)->setChecked (RK_Debug_Flags & debug_flags_group->id (*it));
	}
	connect (debug_flags_group, SIGNAL (buttonClicked(int)), this, SLOT (settingChanged(int)));
	main_vbox->addWidget (group);


	label = new QLabel (i18n ("Command timeout"), this);
	command_timeout_box = new RKSpinBox (this);
	command_timeout_box->setIntMode (0, 10000, RK_Debug_CommandStep);
	connect (command_timeout_box, SIGNAL (valueChanged(int)), this, SLOT (settingChanged(int)));
	main_vbox->addWidget (label);
	main_vbox->addWidget (command_timeout_box);

	main_vbox->addStretch ();
}

RKSettingsModuleDebug::~RKSettingsModuleDebug () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleDebug::settingChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModuleDebug::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Debug"));
}

bool RKSettingsModuleDebug::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleDebug::applyChanges () {
	RK_TRACE (SETTINGS);

	RK_Debug_Level = DL_FATAL - debug_level_box->intValue ();
	RK_Debug_CommandStep = command_timeout_box->intValue ();
	int flags = 0;
	QList<QAbstractButton*> buttons = debug_flags_group->buttons ();
	for (QList<QAbstractButton*>::const_iterator it = buttons.constBegin (); it != buttons.constEnd (); ++it) {
		if ((*it)->isChecked ()) flags |= debug_flags_group->id (*it);
	}
	RK_Debug_Flags = flags;
}

void RKSettingsModuleDebug::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

void RKSettingsModuleDebug::saveSettings (KConfig *) {
	RK_TRACE (SETTINGS);

	// left empty on purpose
}

void RKSettingsModuleDebug::loadSettings (KConfig *) {
	RK_TRACE (SETTINGS);

	// left empty on purpose
}

#include "rksettingsmoduledebug.moc"
