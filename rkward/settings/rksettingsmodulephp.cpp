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
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qlayout.h>
#include <qlabel.h>

#include "../misc/getfilenamewidget.h"
#include "../rkglobals.h"

// static members
QString RKSettingsModulePHP::php_bin;
QString RKSettingsModulePHP::files_path;

RKSettingsModulePHP::RKSettingsModulePHP (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	QVBoxLayout *main_vbox = new QVBoxLayout (this, RKGlobals::marginHint ());
	QLabel *label = new QLabel (i18n ("Changes in this section take effect the next time you start a plugin"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	
	main_vbox->addStretch ();
	
	bin_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingFile, i18n ("File-location of the PHP binary"), "", php_bin);
	connect (bin_choser, SIGNAL (locationChanged ()), this, SLOT (pathChanged ()));
	main_vbox->addWidget (bin_choser);

	main_vbox->addStretch ();

	files_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingDirectory, i18n ("Directory, where the PHP support files are located"), "", files_path);
	connect (files_choser, SIGNAL (locationChanged ()), this, SLOT (pathChanged ()));
	main_vbox->addWidget (files_choser);
}

RKSettingsModulePHP::~RKSettingsModulePHP () {
}

void RKSettingsModulePHP::pathChanged () {
	change ();
}

QString RKSettingsModulePHP::caption () {
	return (i18n ("PHP backend"));
}

bool RKSettingsModulePHP::hasChanges () {
	return changed;
}

void RKSettingsModulePHP::applyChanges () {
	php_bin = bin_choser->getLocation ();
	files_path = files_choser->getLocation ();
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
