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

#include <klocale.h>
#include <kapplication.h>

// modules
#include "rksettingsmoduleplugins.h"
#include "rksettingsmoduler.h"
#include "rksettingsmodulephp.h"
#include "rksettingsmodulelogfiles.h"
#include "rksettingsmoduleoutput.h"
#include "rksettingsmodulewatch.h"

#include "../rkward.h"
#include "../rkglobals.h"

//static
RKSettings *RKSettings::settings_dialog = 0;

//static 
void RKSettings::configureSettings (SettingsPage page, QWidget *parent) {
	if (!settings_dialog) {
		settings_dialog = new RKSettings (parent);
	}
	settings_dialog->show ();
	settings_dialog->raise ();
	settings_dialog->raisePage (page);
}

//static
void RKSettings::dialogClosed () {
	settings_dialog = 0;
}

RKSettings::RKSettings (QWidget *parent, const char *name) : KDialogBase (KDialogBase::Tabbed, i18n ("Settings"), KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel, KDialogBase::Ok, parent, name, false) {
	setWFlags (getWFlags () | QWidget::WDestructiveClose);

	initModules ();
}

RKSettings::~RKSettings() {
	ModuleList::iterator it;
	for (it = modules.begin (); it != modules.end (); ++it) {
		delete *it;
	}
	modules.clear ();
	
	dialogClosed ();
}

void RKSettings::initModules () {
	modules.append (new RKSettingsModulePlugins (this, this));
	modules.append (new RKSettingsModuleR (this, this));
	modules.append (new RKSettingsModulePHP (this, this));
	modules.append (new RKSettingsModuleLogfiles (this, this));
	modules.append (new RKSettingsModuleOutput (this, this));
	modules.append (new RKSettingsModuleWatch (this, this));
	
	ModuleList::iterator it;
	QFrame *page;
	QVBoxLayout *layout;
	for (it = modules.begin (); it != modules.end (); ++it) {
		page = addPage ((*it)->caption ());
		layout = new QVBoxLayout (page);
// this is somewhat ugly, but works fine
		(*it)->reparent (page, QPoint (0,0), true);
		layout->addWidget (*it);
	}
}

void RKSettings::raisePage (SettingsPage page) {
	if (page != NoPage) {
		showPage (((int) page) - 1);
	}
}

void RKSettings::slotApply () {
	ModuleList::iterator it;
	for (it = modules.begin (); it != modules.end (); ++it) {
		if ((*it)->hasChanges ()) {
			(*it)->applyChanges ();
			(*it)->save (RKGlobals::rkApp ()->config);
		}
	}
	enableButtonApply (false);
}

void RKSettings::slotOk () {
	slotApply ();
	accept ();
	close ();
}

void RKSettings::slotCancel () {
	QDialog::reject ();
}

void RKSettings::enableApply () {
	enableButtonApply (true);
}

void RKSettings::loadSettings (KConfig *config) {
	RKSettingsModulePlugins::loadSettings(config);
	RKSettingsModuleR::loadSettings(config);
	RKSettingsModulePHP::loadSettings(config);
	RKSettingsModuleLogfiles::loadSettings(config);
	RKSettingsModuleOutput::loadSettings(config);
	RKSettingsModuleWatch::loadSettings(config);
}

void RKSettings::saveSettings (KConfig *config) {
	RKSettingsModulePlugins::saveSettings(config);
	RKSettingsModuleR::saveSettings(config);
	RKSettingsModulePHP::saveSettings(config);
	RKSettingsModuleLogfiles::saveSettings(config);
	RKSettingsModuleOutput::saveSettings(config);
	RKSettingsModuleWatch::loadSettings(config);
}

#include "rksettings.moc"
