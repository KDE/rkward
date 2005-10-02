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
#include <kinputdialog.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>

#include "../misc/multistringselector.h"
#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
QString RKSettingsModuleR::r_home_dir;
bool RKSettingsModuleR::r_nosave;
bool RKSettingsModuleR::r_slave;
bool RKSettingsModuleR::archive_packages;
QStringList RKSettingsModuleR::package_repositories;

RKSettingsModuleR::RKSettingsModuleR (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this, RKGlobals::marginHint ());

	QLabel *label = new QLabel (i18n ("Note: Settings marked with (*) will not take effect until you restart RKWard!"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	main_vbox->addStretch ();

	repository_selector = new MultiStringSelector (i18n ("Package repositories"), this);
	repository_selector->setValues (package_repositories);
	connect (repository_selector, SIGNAL (listChanged ()), this, SLOT (pathChanged ()));
	connect (repository_selector, SIGNAL (getNewStrings (QStringList*)), this, SLOT (addRepository (QStringList*)));
	main_vbox->addWidget (repository_selector);

	archive_packages_box = new QCheckBox (i18n ("Archive downloaded packages"), this);
	archive_packages_box->setChecked (archive_packages);
	connect (archive_packages_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (archive_packages_box);	

	main_vbox->addStretch ();

	QVButtonGroup *group = new QVButtonGroup (i18n ("R options (*)"), this);
	nosave_box = new QCheckBox ("--no-save", group);
	nosave_box->setChecked (r_nosave);
	connect (nosave_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	slave_box = new QCheckBox ("--slave", group);
	slave_box->setChecked (r_slave);
	connect (slave_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));

	main_vbox->addWidget (group);	
}

RKSettingsModuleR::~RKSettingsModuleR() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleR::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleR::pathChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleR::addRepository (QStringList *string_list) {
	RK_TRACE (SETTINGS);
	QString new_string = KInputDialog::getText (i18n ("Add repository"), i18n ("Add URL of new repository\n(Enter \"@CRAN@\" for the standard CRAN-mirror)"), QString::null, 0, this);
	(*string_list).append (new_string);
}

QString RKSettingsModuleR::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("R-Backend"));
}

bool RKSettingsModuleR::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleR::applyChanges () {
	RK_TRACE (SETTINGS);

	r_nosave = nosave_box->isChecked ();
	r_slave = slave_box->isChecked ();
	archive_packages = archive_packages_box->isChecked ();

	package_repositories = repository_selector->getValues ();
	QString command = "options (repos=c(";
	for (QStringList::const_iterator it = package_repositories.begin (); it != package_repositories.end (); ++it) {
		if (it != package_repositories.begin ()) {
			command.append (", ");
		}
		command.append ("\"" + *it + "\"");
	}
	RKGlobals::rInterface ()->issueCommand (command + "))\n", RCommand::App, QString::null, 0, 0, commandChain ());
}

void RKSettingsModuleR::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}

void RKSettingsModuleR::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("R Settings");
	config->writeEntry ("R_HOME", r_home_dir);
	config->writeEntry ("--no-save", r_nosave);
	config->writeEntry ("--slave", r_slave);
	config->writeEntry ("archive packages", archive_packages);
	config->writeEntry ("Repositories", package_repositories);
}

void RKSettingsModuleR::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("R Settings");
	r_home_dir = config->readEntry ("R_HOME", QString::null);
	r_nosave = config->readBoolEntry ("--no-save", true);
	r_slave = config->readBoolEntry ("--slave", true);
	archive_packages = config->readBoolEntry ("archive packages", false);
	package_repositories = config->readListEntry ("Repositories");
	if (!package_repositories.count ()) {
		package_repositories.append ("@CRAN@");
	}
}

// static
QStringList RKSettingsModuleR::getOptionList () {
	RK_TRACE (SETTINGS);

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
