/***************************************************************************
                          rksettingsmoduleoutput  -  description
                             -------------------
    begin                : Fri Jul 30 2004
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
#include "rksettingsmoduleoutput.h"

#include <klocale.h>
#include <kconfig.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>

// static members
bool RKSettingsModuleOutput::auto_show;
bool RKSettingsModuleOutput::auto_raise;

RKSettingsModuleOutput::RKSettingsModuleOutput (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	QVBoxLayout *main_vbox = new QVBoxLayout (this, 6);
	
	QVButtonGroup *group = new QVButtonGroup (i18n ("Output Window options"), this);
	auto_show_box = new QCheckBox ("show window on new output", group);
	auto_show_box->setChecked (auto_show);
	connect (auto_show_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	auto_raise_box = new QCheckBox ("raise window on new output", group);
	auto_raise_box->setChecked (auto_raise);
	auto_raise_box->setEnabled (auto_show);
	connect (auto_raise_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));

	main_vbox->addWidget (group);	
}

RKSettingsModuleOutput::~RKSettingsModuleOutput() {
}

void RKSettingsModuleOutput::boxChanged (int) {
	change ();
	auto_raise_box->setEnabled (auto_show_box->isChecked ());
}

QString RKSettingsModuleOutput::caption () {
	return (i18n ("Output"));
}

bool RKSettingsModuleOutput::hasChanges () {
	return changed;
}

void RKSettingsModuleOutput::applyChanges () {
	auto_show = auto_show_box->isChecked ();
	auto_raise = auto_raise_box->isChecked ();
}

void RKSettingsModuleOutput::save (KConfig *config) {
	saveSettings (config);
}

void RKSettingsModuleOutput::saveSettings (KConfig *config) {
	config->setGroup ("Output Window");
	config->writeEntry ("auto_show", auto_show);
	config->writeEntry ("auto_raise", auto_raise);
}

void RKSettingsModuleOutput::loadSettings (KConfig *config) {
	config->setGroup ("Output Window");
	auto_show = config->readBoolEntry ("auto_show", true);
	auto_raise = config->readBoolEntry ("auto_raise", true);
}

#include "rksettingsmoduleoutput.moc"
