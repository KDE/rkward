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

#include <qlayout.h>
#include <qlabel.h>

#include "../misc/getfilenamewidget.h"
#include "../misc/rkcommonfunctions.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
QString RKSettingsModulePHP::php_bin;
QString RKSettingsModulePHP::files_path;

RKSettingsModulePHP::RKSettingsModulePHP (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this, RKGlobals::marginHint ());
	QLabel *label = new QLabel (i18n ("Changes in this section take effect the next time you start a plugin"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	
	main_vbox->addSpacing (2*RKGlobals::spacingHint ());
	
	bin_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingFile, i18n ("File-location of the PHP binary"), QString::null, php_bin);
	connect (bin_choser, SIGNAL (locationChanged ()), this, SLOT (pathChanged ()));
	main_vbox->addWidget (bin_choser);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	files_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingDirectory, i18n ("Directory, where the PHP support files are located"), QString::null, files_path);
	connect (files_choser, SIGNAL (locationChanged ()), this, SLOT (pathChanged ()));
	main_vbox->addWidget (files_choser);

	main_vbox->addStretch ();
}

RKSettingsModulePHP::~RKSettingsModulePHP () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModulePHP::pathChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModulePHP::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("PHP backend"));
}

bool RKSettingsModulePHP::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModulePHP::applyChanges () {
	RK_TRACE (SETTINGS);
	php_bin = bin_choser->getLocation ();
	files_path = files_choser->getLocation ();
}

void RKSettingsModulePHP::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

void RKSettingsModulePHP::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("PHP Settings");
	config->writeEntry ("PHP binary", php_bin);
	config->writeEntry ("support files dir", files_path);
}

void RKSettingsModulePHP::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("PHP Settings");
	php_bin = config->readEntry ("PHP binary", "/usr/bin/php");
	files_path = config->readEntry ("support files dir", RKCommonFunctions::getRKWardDataDir () + "phpfiles/");
}

#include "rksettingsmodulephp.moc"
