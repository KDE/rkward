/***************************************************************************
                          rksettingsmoduleplugins  -  description
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
#include "rksettingsmoduleplugins.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "rkward.h"

// static members
QString RKSettingsModulePlugins::plugin_dir;

RKSettingsModulePlugins::RKSettingsModulePlugins (RKSettings *gui, RKwardApp *parent) : RKSettingsModule(gui, parent) {
	QVBoxLayout *main_vbox = new QVBoxLayout (this, 6);
	
	main_vbox->addStretch ();
	
	main_vbox->addWidget (new QLabel (i18n ("Directory where the plugins are located"), this));
	
	QHBoxLayout *location_hbox = new QHBoxLayout (main_vbox, 6);
	location_edit = new QLineEdit (this);
	location_edit->setText (plugin_dir);
	connect (location_edit, SIGNAL (textChanged (const QString &)), this, SLOT (pathChanged(const QString&)));
	location_hbox->addWidget (location_edit);
	
	browse_button = new QPushButton (i18n ("Browse"), this);
	connect (browse_button, SIGNAL (clicked ()), this, SLOT (browse ()));
	location_hbox->addWidget (browse_button);
}

RKSettingsModulePlugins::~RKSettingsModulePlugins() {
}

void RKSettingsModulePlugins::browse () {
	QString temp = KFileDialog::getExistingDirectory (location_edit->text (), this, i18n ("Select Plugin-directory"));
	if (temp != "") {
		location_edit->setText (temp);
	}
}


void RKSettingsModulePlugins::pathChanged (const QString &) {
	change ();
}

QString RKSettingsModulePlugins::caption () {
	return (i18n ("Plugins"));
}

bool RKSettingsModulePlugins::hasChanges () {
	return changed;
}

void RKSettingsModulePlugins::applyChanges () {
	plugin_dir = location_edit->text ();
	rk->initPlugins();
}

void RKSettingsModulePlugins::save (KConfig *config) {
	saveSettings (config);
}

void RKSettingsModulePlugins::saveSettings (KConfig *config) {
	config->setGroup ("Plugin Settings");
	config->writeEntry ("Plugin-Directory", plugin_dir);
}

void RKSettingsModulePlugins::loadSettings (KConfig *config) {
	config->setGroup ("Plugin Settings");
	plugin_dir = config->readEntry ("Plugin-Directory", "#unknown#");
	if (plugin_dir == "#unknown#") {
		plugin_dir = KGlobal::dirs()->findResourceDir("plugins", "description.xml");
		if (plugin_dir == "") {
			// try our luck with a relative path
			plugin_dir = "plugins/";
		}
	}
}

#include "rksettingsmoduleplugins.moc"
