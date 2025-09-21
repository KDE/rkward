/*
rksettings - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettings.h"

#include <QPushButton>

#include <KLocalizedString>
#include <KSharedConfig>

#include "../rkward.h"
#include "../windows/rkworkplace.h"

// modules
#include "rksettingsmodulecommandeditor.h"
#include "rksettingsmoduleconsole.h"
#include "rksettingsmoduledebug.h"
#include "rksettingsmodulegeneral.h"
#include "rksettingsmodulegraphics.h"
#include "rksettingsmodulekateplugins.h"
#include "rksettingsmoduleobjectbrowser.h"
#include "rksettingsmoduleoutput.h"
#include "rksettingsmoduleplugins.h"
#include "rksettingsmoduler.h"
#include "rksettingsmodulewatch.h"

#include "../debug.h"

// static
RKSettings *RKSettings::settings_dialog = nullptr;
QList<RKSettingsModule *> RKSettings::modules;

// static
void RKSettings::configureSettings(const RKSettingsModule::PageId page, QWidget *parent, RCommandChain *chain) {
	RK_TRACE(SETTINGS);

	RKSettingsModule::chain = chain;

	if (!settings_dialog) {
		settings_dialog = new RKSettings(parent);
	}

	if (!page.isEmpty()) {
		settings_dialog->setCurrentPage(settings_dialog->findPage(page));
	}
	settings_dialog->show();
	settings_dialog->raise();
}

RKSettings::RKSettings(QWidget *parent) : KPageDialog(parent) {
	RK_TRACE(SETTINGS);
	RK_ASSERT(!settings_dialog);

	setFaceType(KPageDialog::Tree);
	setWindowTitle(i18n("Settings"));
	buttonBox()->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
	button(QDialogButtonBox::Apply)->setEnabled(false);
	connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &RKSettings::applyAll);
	connect(button(QDialogButtonBox::Help), &QPushButton::clicked, this, &RKSettings::helpClicked);

	setAttribute(Qt::WA_DeleteOnClose, true);

	initDialogPages();

	connect(this, &KPageDialog::currentPageChanged, this, &RKSettings::pageChange);
	setCurrentPage(pages[0]); // init -> see pageChange
}

RKSettings::~RKSettings() {
	RK_TRACE(SETTINGS);
	settings_dialog = nullptr;
}

void RKSettings::initDialogPages() {
	RK_TRACE(SETTINGS);
	RK_ASSERT(pages.isEmpty());

	for (auto it = modules.constBegin(); it != modules.constEnd(); ++it) {
		(*it)->createPages(this);
	}
}

void RKSettings::addSettingsPage(RKSettingsModuleWidget *widget) {
	RK_TRACE(SETTINGS);

	RK_ASSERT(widget->pageid != RKSettingsModule::no_page_id); // all toplevel pages shall have an id
	auto superp = widget->superpageid;
	KPageWidgetItem *page;
	if (superp.isEmpty()) {
		page = addPage(widget, widget->windowTitle());
	} else {
		auto superpage = findPage(superp);
		RK_ASSERT(superpage);
		page = addSubPage(superpage, widget, widget->windowTitle());
	}
	page->setHeader(widget->longCaption());
	page->setIcon(widget->windowIcon());
	connect(widget, &RKSettingsModuleWidget::settingsChanged, this, [this]() { button(QDialogButtonBox::Apply)->setEnabled(true); });
	pages.append(page);
}

KPageWidgetItem *RKSettings::findPage(const RKSettingsModule::PageId id) const {
	RK_TRACE(SETTINGS);
	auto it = std::find_if(pages.constBegin(), pages.constEnd(), [id](KPageWidgetItem *p) {
		return (static_cast<RKSettingsModuleWidget *>(p->widget())->pageid == id);
	});
	RK_ASSERT(it != pages.constEnd());
	return *it;
}

void RKSettings::pageChange(KPageWidgetItem *current, KPageWidgetItem *) {
	RK_TRACE(SETTINGS);
	RKSettingsModuleWidget *current_page = dynamic_cast<RKSettingsModuleWidget *>(current->widget());
	RK_ASSERT(current_page);

	bool has_help = current_page && !current_page->helpURL().isEmpty();
	button(QDialogButtonBox::Help)->setEnabled(has_help);
}

void RKSettings::done(int result) {
	RK_TRACE(SETTINGS);

	if (result == Accepted) applyAll();
	QDialog::done(result);
}

void RKSettings::helpClicked() {
	RK_TRACE(SETTINGS);

	RKSettingsModuleWidget *current_page = dynamic_cast<RKSettingsModuleWidget *>(currentPage()->widget());
	if (!current_page) {
		RK_ASSERT(false);
		return;
	}
	RKWorkplace::mainWorkplace()->openHelpWindow(current_page->helpURL());
}

void RKSettings::applyAll() {
	RK_TRACE(SETTINGS);

	// NOTE: This is shoddy design, but also kind of difficult: While applying the changes of the kate plugin page, pages may be added/removed
	//       to this very dialog. I.e. while looping over the pages, they'd get modified (and even deleted!). Can't have that, so for now, we
	//       special-case the kate plugins page, and handle it last.
	auto kate_plugin_page = findPage(RKSettingsModuleKatePlugins::page_id);
	RK_ASSERT(kate_plugin_page);
	QSet<RKSettingsModule *> changed_modules;
	for (auto it = pages.constBegin(); it != pages.constEnd(); ++it) {
		if ((*it) == kate_plugin_page) continue;
		auto w = static_cast<RKSettingsModuleWidget *>((*it)->widget());
		if (w->hasChanges()) {
			w->doApply();
			changed_modules.insert(w->parentModule());
		}
	}
	auto kateconfig = static_cast<RKSettingsModuleWidget *>(kate_plugin_page->widget());
	if (kateconfig->hasChanges()) {
		kateconfig->doApply();
		changed_modules.insert(kateconfig->parentModule());
	}

	for (auto mod : std::as_const(changed_modules)) {
		mod->syncConfig(KSharedConfig::openConfig().data(), RKConfigBase::SaveConfig);
		Q_EMIT mod->settingsChanged();
	}
	button(QDialogButtonBox::Apply)->setEnabled(false);
}

void RKSettings::removeSettingsPage(RKSettingsModuleWidget *which) {
	RK_TRACE(SETTINGS);

	auto it = std::find_if(pages.constBegin(), pages.constEnd(), [which](KPageWidgetItem *p) {
		return (p->widget() == which);
	});
	RK_ASSERT(it != pages.constEnd());
	if (it != pages.constEnd()) {
		auto pi = *it;
		pages.removeAll(pi);
		(pi)->deleteLater();
		KPageDialog::removePage(pi);
	}
}

void RKSettings::loadSettings(KConfig *config) {
	RK_TRACE(SETTINGS);

	if (modules.isEmpty()) {
		QObject *p = RKWardMainWindow::getMain();
		// NOTE: handle ModuleGeneral, first, as it contains paths used by other modules
		modules.append(new RKSettingsModuleGeneral(p));
		modules.append(new RKSettingsModulePlugins(p));
		modules.append(new RKSettingsModuleKatePlugins(p));
		modules.append(new RKSettingsModuleR(p));
		modules.append(new RKSettingsModuleRPackages(p));
		modules.append(new RKSettingsModuleOutput(p));
		modules.append(new RKSettingsModuleGraphics(p));
		modules.append(new RKSettingsModuleWatch(p));
		modules.append(new RKSettingsModuleConsole(p));
		modules.append(new RKSettingsModuleCommandEditor(p));
		modules.append(new RKSettingsModuleObjectBrowser(p));
		modules.append(new RKSettingsModuleDebug(p));
	}
	for (auto it = modules.constBegin(); it != modules.constEnd(); ++it) {
		(*it)->syncConfig(config, RKConfigBase::LoadConfig);
	}
}

void RKSettings::saveSettings(KConfig *config) {
	RK_TRACE(SETTINGS);
	RK_ASSERT(!modules.isEmpty());

	for (auto it = modules.constBegin(); it != modules.constEnd(); ++it) {
		(*it)->syncConfig(config, RKConfigBase::SaveConfig);
	}
}

QList<RKSetupWizardItem *> RKSettings::validateSettingsInteractive() {
	RK_TRACE(SETTINGS);

	QList<RKSetupWizardItem *> interaction_items;
	for (auto it = modules.constBegin(); it != modules.constEnd(); ++it) {
		(*it)->validateSettingsInteractive(&interaction_items);
	}
	return interaction_items;
}

//############ END RKSettings ##################
