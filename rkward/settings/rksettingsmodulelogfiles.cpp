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
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qdir.h>

#include "../misc/getfilenamewidget.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
QString RKSettingsModuleLogfiles::files_path;
QString RKSettingsModuleLogfiles::new_files_path;

RKSettingsModuleLogfiles::RKSettingsModuleLogfiles (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this, RKGlobals::marginHint ());
	QLabel *label = new QLabel (i18n ("Settings marked with (*) do not take effect until you restart RKWard"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	
	main_vbox->addStretch ();
	
	files_choser = new GetFileNameWidget (this, GetFileNameWidget::ExistingDirectory, i18n ("Directory where the logfiles should be kept (*)"), QString::null, new_files_path);
	connect (files_choser, SIGNAL (locationChanged ()), this, SLOT (pathChanged ()));
	main_vbox->addWidget (files_choser);
}

RKSettingsModuleLogfiles::~RKSettingsModuleLogfiles() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleLogfiles::pathChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModuleLogfiles::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Logfiles"));
}

bool RKSettingsModuleLogfiles::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleLogfiles::applyChanges () {
	RK_TRACE (SETTINGS);
	new_files_path = files_choser->getLocation ();
}

void RKSettingsModuleLogfiles::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

void RKSettingsModuleLogfiles::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Logfiles");
	config->writeEntry ("logfile dir", new_files_path);
}

void RKSettingsModuleLogfiles::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	config->setGroup ("Logfiles");
	files_path = new_files_path = config->readEntry ("logfile dir", QDir ().homeDirPath () + "/.rkward/");
}

#include "rksettingsmodulelogfiles.moc"
