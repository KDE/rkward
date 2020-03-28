/***************************************************************************
                          rksettings  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004-2020 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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

#include <QPushButton>

#include <KLocalizedString>
#include <KSharedConfig>

#include "../windows/rkworkplace.h"

// modules
#include "rksettingsmoduleplugins.h"
#include "rksettingsmodulekateplugins.h"
#include "rksettingsmoduler.h"
#include "rksettingsmodulegeneral.h"
#include "rksettingsmoduleoutput.h"
#include "rksettingsmodulegraphics.h"
#include "rksettingsmodulewatch.h"
#include "rksettingsmoduleobjectbrowser.h"
#include "rksettingsmoduleconsole.h"
#include "rksettingsmodulecommandeditor.h"
#include "rksettingsmoduledebug.h"

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

	settings_dialog->raisePage (page);
	settings_dialog->show ();
	settings_dialog->raise ();
}

//static
void RKSettings::dialogClosed () {
	RK_TRACE (SETTINGS);
	settings_dialog = 0;
}

RKSettings::RKSettings (QWidget *parent) : KPageDialog (parent) {
	RK_TRACE (SETTINGS);

	setFaceType (KPageDialog::Tree);
	setWindowTitle (i18n ("Settings"));
	buttonBox ()->setStandardButtons (QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
	// KF5 TODO: connect buttons
	button (QDialogButtonBox::Apply)->setEnabled (false);
	connect (button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &RKSettings::applyAll);
	connect (button(QDialogButtonBox::Help), &QPushButton::clicked, this, &RKSettings::helpClicked);

	setAttribute (Qt::WA_DeleteOnClose, true);

	initModules ();

	connect (this, &KPageDialog::currentPageChanged, this, &RKSettings::pageChange);
	pageChange (currentPage (), currentPage ());	// init
}

RKSettings::~RKSettings() {
	RK_TRACE (SETTINGS);

	ModuleMap::const_iterator it;
	for (it = modules.constBegin (); it != modules.constEnd (); ++it) {
		delete *it;
	}
	modules.clear ();
	
	dialogClosed ();
}

void RKSettings::registerPageModule(RKSettings::SettingsPage super, RKSettings::SettingsPage child) {
	RK_TRACE (SETTINGS);

	RKSettingsModule *childm = modules[child];
	if (super == NoPage) {
		pages[child] = addPage(childm, childm->caption());
	} else {
		pages[child] = addSubPage(pages[super], childm, childm->caption());
	}
}

#include <QLabel>

void RKSettings::initModules () {
	RK_TRACE (SETTINGS);

	modules.insert (PagePlugins, new RKSettingsModulePlugins (this, 0));
	modules.insert (PageKatePlugins, new RKSettingsModuleKatePlugins (this, 0));
	modules.insert (PageR, new RKSettingsModuleR (this, 0));
	modules.insert (PageRPackages, new RKSettingsModuleRPackages (this, 0));
	modules.insert (PageGeneral, new RKSettingsModuleGeneral (this, 0));
	modules.insert (PageOutput, new RKSettingsModuleOutput (this, 0));
	modules.insert (PageX11, new RKSettingsModuleGraphics (this, 0));
	modules.insert (PageWatch, new RKSettingsModuleWatch (this, 0));
	modules.insert (PageConsole, new RKSettingsModuleConsole (this, 0));
	modules.insert (PageCommandEditor, new RKSettingsModuleCommandEditor (this, 0));
	modules.insert (PageObjectBrowser, new RKSettingsModuleObjectBrowser (this, 0));
	modules.insert (PageDebug, new RKSettingsModuleDebug (this, 0));

	QLabel *l = new QLabel(i18n("<h1>Add-ons</h1><p>RKWard add-ons come in a variety of forms, each with their own configuration options:</p><h2>R packages</h2><p><a href=\"rkward://settings/rpackages\">Add-ons to the R language itself</a>. These are usually downloaded from \"CRAN\". Some of these add-on packages may additionally contain RKWard plugins.</p><h2>RKWard plugins</h2><p><a href=\"rkward://settings/plugins\">Graphical dialogs to R functionality</a>. These plugins are usually pre-installed with RKWard, or with an R package. However, they can be activated/deactivated to help keep the menus manageable. Note that it is relatively easy to <a href=\"https://api.kde.org/doc/rkwardplugins/\">create your own custom dialogs as plugins</a>!</p><h2>Kate plugins</h2><p><a href=\"rkward://settings/kateplugins\">Plugins developed for Kate / KTextEditor</a>. These provide shared functionality that is useful in the context of text editing and IDE applications. These plugins are usually found pre-installed on your system. You can configure to load the plugins that are useful to your own workflow.</p>"));
	l->setWordWrap(true);
	connect(l, &QLabel::linkActivated, [=](const QString url) { RKWorkplace::mainWorkplace()->openAnyUrl(QUrl(url)); });
	pages[SuperPageAddons] = addPage(l, i18n("Add-ons"));
	registerPageModule(SuperPageAddons, PageRPackages);
	registerPageModule(SuperPageAddons, PagePlugins);
	registerPageModule(SuperPageAddons, PageKatePlugins);
	registerPageModule(NoPage, PageR);
	registerPageModule(NoPage, PageGeneral);
	registerPageModule(NoPage, PageOutput);
	registerPageModule(NoPage, PageX11);
	registerPageModule(NoPage, PageWatch);
	registerPageModule(NoPage, PageConsole);
	registerPageModule(NoPage, PageCommandEditor);
	registerPageModule(NoPage, PageObjectBrowser);
	registerPageModule(NoPage, PageDebug);
}

void RKSettings::raisePage (SettingsPage page) {
	RK_TRACE (SETTINGS);

	if (page != NoPage) {
		setCurrentPage (pages[(int) page]);
	}
}

void RKSettings::pageChange (KPageWidgetItem *current, KPageWidgetItem *) {
	RK_TRACE (SETTINGS);
	RKSettingsModule *new_module = dynamic_cast<RKSettingsModule*> (current->widget ());

	bool has_help;
	if (!new_module) {
		RK_ASSERT (false);
		has_help = false;
	} else {
		has_help = !(new_module->helpURL ().isEmpty ());
	}
	button (QDialogButtonBox::Help)->setEnabled (has_help);
}

void RKSettings::done (int result) {
	RK_TRACE (SETTINGS);

	if (result == Accepted) applyAll ();
	QDialog::done (result);
}

void RKSettings::helpClicked () {
	RK_TRACE (SETTINGS);

	RKSettingsModule *current_module = dynamic_cast<RKSettingsModule*> (currentPage ()->widget ());
	if (!current_module) {
		RK_ASSERT (false);
		return;
	}

	RKWorkplace::mainWorkplace ()->openHelpWindow (current_module->helpURL ());
}

void RKSettings::applyAll () {
	RK_TRACE (SETTINGS);

	ModuleMap::const_iterator it;
	for (it = modules.constBegin (); it != modules.constEnd (); ++it) {
		if (it.value ()->hasChanges ()) {
			it.value ()->applyChanges ();
			it.value ()->changed = false;
			it.value ()->save (KSharedConfig::openConfig ().data ());
			tracker ()->signalSettingsChange (it.key ());
		}
	}
	button (QDialogButtonBox::Apply)->setEnabled (false);
}

void RKSettings::enableApply () {
	RK_TRACE (SETTINGS);
	button (QDialogButtonBox::Apply)->setEnabled (true);
}

#define FOREACH_SETTINGS_MODULE(X)           \
	RKSettingsModuleGeneral::X;  /* always handle this first (esp., when loading settings), as it contains the base path for rkward files */ \
	RKSettingsModuleKatePlugins::X;          \
	RKSettingsModulePlugins::X;          \
	RKSettingsModuleR::X;                \
	RKSettingsModuleRPackages::X;        \
	RKSettingsModuleOutput::X;           \
	RKSettingsModuleGraphics::X;         \
	RKSettingsModuleWatch::X;            \
	RKSettingsModuleConsole::X;          \
	RKSettingsModuleCommandEditor::X;    \
	RKSettingsModuleObjectBrowser::X;

void RKSettings::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	FOREACH_SETTINGS_MODULE(loadSettings(config));
}

void RKSettings::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	FOREACH_SETTINGS_MODULE(saveSettings(config));
}

#include <KAssistantDialog>
void RKSettings::validateSettingsInteractive () {
	RK_TRACE (SETTINGS);

	QList<RKSettingsWizardPage*> interaction_pages;
	FOREACH_SETTINGS_MODULE(validateSettingsInteractive(&interaction_pages));
	if (!interaction_pages.isEmpty ()) {
		KAssistantDialog dialog ((QWidget*) 0);
		for (int i = 0; i < interaction_pages.size (); ++i) {
			dialog.addPage (interaction_pages[i], interaction_pages[i]->windowTitle ());
		}
		QPushButton *help_button = dialog.button (QDialogButtonBox::Help);
		if (help_button) help_button->hide ();
		if (dialog.exec () == QDialog::Accepted) {
			for (int i = 0; i < interaction_pages.size (); ++i) {
				interaction_pages[i]->apply ();
			}
		}
	}
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

