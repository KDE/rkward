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
#include "rksettingsmoduleobjectbrowser.h"
#include "rksettingsmoduleconsole.h"

#include "../debug.h"

//static
RKSettings *RKSettings::settings_dialog = 0;
RKSettingsTracker *RKSettings::settings_tracker = 0;

//static 
void RKSettings::configureSettings (SettingsPage page, QWidget *parent, RCommandChain *chain) {
	RK_TRACE (SETTINGS);

	RKSettingsModule::chain = chain;

	if (!settings_dialog) {
		settings_dialog = new RKSettings (parent);
	}

	settings_dialog->show ();
	settings_dialog->raise ();
	settings_dialog->raisePage (page);
}

//static
void RKSettings::dialogClosed () {
	RK_TRACE (SETTINGS);
	settings_dialog = 0;
}

RKSettings::RKSettings (QWidget *parent, const char *name) : KDialogBase (KDialogBase::Tabbed, i18n ("Settings"), KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel, KDialogBase::Ok, parent, name, false) {
	RK_TRACE (SETTINGS);
	setWFlags (getWFlags () | QWidget::WDestructiveClose);

	initModules ();
}

RKSettings::~RKSettings() {
	RK_TRACE (SETTINGS);

	ModuleList::iterator it;
	for (it = modules.begin (); it != modules.end (); ++it) {
		delete *it;
	}
	modules.clear ();
	
	dialogClosed ();
}

void RKSettings::initModules () {
	RK_TRACE (SETTINGS);

	modules.append (new RKSettingsModulePlugins (this, this));
	modules.append (new RKSettingsModuleR (this, this));
	modules.append (new RKSettingsModulePHP (this, this));
	modules.append (new RKSettingsModuleLogfiles (this, this));
	modules.append (new RKSettingsModuleOutput (this, this));
	modules.append (new RKSettingsModuleWatch (this, this));
	modules.append (new RKSettingsModuleConsole (this, this));
	modules.append (new RKSettingsModuleObjectBrowser (this, this));
	
	ModuleList::iterator it;
	QFrame *page;
	QVBoxLayout *layout;
	for (it = modules.begin (); it != modules.end (); ++it) {
		page = addPage ((*it)->caption ());
		layout = new QVBoxLayout (page, 0, KDialog::spacingHint ());
// this is somewhat ugly, but works fine
		(*it)->reparent (page, QPoint (0,0), true);
		layout->addWidget (*it);
	}
}

void RKSettings::raisePage (SettingsPage page) {
	RK_TRACE (SETTINGS);

	if (page != NoPage) {
		showPage (((int) page) - 1);
	}
}

void RKSettings::slotApply () {
	RK_TRACE (SETTINGS);

	ModuleList::iterator it;
	for (it = modules.begin (); it != modules.end (); ++it) {
		if ((*it)->hasChanges ()) {
			(*it)->applyChanges ();
			(*it)->save (kapp->config ());
		}
	}
	enableButtonApply (false);
}

void RKSettings::slotOk () {
	RK_TRACE (SETTINGS);

	slotApply ();
	accept ();
	close ();
}

void RKSettings::slotCancel () {
	RK_TRACE (SETTINGS);
	QDialog::reject ();
}

void RKSettings::enableApply () {
	RK_TRACE (SETTINGS);
	enableButtonApply (true);
}

void RKSettings::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	RKSettingsModulePlugins::loadSettings(config);
	RKSettingsModuleR::loadSettings(config);
	RKSettingsModulePHP::loadSettings(config);
	RKSettingsModuleLogfiles::loadSettings(config);
	RKSettingsModuleOutput::loadSettings(config);
	RKSettingsModuleWatch::loadSettings(config);
	RKSettingsModuleConsole::loadSettings(config);
	RKSettingsModuleObjectBrowser::loadSettings(config);
}

void RKSettings::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	RKSettingsModulePlugins::saveSettings(config);
	RKSettingsModuleR::saveSettings(config);
	RKSettingsModulePHP::saveSettings(config);
	RKSettingsModuleLogfiles::saveSettings(config);
	RKSettingsModuleOutput::saveSettings(config);
	RKSettingsModuleWatch::saveSettings(config);
	RKSettingsModuleConsole::saveSettings(config);
	RKSettingsModuleObjectBrowser::saveSettings(config);
}

//############ END RKSettings ##################
//############ BEGIN RKSettingsTracker ############

RKSettingsTracker::RKSettingsTracker (QObject *parent) : QObject (parent) {
	RK_TRACE (SETTINGS);
}

RKSettingsTracker::~RKSettingsTracker () {
	RK_TRACE (SETTINGS);
}

void RKSettingsTracker::settingsChangedObjectBrowser () {
	RK_TRACE (SETTINGS);
	emit (objectBrowserSettingsChanged ());
}

#include "rksettings.moc"
