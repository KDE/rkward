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
#include <kfiledialog.h>
#include <kconfig.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>

#include "rkward.h"

// static members
QString RKSettingsModuleR::r_home_dir;
bool RKSettingsModuleR::r_nosave;
bool RKSettingsModuleR::r_slave;

RKSettingsModuleR::RKSettingsModuleR (RKSettings *gui, RKwardApp *parent) : RKSettingsModule(gui, parent) {
	QVBoxLayout *main_vbox = new QVBoxLayout (this, 6);
	
	QLabel *label = new QLabel (i18n ("Note: changes in this panel do not have any effect, yet!"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	main_vbox->addStretch ();
	
	main_vbox->addWidget (new QLabel (i18n ("R_HOME-Directory"), this));
	
	QHBoxLayout *location_hbox = new QHBoxLayout (main_vbox, 6);
	location_edit = new QLineEdit (this);
	location_edit->setText (r_home_dir);
	connect (location_edit, SIGNAL (textChanged (const QString &)), this, SLOT (pathChanged(const QString&)));
	location_hbox->addWidget (location_edit);
	
	browse_button = new QPushButton (i18n ("Browse"), this);
	connect (browse_button, SIGNAL (clicked ()), this, SLOT (browse ()));
	location_hbox->addWidget (browse_button);

	main_vbox->addStretch ();

	QVButtonGroup *group = new QVButtonGroup (i18n ("R options"), this);
	nosave_box = new QCheckBox ("--no-save", group);
	nosave_box->setChecked (r_nosave);
	slave_box = new QCheckBox ("--slave", group);
	slave_box->setChecked (r_slave);

	main_vbox->addWidget (group);	
}

RKSettingsModuleR::~RKSettingsModuleR() {
}

void RKSettingsModuleR::browse () {
	QString temp = KFileDialog::getExistingDirectory (location_edit->text (), this, i18n ("Select R_HOME-directory"));
	if (temp != "") {
		location_edit->setText (temp);
	}
}

void RKSettingsModuleR::boxChanged (int) {
	change ();
}

void RKSettingsModuleR::pathChanged (const QString &) {
	change ();
}

QString RKSettingsModuleR::caption () {
	return (i18n ("R-Backend"));
}

bool RKSettingsModuleR::hasChanges () {
	return changed;
}

void RKSettingsModuleR::applyChanges () {
	r_home_dir = location_edit->text ();
	r_nosave = nosave_box->isChecked ();
	r_slave = slave_box->isChecked ();
}

void RKSettingsModuleR::save (KConfig *config) {
	saveSettings (config);
}

void RKSettingsModuleR::saveSettings (KConfig *config) {
	config->setGroup ("R Settings");
	config->writeEntry ("R_HOME", r_home_dir);
	config->writeEntry ("--no-save", r_nosave);
	config->writeEntry ("--slave", r_slave);
}

void RKSettingsModuleR::loadSettings (KConfig *config) {
	config->setGroup ("R Settings");
	r_nosave = config->readBoolEntry ("Option --no-save", true);
	r_slave = config->readBoolEntry ("Option --slave", true);
	r_home_dir = config->readEntry ("R_HOME", "/usr/lib/R");
}

#include "rksettingsmoduler.moc"
