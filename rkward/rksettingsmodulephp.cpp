/***************************************************************************
                          rksettingsmodulephp  -  description
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
#include "rksettingsmodulephp.h"

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
QString RKSettingsModulePHP::php_bin;
QString RKSettingsModulePHP::files_path;

RKSettingsModulePHP::RKSettingsModulePHP (RKSettings *gui, RKwardApp *parent) : RKSettingsModule(gui, parent) {
	QVBoxLayout *main_vbox = new QVBoxLayout (this, 6);
	QLabel *label = new QLabel (i18n ("Changes in this section take effect the next time you start a plugin"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	
	main_vbox->addStretch ();
	
	main_vbox->addWidget (new QLabel (i18n ("Location of the PHP binary"), this));
	
	QHBoxLayout *location_hbox = new QHBoxLayout (main_vbox, 6);
	bin_location_edit = new QLineEdit (this);
	bin_location_edit->setText (php_bin);
	connect (bin_location_edit, SIGNAL (textChanged (const QString &)), this, SLOT (pathChanged(const QString&)));
	location_hbox->addWidget (bin_location_edit);
	
	bin_browse_button = new QPushButton (i18n ("Browse"), this);
	connect (bin_browse_button, SIGNAL (clicked ()), this, SLOT (browseBin ()));
	location_hbox->addWidget (bin_browse_button);
	
	main_vbox->addStretch ();
	
	main_vbox->addWidget (new QLabel (i18n ("Location of the PHP support files"), this));
	
	location_hbox = new QHBoxLayout (main_vbox, 6);
	files_location_edit = new QLineEdit (this);
	files_location_edit->setText (files_path);
	connect (files_location_edit, SIGNAL (textChanged (const QString &)), this, SLOT (pathChanged(const QString&)));
	location_hbox->addWidget (files_location_edit);
	
	files_browse_button = new QPushButton (i18n ("Browse"), this);
	connect (files_browse_button, SIGNAL (clicked ()), this, SLOT (browseFiles ()));
	location_hbox->addWidget (files_browse_button);	
}

RKSettingsModulePHP::~RKSettingsModulePHP() {
}

void RKSettingsModulePHP::browseBin () {
	QString temp = KFileDialog::getOpenFileName (bin_location_edit->text (), QString::null, this, i18n ("Select Plugin-directory"));
	if (temp != "") {
		bin_location_edit->setText (temp);
	}
}

void RKSettingsModulePHP::browseFiles () {
	QString temp = KFileDialog::getExistingDirectory (files_location_edit->text (), this, i18n ("Select directory with support files (e.g. common.php)"));
	if (temp != "") {
		files_location_edit->setText (temp);
	}
}

void RKSettingsModulePHP::pathChanged (const QString &) {
	change ();
}

QString RKSettingsModulePHP::caption () {
	return (i18n ("PHP backend"));
}

bool RKSettingsModulePHP::hasChanges () {
	return changed;
}

void RKSettingsModulePHP::applyChanges () {
	php_bin = bin_location_edit->text ();
	files_path = files_location_edit->text ();
}

void RKSettingsModulePHP::save (KConfig *config) {
	saveSettings (config);
}

void RKSettingsModulePHP::saveSettings (KConfig *config) {
	config->setGroup ("PHP Settings");
	config->writeEntry ("PHP binary", php_bin);
	config->writeEntry ("support files dir", files_path);
}

void RKSettingsModulePHP::loadSettings (KConfig *config) {
	config->setGroup ("PHP Settings");
	php_bin = config->readEntry ("PHP binary", "/usr/bin/php");
	files_path = config->readEntry ("support files dir", KGlobal::dirs ()->findResourceDir ("data", "rkward/phpfiles/common.php") + "rkward/phpfiles/");
}

#include "rksettingsmodulephp.moc"
