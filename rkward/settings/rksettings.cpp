/*
rksettings - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettings.h"

#include <QPushButton>
#include <QVBoxLayout>

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
RKSettings *RKSettings::settings_dialog = nullptr;
RKSettingsTracker *RKSettings::settings_tracker = nullptr;

//static 
void RKSettings::configureSettings (SettingsPage page, QWidget *parent, RCommandChain *chain) {
	RK_TRACE (SETTINGS);

	RKSettingsModule::chain = chain;

	if (!settings_dialog) {
		settings_dialog = new RKSettings (parent);
	}

	if (page != NoPage) settings_dialog->raisePage (page);
	settings_dialog->show ();
	settings_dialog->raise ();
}

//static
void RKSettings::configureSettings (const QString& page, QWidget *parent, RCommandChain *chain) {
	RK_TRACE (SETTINGS);

	if (page == QStringLiteral("rbackend")) {
		RKSettings::configureSettings(RKSettings::PageR, parent, chain);
	} else if (page == QStringLiteral("console")) {
		RKSettings::configureSettings(RKSettings::PageConsole, parent, chain);
	} else if (page == QStringLiteral("editor")) {
		RKSettings::configureSettings(RKSettings::PageCommandEditor, parent, chain);
	} else if (page == QStringLiteral("graphics")) {
		RKSettings::configureSettings(RKSettings::PageX11, parent, chain);
	} else if (page == QStringLiteral("browser")) {
		RKSettings::configureSettings(RKSettings::PageObjectBrowser, parent, chain);
	} else if (page == QStringLiteral("rpackages")) {
		RKSettings::configureSettings(RKSettings::PageRPackages, parent, chain);
	} else if (page == QStringLiteral("output")) {
		RKSettings::configureSettings(RKSettings::PageOutput, parent, chain);
	} else if (page == QStringLiteral("general")) {
		RKSettings::configureSettings(RKSettings::PageGeneral, parent, chain);
	} else if (page == QStringLiteral("addons")) {
		RKSettings::configureSettings(RKSettings::SuperPageAddons, parent, chain);
	} else if (page == QStringLiteral("plugins")) {
		RKSettings::configureSettings(RKSettings::PagePlugins, parent, chain);
	} else if (page == QStringLiteral("kateplugins")) {
		RKSettings::configureSettings(RKSettings::PageKatePlugins, parent, chain);
	} else {
		RK_ASSERT(page.isEmpty());
		RKSettings::configureSettings(RKSettings::NoPage, parent, chain);
	}
}

//static
void RKSettings::dialogClosed () {
	RK_TRACE (SETTINGS);
	settings_dialog = nullptr;
}

RKSettings::RKSettings (QWidget *parent) : KPageDialog (parent) {
	RK_TRACE (SETTINGS);

	setFaceType (KPageDialog::Tree);
	setWindowTitle (i18n ("Settings"));
	buttonBox ()->setStandardButtons (QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
	button (QDialogButtonBox::Apply)->setEnabled (false);
	connect (button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &RKSettings::applyAll);
	connect (button(QDialogButtonBox::Help), &QPushButton::clicked, this, &RKSettings::helpClicked);

	setAttribute (Qt::WA_DeleteOnClose, true);

	initModules ();

	connect (this, &KPageDialog::currentPageChanged, this, &RKSettings::pageChange);
	setCurrentPage(pages[0]); // init -> see pageChange
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

void RKSettings::registerPageModule(RKSettings::SettingsPage super, int child) {
	RK_TRACE (SETTINGS);

	RKSettingsModule *childm = modules[child];
	if (super == NoPage) {
		pages[child] = addPage(childm, childm->caption());
	} else {
		pages[child] = addSubPage(pages[super], childm, childm->caption());
	}
	pages[child]->setHeader(childm->longCaption());
	pages[child]->setIcon(childm->icon());
}

#include <QLabel>

void RKSettings::initModules () {
	RK_TRACE (SETTINGS);

	auto ktexteditorpages = RKSettingsModuleCommandEditor::kateConfigPages(this, nullptr);
	pages.resize(NumPages + ktexteditorpages.size());
	modules.insert (PagePlugins, new RKSettingsModulePlugins(this, nullptr));
	modules.insert (PageKatePlugins, new RKSettingsModuleKatePlugins(this, nullptr));
	modules.insert (PageR, new RKSettingsModuleR(this, nullptr));
	modules.insert (PageRPackages, new RKSettingsModuleRPackages(this, nullptr));
	modules.insert (PageGeneral, new RKSettingsModuleGeneral(this, nullptr));
	modules.insert (PageOutput, new RKSettingsModuleOutput(this, nullptr));
	modules.insert (PageX11, new RKSettingsModuleGraphics(this, nullptr));
	modules.insert (PageWatch, new RKSettingsModuleWatch(this, nullptr));
	modules.insert (PageConsole, new RKSettingsModuleConsole(this, nullptr));
	modules.insert (PageCommandEditor, new RKSettingsModuleCommandEditor(this, nullptr));
	modules.insert (PageObjectBrowser, new RKSettingsModuleObjectBrowser(this, nullptr));
	modules.insert (PageDebug, new RKSettingsModuleDebug(this, nullptr));
	for (int i = 0; i < ktexteditorpages.size(); ++i) {
		modules.insert(NumPages+i, ktexteditorpages[i]);
	}

	QWidget *page = new QWidget();
	auto layout = new QVBoxLayout(page);
	QLabel *l = new QLabel(i18n("<h1>Add-ons</h1><p>RKWard add-ons come in a variety of forms, each with their own configuration options:</p><h2>R packages</h2><p><a href=\"rkward://settings/rpackages\">Add-ons to the R language itself</a>. These are usually downloaded from \"CRAN\". Some of these add-on packages may additionally contain RKWard plugins.</p><h2>RKWard plugins</h2><p><a href=\"rkward://settings/plugins\">Graphical dialogs to R functionality</a>. These plugins are usually pre-installed with RKWard, or with an R package. However, they can be activated/deactivated to help keep the menus manageable. Note that it is relatively easy to <a href=\"https://api.kde.org/doc/rkwardplugins/\">create your own custom dialogs as plugins</a>!</p><h2>Kate plugins</h2><p><a href=\"rkward://settings/kateplugins\">Plugins developed for Kate / KTextEditor</a>. These provide shared functionality that is useful in the context of text editing and IDE applications. These plugins are usually found pre-installed on your system. You can configure to load the plugins that are useful to your own workflow.</p>"));
	l->setWordWrap(true);
	connect(l, &QLabel::linkActivated, [=](const QString &url) { RKWorkplace::mainWorkplace()->openAnyUrl(QUrl(url)); });
	layout->addWidget(l);
	layout->addStretch();
	pages[SuperPageAddons] = addPage(page, i18n("Add-ons"));
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
	for (int i = 0; i < ktexteditorpages.size(); ++i) {
		registerPageModule(PageCommandEditor, NumPages+i);
	}
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

void RKSettings::applyAll() {
	RK_TRACE (SETTINGS);

	for (auto it = modules.constBegin(); it != modules.constEnd(); ++it) {
		if (it.value()->hasChanges()) {
			it.value()->doApply();
			it.value()->save(KSharedConfig::openConfig().data());
			tracker()->signalSettingsChange(RKSettings::SettingsPage(it.key()));
		}
	}
	button(QDialogButtonBox::Apply)->setEnabled(false);
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

	FOREACH_SETTINGS_MODULE(syncConfig(config, RKConfigBase::LoadConfig));
}

void RKSettings::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	FOREACH_SETTINGS_MODULE(syncConfig(config, RKConfigBase::SaveConfig));
}

QList<RKSetupWizardItem*> RKSettings::validateSettingsInteractive () {
	RK_TRACE (SETTINGS);

	QList<RKSetupWizardItem*> interaction_items;
	FOREACH_SETTINGS_MODULE(validateSettingsInteractive(&interaction_items));
	return interaction_items;
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
	Q_EMIT settingsChanged(page);
}

