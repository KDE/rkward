/***************************************************************************
                          rksettingsmodulelogfiles  -  description
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
#include "rksettingsmodulelogfiles.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qdir.h>

#include "rkward.h"

// static members
QString RKSettingsModuleLogfiles::files_path;

RKSettingsModuleLogfiles::RKSettingsModuleLogfiles (RKSettings *gui, RKwardApp *parent) : RKSettingsModule(gui, parent) {
	QVBoxLayout *main_vbox = new QVBoxLayout (this, 6);
	QLabel *label = new QLabel (i18n ("For now, when you change the setting for the location of the logfiles, RKWard will not function properly until you restart the application!"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	
	main_vbox->addStretch ();
	
	main_vbox->addWidget (new QLabel (i18n ("Directory where the logfiles should be kept"), this));
	
	QHBoxLayout *location_hbox = new QHBoxLayout (main_vbox, 6);
	files_location_edit = new QLineEdit (this);
	files_location_edit->setText (files_path);
	connect (files_location_edit, SIGNAL (textChanged (const QString &)), this, SLOT (pathChanged(const QString&)));
	location_hbox->addWidget (files_location_edit);
	
	files_browse_button = new QPushButton (i18n ("Browse"), this);
	connect (files_browse_button, SIGNAL (clicked ()), this, SLOT (browseFiles ()));
	location_hbox->addWidget (files_browse_button);	
}

RKSettingsModuleLogfiles::~RKSettingsModuleLogfiles() {
}

void RKSettingsModuleLogfiles::browseFiles () {
	QString temp = KFileDialog::getExistingDirectory (files_location_edit->text (), this, i18n ("Select directory with support files (e.g. common.php)"));
	if (temp != "") {
		files_location_edit->setText (temp);
	}
}

void RKSettingsModuleLogfiles::pathChanged (const QString &) {
	change ();
}

QString RKSettingsModuleLogfiles::caption () {
	return (i18n ("Logfiles"));
}

bool RKSettingsModuleLogfiles::hasChanges () {
	return changed;
}

void RKSettingsModuleLogfiles::applyChanges () {
	files_path = files_location_edit->text ();
}

void RKSettingsModuleLogfiles::save (KConfig *config) {
	saveSettings (config);
}

void RKSettingsModuleLogfiles::saveSettings (KConfig *config) {
	config->setGroup ("Logfiles");
	config->writeEntry ("logfile dir", files_path);
}

void RKSettingsModuleLogfiles::loadSettings (KConfig *config) {
	config->setGroup ("Logfiles");
	files_path = config->readEntry ("logfile dir", QDir ().homeDirPath () + "/.rkward/");
}

#include "rksettingsmodulelogfiles.moc"
