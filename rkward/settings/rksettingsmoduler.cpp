/***************************************************************************
                          rksettingsmoduler  -  description
                             -------------------
    begin                : Wed Jul 28 2004
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
#include "rksettingsmoduler.h"

#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>

#include "../misc/getfilenamewidget.h"

// static members
QString RKSettingsModuleR::r_home_dir;
QString RKSettingsModuleR::pager_app;
QString RKSettingsModuleR::r_startup_file;
bool RKSettingsModuleR::r_nosave;
bool RKSettingsModuleR::r_slave;

RKSettingsModuleR::RKSettingsModuleR (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	QVBoxLayout *main_vbox = new QVBoxLayout (this, 6);
	
	QLabel *label = new QLabel (i18n ("Note: changes in this section do not take effect until you restart RKWard!"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	main_vbox->addStretch ();
	
	pager_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingFile, i18n ("Program to use as a pager (e.g. for R-help)"), "", pager_app);
	connect (pager_choser, SIGNAL (locationChanged ()), this, SLOT (pathChanged ()));
	main_vbox->addWidget (pager_choser);

	main_vbox->addStretch ();

	QVButtonGroup *group = new QVButtonGroup (i18n ("R options"), this);
	nosave_box = new QCheckBox ("--no-save", group);
	nosave_box->setChecked (r_nosave);
	connect (nosave_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	slave_box = new QCheckBox ("--slave", group);
	slave_box->setChecked (r_slave);
	connect (slave_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));

	main_vbox->addWidget (group);	

	main_vbox->addStretch ();
	startup_file_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingFile, i18n ("Location of 'startup.R'-script"), "", r_startup_file);
	connect (startup_file_choser, SIGNAL (locationChanged ()), this, SLOT (pathChanged ()));
	main_vbox->addWidget (startup_file_choser);
}

RKSettingsModuleR::~RKSettingsModuleR() {
}

void RKSettingsModuleR::boxChanged (int) {
	change ();
}

void RKSettingsModuleR::pathChanged () {
	change ();
}

QString RKSettingsModuleR::caption () {
	return (i18n ("R-Backend"));
}

bool RKSettingsModuleR::hasChanges () {
	return changed;
}

void RKSettingsModuleR::applyChanges () {
	pager_app = pager_choser->getLocation ();
	r_nosave = nosave_box->isChecked ();
	r_slave = slave_box->isChecked ();
	r_startup_file = startup_file_choser->getLocation ();
}

void RKSettingsModuleR::save (KConfig *config) {
	saveSettings (config);
}

void RKSettingsModuleR::saveSettings (KConfig *config) {
	config->setGroup ("R Settings");
	config->writeEntry ("R_HOME", r_home_dir);
	config->writeEntry ("pager app", pager_app);
	config->writeEntry ("startup file", r_startup_file);
	config->writeEntry ("--no-save", r_nosave);
	config->writeEntry ("--slave", r_slave);
}

void RKSettingsModuleR::loadSettings (KConfig *config) {
	config->setGroup ("R Settings");
	r_home_dir = config->readEntry ("R_HOME", "");
	pager_app = config->readEntry ("pager app", "xless");
	r_startup_file = config->readEntry ("startup file", KGlobal::dirs ()->findResourceDir ("data", "rkward/rfiles/startup.R") + "rkward/rfiles/");
	r_nosave = config->readBoolEntry ("--no-save", true);
	r_slave = config->readBoolEntry ("--slave", true);
}

// static
QStringList RKSettingsModuleR::getOptionList () {
	QStringList ret;
	if (r_nosave) {
		ret.append ("--no-save");
	}
	if (r_slave) {
		ret.append ("--slave");
	}
	return ret;
}

#include "rksettingsmoduler.moc"
