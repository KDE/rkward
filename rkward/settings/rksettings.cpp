/***************************************************************************
                          rksettings  -  description
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
#include "rksettings.h"

#include <qlayout.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kapplication.h>

// modules
#include "rksettingsmoduleplugins.h"
#include "rksettingsmoduler.h"
#include "rksettingsmodulephp.h"
#include "rksettingsmodulelogfiles.h"
#include "rksettingsmoduleoutput.h"

#include "../rkward.h"

RKSettings::RKSettings (RKwardApp *parent, const char *name) : QDialog (parent, name,false, QWidget::WDestructiveClose) {
	rk = parent;
	
	setCaption (i18n ("Settings"));
	
	QGridLayout *grid = new QGridLayout (this, 2, 1, 11, 6);
	tabs = new QTabWidget (this);
	grid->addWidget (tabs, 0, 0);
	
	QHBoxLayout *button_layout = new QHBoxLayout (0, 0, 6);
	grid->addLayout (button_layout, 1, 0);
	
	okbutton = new QPushButton (i18n ("Ok"), this);
	connect (okbutton, SIGNAL (clicked ()), this, SLOT (ok ()));
	applybutton = new QPushButton (i18n ("Apply"), this);
	applybutton->setEnabled (false);
	connect (applybutton, SIGNAL (clicked ()), this, SLOT (apply ()));
	cancelbutton = new QPushButton (i18n ("Cancel"), this);
	connect (cancelbutton, SIGNAL (clicked ()), this, SLOT (cancel ()));
	
	button_layout->addWidget (okbutton);
	button_layout->addWidget (applybutton);
	button_layout->addStretch ();
	button_layout->addWidget (cancelbutton);

	initModules ();
}

RKSettings::~RKSettings() {
	ModuleList::iterator it;
	for (it = modules.begin (); it != modules.end (); ++it) {
		delete *it;
	}
	modules.clear ();
}

void RKSettings::initModules () {
	modules.append (new RKSettingsModulePlugins (this, rk));
	modules.append (new RKSettingsModuleR (this, rk));
	modules.append (new RKSettingsModulePHP (this, rk));
	modules.append (new RKSettingsModuleLogfiles (this, rk));
	modules.append (new RKSettingsModuleOutput (this, rk));
	
	ModuleList::iterator it;
	for (it = modules.begin (); it != modules.end (); ++it) {
		tabs->addTab (*it, (*it)->caption ());
	}
}

void RKSettings::apply () {
	ModuleList::iterator it;
	for (it = modules.begin (); it != modules.end (); ++it) {
		if ((*it)->hasChanges ()) {
			(*it)->applyChanges ();
			(*it)->save (rk->config);
		}
	}
	applybutton->setEnabled (false);
}

void RKSettings::ok () {
	apply ();
	accept ();
	close ();
}

void RKSettings::cancel () {
	close ();
}

void RKSettings::enableApply () {
	applybutton->setEnabled (true);
}

void RKSettings::loadSettings (KConfig *config) {
	RKSettingsModulePlugins::loadSettings(config);
	RKSettingsModuleR::loadSettings(config);
	RKSettingsModulePHP::loadSettings(config);
	RKSettingsModuleLogfiles::loadSettings(config);
	RKSettingsModuleOutput::loadSettings(config);
}

void RKSettings::saveSettings (KConfig *config) {
	RKSettingsModulePlugins::saveSettings(config);
	RKSettingsModuleR::saveSettings(config);
	RKSettingsModulePHP::saveSettings(config);
	RKSettingsModuleLogfiles::saveSettings(config);
	RKSettingsModuleOutput::saveSettings(config);
}

#include "rksettings.moc"
