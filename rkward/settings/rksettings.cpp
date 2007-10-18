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
#include <kvbox.h>

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

RKSettings::RKSettings (QWidget *parent) : KPageDialog (parent) {
	RK_TRACE (SETTINGS);

	setFaceType (KPageDialog::Tabbed);
	setCaption (i18n ("Settings"));
	setButtons (KDialog::Ok | KDialog::Apply | KDialog::Cancel | KDialog::Help);
// KDE4: is this needed?	setModal (false);

	setAttribute (Qt::WA_DeleteOnClose, true);

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

	modules.insert (PagePlugins, new RKSettingsModulePlugins (this, this));
	modules.insert (PageR, new RKSettingsModuleR (this, this));
	modules.insert (PageRPackages, new RKSettingsModuleRPackages (this, this));
	modules.insert (PagePHP, new RKSettingsModulePHP (this, this));
	modules.insert (PageGeneral, new RKSettingsModuleGeneral (this, this));
	modules.insert (PageOutput, new RKSettingsModuleOutput (this, this));
	modules.insert (PageWatch, new RKSettingsModuleWatch (this, this));
	modules.insert (PageConsole, new RKSettingsModuleConsole (this, this));
	modules.insert (PageObjectBrowser, new RKSettingsModuleObjectBrowser (this, this));

	ModuleList::const_iterator it;
	for (it = modules.constBegin (); it != modules.constEnd (); ++it) {
		pages[it.key ()] = addPage (it.value (), it.value ()->caption ());
		it.value ()->show ();
	}
}

void RKSettings::raisePage (SettingsPage page) {
	RK_TRACE (SETTINGS);

	if (page != NoPage) {
		setCurrentPage (pages[(int) page - 1]);
	}
}

void RKSettings::pageAboutToBeShown (QWidget *page) {
	RK_TRACE (SETTINGS);

	// which module is it?
	RKSettingsModule *new_module = 0;
	for (ModuleList::const_iterator it = modules.constBegin (); it != modules.constEnd (); ++it) {
		if (it.value () == page) {
			new_module = it.value ();
			break;
		}
	}

	bool has_help;
	if (!new_module) {
		RK_ASSERT (false);
		has_help = false;
	} else {
		has_help = !(new_module->helpURL ().isEmpty ());
	}
	enableButton (KDialog::Help, has_help);
}

void RKSettings::slotButtonClicked (int button) {
	RK_TRACE (SETTINGS);

	if (button == KDialog::Apply) {
		applyAll ();
	} else if (button == KDialog::Ok) {
		applyAll ();
		accept ();
		close ();
	} else if (button == KDialog::Cancel) {
		reject ();
	} else if (button == KDialog::Help) {
		int i = 0;
		for (i = 0; i < NumPages; ++i) {
			if (currentPage () == pages[i]) break;
		}
		RKSettingsModule *current_module = modules[static_cast<SettingsPage> (i)];
		RK_ASSERT (current_module);
	
		RKWorkplace::mainWorkplace ()->openHelpWindow (current_module->helpURL ());
	} else {
		RK_ASSERT (false);
	}
}

void RKSettings::applyAll () {
	RK_TRACE (SETTINGS);

	ModuleList::const_iterator it;
	for (it = modules.constBegin (); it != modules.constEnd (); ++it) {
		if (it.value ()->hasChanges ()) {
			it.value ()->applyChanges ();
			it.value ()->save (KGlobal::config ().data ());
			tracker ()->signalSettingsChange (it.key ());
		}
	}
	enableButtonApply (false);
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

void RKSettingsTracker::signalSettingsChange (RKSettings::SettingsPage page) {
	RK_TRACE (SETTINGS);
	emit (settingsChanged (page));
}

#include "rksettings.moc"
