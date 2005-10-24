/***************************************************************************
                          rksettingsmodule  -  description
                             -------------------
    begin                : Fri Apr 22 2005
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

#include "rksettingsmoduleobjectbrowser.h"

#include <klocale.h>
#include <kconfig.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include "../rkglobals.h"
#include "rksettings.h"
#include "../debug.h"

// static
int RKSettingsModuleObjectBrowser::options = 0;

RKSettingsModuleObjectBrowser::RKSettingsModuleObjectBrowser (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *layout = new QVBoxLayout (this, RKGlobals::marginHint ());

	show_hidden_vars_box = new QCheckBox (i18n ("Show hidden objects"), this);
	show_hidden_vars_box->setChecked (showHiddenVars ());
	connect (show_hidden_vars_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	layout->addWidget (show_hidden_vars_box);

	layout->addSpacing (2*RKGlobals::spacingHint ());

	QLabel *label = new QLabel (i18n ("Which columns should be shown?"), this);
	layout->addWidget (label);

	show_label_field_box = new QCheckBox (i18n ("Label field"), this);
	show_label_field_box->setChecked (showLabelField ());
	connect (show_label_field_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	layout->addWidget (show_label_field_box);

	show_type_field_box = new QCheckBox (i18n ("Type field"), this);
	show_type_field_box->setChecked (showTypeField ());
	connect (show_type_field_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	layout->addWidget (show_type_field_box);

	show_class_field_box = new QCheckBox (i18n ("Class field"), this);
	show_class_field_box->setChecked (showClassField ());
	connect (show_class_field_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	layout->addWidget (show_class_field_box);

	layout->addStretch ();
}

RKSettingsModuleObjectBrowser::~RKSettingsModuleObjectBrowser () {
	RK_TRACE (SETTINGS);
}

bool RKSettingsModuleObjectBrowser::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleObjectBrowser::applyChanges () {
	RK_TRACE (SETTINGS);
	options = 0;
	if (show_hidden_vars_box->isChecked ()) options |= ShowHiddenVars;
	if (show_label_field_box->isChecked ()) options |= ShowLabelField;
	if (show_type_field_box->isChecked ()) options |= ShowTypeField;
	if (show_class_field_box->isChecked ()) options |= ShowClassField;

	RKSettings::tracker ()->settingsChangedObjectBrowser ();
}

void RKSettingsModuleObjectBrowser::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

QString RKSettingsModuleObjectBrowser::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Workspace"));
}

//static
void RKSettingsModuleObjectBrowser::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Object Browser");
	config->writeEntry ("show hidden vars", showHiddenVars ());
	config->writeEntry ("show label field", showLabelField ());
	config->writeEntry ("show type field", showTypeField ());
	config->writeEntry ("show class field", showClassField ());
}

//static
void RKSettingsModuleObjectBrowser::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	options = 0;
	config->setGroup ("Object Browser");
	if (config->readBoolEntry ("show hidden vars", false)) options |= ShowHiddenVars;
	if (config->readBoolEntry ("show label field", true)) options |= ShowLabelField;
	if (config->readBoolEntry ("show type field", true)) options |= ShowTypeField;
	if (config->readBoolEntry ("show class field", true)) options |= ShowClassField;
}

void RKSettingsModuleObjectBrowser::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

#include "rksettingsmoduleobjectbrowser.moc"
