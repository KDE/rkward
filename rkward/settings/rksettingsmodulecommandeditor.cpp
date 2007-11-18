/***************************************************************************
                          rksettingsmodulecommandeditor  -  description
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
#include "rksettingsmodulecommandeditor.h"

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <qlayout.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>

#include "../misc/rkspinbox.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
int RKSettingsModuleCommandEditor::completion_min_chars;
int RKSettingsModuleCommandEditor::completion_timeout;
bool RKSettingsModuleCommandEditor::completion_enabled;

RKSettingsModuleCommandEditor::RKSettingsModuleCommandEditor (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout* main_vbox = new QVBoxLayout (this);

	QGroupBox* group = new QGroupBox (i18n ("Code Completion"), this);
	QVBoxLayout* box_layout = new QVBoxLayout (group);

	completion_enabled_box = new QCheckBox (i18n ("Enable code completion"), group);
	completion_enabled_box->setChecked (completion_enabled);
	connect (completion_enabled_box, SIGNAL (stateChanged(int)), this, SLOT (settingChanged(int)));
	box_layout->addWidget (completion_enabled_box);

	box_layout->addSpacing (RKGlobals::spacingHint ());

	QLabel* label = new QLabel (i18n ("Minimum number of characters before completion is attempted"), group);
	label->setWordWrap (true);
	completion_min_chars_box = new RKSpinBox (group);
	completion_min_chars_box->setIntMode (1, INT_MAX, completion_min_chars);
	completion_min_chars_box->setEnabled (completion_enabled);
	connect (completion_min_chars_box, SIGNAL (valueChanged(int)), this, SLOT (settingChanged(int)));
	box_layout->addWidget (label);
	box_layout->addWidget (completion_min_chars_box);

	main_vbox->addSpacing (RKGlobals::spacingHint ());

	label = new QLabel (i18n ("Timeout (milli seconds) before completion is attempted"), group);
	label->setWordWrap (true);
	completion_timeout_box = new RKSpinBox (group);
	completion_timeout_box->setIntMode (0, INT_MAX, completion_timeout);
	completion_timeout_box->setEnabled (completion_enabled);
	connect (completion_timeout_box, SIGNAL (valueChanged(int)), this, SLOT (settingChanged(int)));
	box_layout->addWidget (label);
	box_layout->addWidget (completion_timeout_box);

	main_vbox->addWidget (group);

	main_vbox->addStretch ();
}

RKSettingsModuleCommandEditor::~RKSettingsModuleCommandEditor () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleCommandEditor::settingChanged (int) {
	RK_TRACE (SETTINGS);
	change ();

	completion_timeout_box->setEnabled (completion_enabled_box->isChecked ());
	completion_min_chars_box->setEnabled (completion_enabled_box->isChecked ());
}

QString RKSettingsModuleCommandEditor::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Script editor"));
}

bool RKSettingsModuleCommandEditor::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleCommandEditor::applyChanges () {
	RK_TRACE (SETTINGS);

	completion_enabled = completion_enabled_box->isChecked ();
	completion_min_chars = completion_min_chars_box->intValue ();
	completion_timeout = completion_timeout_box->intValue ();
}

void RKSettingsModuleCommandEditor::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

void RKSettingsModuleCommandEditor::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Command Editor Windows");
	cg.writeEntry ("Completion enabled", completion_enabled);
	cg.writeEntry ("Completion min chars", completion_min_chars);
	cg.writeEntry ("Completion timeout", completion_timeout);
}

void RKSettingsModuleCommandEditor::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Command Editor Windows");
	completion_enabled = cg.readEntry ("Completion enabled", true);
	completion_min_chars = cg.readEntry ("Completion min chars", 2);
	completion_timeout = cg.readEntry ("Completion timeout", 500);
}

#include "rksettingsmodulecommandeditor.moc"
