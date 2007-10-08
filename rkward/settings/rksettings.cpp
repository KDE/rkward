/***************************************************************************
                          rksettings  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
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
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3Frame>

#include <klocale.h>
#include <kapplication.h>
#include <kglobal.h>

#include "../windows/rkworkplace.h"

// modules
#include "rksettingsmoduleplugins.h"
#include "rksettingsmoduler.h"
#include "rksettingsmodulephp.h"
#include "rksettingsmodulegeneral.h"
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

RKSettings::RKSettings (QWidget *parent, const char *name) : KDialogBase (KDialogBase::Tabbed, i18n ("Settings"), KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel | KDialogBase::Help, KDialogBase::Ok, parent, name, false) {
	RK_TRACE (SETTINGS);
	setWFlags (getWFlags () | QWidget::WDestructiveClose);

	initModules ();

	connect (this, SIGNAL (aboutToShowPage (QWidget *)), this, SLOT (pageAboutToBeShown (QWidget *)));
}

RKSettings::~RKSettings() {
	RK_TRACE (SETTINGS);

	ModuleList::const_iterator it;
	for (it = modules.constBegin (); it != modules.constEnd (); ++it) {
		delete *it;
	}
	modules.clear ();
	
	dialogClosed ();
}

void RKSettings::initModules () {
	RK_TRACE (SETTINGS);

	modules.append (new RKSettingsModulePlugins (this, this));
	modules.append (new RKSettingsModuleR (this, this));
	modules.append (new RKSettingsModuleRPackages (this, this));
	modules.append (new RKSettingsModulePHP (this, this));
	modules.append (new RKSettingsModuleGeneral (this, this));
	modules.append (new RKSettingsModuleOutput (this, this));
	modules.append (new RKSettingsModuleWatch (this, this));
	modules.append (new RKSettingsModuleConsole (this, this));
	modules.append (new RKSettingsModuleObjectBrowser (this, this));
	
	ModuleList::const_iterator it;
	Q3Frame *page;
	Q3VBoxLayout *layout;
	for (it = modules.constBegin (); it != modules.constEnd (); ++it) {
		page = addPage ((*it)->caption ());
		layout = new Q3VBoxLayout (page, 0, KDialog::spacingHint ());
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

void RKSettings::pageAboutToBeShown (QWidget *page) {
	RK_TRACE (SETTINGS);

	// which module is it?
	RKSettingsModule *new_module = 0;
	for (ModuleList::const_iterator it = modules.constBegin (); it != modules.constEnd (); ++it) {
		QWidget *pwidget = *it;
		while (pwidget) {
			if (pwidget == page) {
				new_module = *it;
				break;
			}
			pwidget = pwidget->parentWidget ();
		}
		if (new_module) break;
	}

	bool has_help;
	if (!new_module) {
		RK_ASSERT (false);
		has_help = false;
	} else {
		has_help = !(new_module->helpURL ().isEmpty ());
	}
	enableButton (KDialogBase::Help, has_help);
}

void RKSettings::slotApply () {
	RK_TRACE (SETTINGS);

	ModuleList::const_iterator it;
	for (it = modules.constBegin (); it != modules.constEnd (); ++it) {
		if ((*it)->hasChanges ()) {
			(*it)->applyChanges ();
			(*it)->save (KGlobal::config ());
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

void RKSettings::slotHelp () {
	RK_TRACE (SETTINGS);

	// which page are we on?
	RKSettingsModule *current_module = modules[activePageIndex ()];
	RK_ASSERT (current_module);

	RKWorkplace::mainWorkplace ()->openHelpWindow (current_module->helpURL ());
}

void RKSettings::enableApply () {
	RK_TRACE (SETTINGS);
	enableButtonApply (true);
}

void RKSettings::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	RKSettingsModuleGeneral::loadSettings(config);		// alway load this first, as it contains the base path for rkward files
	RKSettingsModulePlugins::loadSettings(config);
	RKSettingsModuleR::loadSettings(config);
	RKSettingsModuleRPackages::loadSettings(config);
	RKSettingsModulePHP::loadSettings(config);
	RKSettingsModuleOutput::loadSettings(config);
	RKSettingsModuleWatch::loadSettings(config);
	RKSettingsModuleConsole::loadSettings(config);
	RKSettingsModuleObjectBrowser::loadSettings(config);
}

void RKSettings::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	RKSettingsModuleGeneral::saveSettings(config);
	RKSettingsModulePlugins::saveSettings(config);
	RKSettingsModuleR::saveSettings(config);
	RKSettingsModuleRPackages::saveSettings(config);
	RKSettingsModulePHP::saveSettings(config);
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
