/***************************************************************************
                          rksetupwizard  -  description
                             -------------------
    begin                : Fri May 25 20200
    copyright            : (C) 2020 by Thomas Friedrichsmeier
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

#include "rksetupwizard.h"

#include <functional>

#include <QLabel>
#include <QApplication>
#include <QComboBox>
#include <QGridLayout>
#include <QIcon>
#include <QPushButton>

#include <KLocalizedString>
#include <KMessageBox>

#include "../settings/rksettingsmoduleplugins.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardicons.h"
#include "../dialogs/rkloadlibsdialog.h"
#include "../windows/katepluginintegration.h"
#include "../rbackend/rksessionvars.h"
#include "../rbackend/rkrinterface.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

bool RKSetupWizard::has_been_run = false;

class RKSetupWizardItem {
public:
	enum Status {
		Error,
		Warning,
		Good
	};
	RKSetupWizardItem(const QString &shortlabel, const QString &longlabel=QString(), Status status=Good, const QString &shortstatuslabel=QString()) : status(status), shortlabel(shortlabel), longlabel(longlabel), shortstatuslabel(shortstatuslabel), box(nullptr) {};
	~RKSetupWizardItem() {};
	void addOption(const QString &shortlabel, const QString &longlabel, std::function<void(RKSetupWizard*)> callback) {
		options.append(Option(shortlabel, longlabel, callback));
	}
	void setStatus(Status _status, const QString &_shortstatuslabel) { status = _status; shortstatuslabel = _shortstatuslabel; };
	void setShortLabel(const QString &label) { shortlabel = label; };
	void setLongLabel(const QString &label) { longlabel = label; };
private:
friend class RKSetupWizard;
	void createWidget(QGridLayout *layout, int row) {
		QString icon_id;
		if (status == Good) icon_id = QLatin1String("dialog-positive");
		else if (status == Warning) icon_id = QLatin1String("dialog-warning");
		else icon_id = QLatin1String("dialog-error");
		auto label = new QLabel();
		label->setPixmap(QIcon::fromTheme(icon_id).pixmap(32, 32));  // TODO: Correct way to not hardcode size?
		layout->addWidget(label, row, 0);

		layout->addWidget(new QLabel(shortlabel + ": " + shortstatuslabel), row, 1);

		if (options.isEmpty()) {
			layout->addWidget(new QLabel(i18n("No action needed.")), row, 2);
		} else if (options.length() == 1) {
			layout->addWidget(new QLabel(options[0].shortlabel), row, 2);
		} else {
			box = new QComboBox();
			for (int i = 0; i < options.size(); ++i) {
				box->addItem(options[i].shortlabel);
			}
			layout->addWidget(box, row, 2);
		}

		if (!(longlabel.isEmpty() && options.isEmpty())) {
			QString details = longlabel;
			for (int i = 0; i < options.size(); ++i) {
				details += QString("<p><b>%1</b>: %2</p>").arg(options[i].shortlabel).arg(options[i].longlabel);
			}
			auto info = new QPushButton();
			info->setIcon(RKStandardIcons::getIcon(RKStandardIcons::WindowHelp));
			QObject::connect(info, &QPushButton::clicked, [details, layout]() { KMessageBox::information(layout->parentWidget(), details); });
			layout->addWidget(info, row, 3);
		}
	}
	void apply(RKSetupWizard *wizard) {
		if (options.isEmpty()) return;
		int opt = 0;
		if (box) opt = box->currentIndex();
		options[opt].callback(wizard);
	};

	struct Option {
		Option(const QString &shortlabel, const QString &longlabel, std::function<void(RKSetupWizard*)> callback) : shortlabel(shortlabel), longlabel(longlabel), callback(callback) {};
		QString shortlabel;
		QString longlabel;
		std::function<void(RKSetupWizard*)> callback;
	};
	QList<Option> options;
	Status status;
	QString shortstatuslabel;
	QString shortlabel;
	QString longlabel;
	QComboBox *box;
};

RKSetupWizardItem* makeRPackageCheck(const QString &packagename, const QString &explanation, RKSetupWizardItem::Status status_if_missing, RKSetupWizard* wizard) {
	RK_TRACE (DIALOGS);

	auto ret = new RKSetupWizardItem(packagename, explanation);
	if (!RKSessionVars::instance()->installedPackages().contains(packagename)) {
		ret->setStatus(status_if_missing, i18n("Not installed"));
		ret->addOption(i18n("Install %1", packagename), i18n("(Attempt to) install the package %1 and its dependencies for this version of R. In most cases, this requires a working internet connection.", packagename), [packagename](RKSetupWizard *wizard) { wizard->markRPackageForInstallation(packagename, true); });
		ret->addOption(i18n("No change"), i18n("Proceed without the package. You will again be given the option to install the package, when the pacakge is needed."), [packagename](RKSetupWizard *wizard) { wizard->markRPackageForInstallation(packagename, false); });
	} else {
		ret->setStatus(RKSetupWizardItem::Good, i18n("Installed"));
	}
	return ret;
}

RKSetupWizard::RKSetupWizard(QWidget* parent) : KAssistantDialog(parent) {
	RK_TRACE (DIALOGS);
}

RKSetupWizard::~RKSetupWizard() {
	RK_TRACE (DIALOGS);
	for(int i = 0; i < items.size(); ++i) {
		delete items[i];
	}
}

void RKSetupWizard::doAutoCheck() {
	RK_TRACE (DIALOGS);

	if (RKCommonFunctions::getRKWardDataDir ().isEmpty () || RKSettingsModulePlugins::pluginMaps().isEmpty() || (RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount() == 0)) {
		fullInteractiveCheck(ProblemsDetected);
	} else if (RKSettingsModuleGeneral::rkwardVersionChanged()) {
		fullInteractiveCheck(NewVersionRKWard);
	}

	// TODO: remove me
	fullInteractiveCheck(ProblemsDetected);
}

void RKSetupWizard::fullInteractiveCheck(InvokationReason reason) {
	RK_TRACE (DIALOGS);

	if (has_been_run && reason != ManualCheck) return;
	has_been_run = true;

	auto wizard = new RKSetupWizard(RKWardMainWindow::getMain());

	// Cover page
	auto firstpage = new QWidget();
	auto l = new QVBoxLayout(firstpage);
	QString intro = i18n("<p>This dialog will guide you through a quick check of the basic setup of the required (or recommended) components.</p>");
	if (reason == NewVersionRKWard) {
		intro += i18n("<p>The setup assistant has been invoked, automatically, because a new version of RKWard has been detected.</p");
	} else if (reason == NewVersionR) {
		// TODO: invoke this!
		intro += i18n("<p>The setup assistant has been invoked, automatically, because a new version of R has been detected.</p");
	} else if (reason == ProblemsDetected) {
		intro += i18n("<p>The setup assistant has been invoked, automatically, because a problem has been detected in your setup.</p");
	}
	l->addWidget(RKCommonFunctions::wordWrappedLabel(intro));
	auto waiting_to_start_label = RKCommonFunctions::wordWrappedLabel(i18n("<b>Waiting for R backend...</b>") + "<p>&nbsp;</p><p>&nbsp;</p>");
	l->addWidget(waiting_to_start_label);
	auto firstpageref = wizard->addPage (firstpage, i18n("RKWard Setup Assistant"));
	wizard->setValid(firstpageref, false);

	// Basic installation page
	wizard->createStandardPage();
	bool reinstallation_required = false;
	auto idir = new RKSetupWizardItem(i18n("Installation directory"));
	if (RKCommonFunctions::getRKWardDataDir ().isEmpty ()) {
		idir->setStatus(RKSetupWizardItem::Error, i18n("Not found."));
		// TODO: test, whether links work, here
		idir->setLongLabel("<p>RKWard either could not find its resource files at all, or only an old version of those files. The most likely cause is that the last installation failed to place the files in the correct place. This can lead to all sorts of problems, from single missing features to complete failure to function.</p><p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://rkward.kde.org/compiling\">http://rkward.kde.org/compiling</a>.</p>");
		idir->addOption(i18n("Reinstallation required"), i18n("This problem cannot be corrected, automatically. You will have to reinstall RKWard."), [](RKSetupWizard*) {});
		reinstallation_required = true;
	} else {
		idir->setStatus(RKSetupWizardItem::Good, i18n("Found."));
	}
	wizard->appendItem(idir);

	auto pluginmaps = new RKSetupWizardItem(i18n("RKWard plugins"));
	if (RKSettingsModulePlugins::pluginMaps().isEmpty()) {
		// TODO: Move to RKSettingsModulePlugins::validateSettingsInteractive
		pluginmaps->setLongLabel(i18n("<p>No plugins are enabled. This is probably not intended.</p>"));
		pluginmaps->setStatus(RKSetupWizardItem::Warning, i18n("None selected"));
		pluginmaps->addOption(i18n("Restore defaults"), i18n("Enable the default plugins"), [](RKSetupWizard*) { RKSettingsModulePlugins::registerDefaultPluginMaps(RKSettingsModulePlugins::AddIfDefault); });
		pluginmaps->addOption(i18n("No change"), i18n("Proceed without plugins"), [](RKSetupWizard*) {});

		// TODO: Also offer help, if a suspicioulsy small share of plugins is active? RKSettingsModulePlugins::knownUsablePluginCount();
	} else {
		pluginmaps->setStatus(RKSetupWizardItem::Good, i18n("Found."));
	}
	wizard->appendItem(pluginmaps);

	auto kateplugins = new RKSetupWizardItem(i18n("Kate plugins"));
	int kateplugincount = RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount();
	if (kateplugincount < 1) {
		// TODO: Move to RKSettingsModuleKatePlugins::validateSettingsInteractive
		kateplugins->setLongLabel(i18n("<p>Important functionality in RKWard is provided by kate plugins. It looks like none are installed on this system. On Linux/BSD, this can usually be fixed by installing kate.</p>"));
		kateplugins->setStatus(RKSetupWizardItem::Error, i18n("None found"));
		kateplugins->addOption(i18n("(Attempt to) install kate"), i18n("Attempt to install kate using the muon package manager. If this is not available, please install kate, manuelly"), [](RKSetupWizard* wizard) { wizard->markExternalPackageForInstallation(QStringLiteral("kate"), true); });
		kateplugins->addOption(i18n("No change"), i18n("Proceed without plugins"), [](RKSetupWizard* wizard) { wizard->markExternalPackageForInstallation(QStringLiteral("kate"), false); });
	} else {
		kateplugins->setStatus(RKSetupWizardItem::Good, i18n("Found %1 plugins.", kateplugincount));
	}
	wizard->appendItem(kateplugins);

	wizard->current_layout->setRowStretch(++wizard->current_row, 1);
	wizard->addPage(wizard->current_page, i18n("Basic installation"));

	// Wait for R Interface, then start dialog
	wizard->show();
	while (!RKGlobals::rInterface()->backendIsIdle()) {
		if (RKGlobals::rInterface()->backendIsDead()) {
			waiting_to_start_label->setText(i18n("<b>R backend has crashed. Click \"Cancel\" to exit setup assitant.</b>"));
		} else {
			QApplication::processEvents(QEventLoop::AllEvents, 1000);
		}
	}
	waiting_to_start_label->setText(i18n("<b>R backend has started. Click \"Next\" to continue.</b>"));
	wizard->setValid(firstpageref, true);

	// R packages page
	// This must be created _after_ the backend has started, for obvious reasons.
	wizard->createStandardPage();

	auto r2htmlpackage = makeRPackageCheck("R2HTML", i18n("The R2HTML package is used by nearly all RKWard output functions, and thus required."), RKSetupWizardItem::Error, wizard);
	wizard->appendItem(r2htmlpackage);
	auto rmarkdownpackage = makeRPackageCheck("rmarkdown", i18n("The rmarkdown package is required for rendering .Rmd files (including preview rendering), which is an optional but recommended feature."), RKSetupWizardItem::Warning, wizard);
	wizard->appendItem(rmarkdownpackage);

	wizard->current_layout->setRowStretch(++wizard->current_row, 1);
	wizard->addPage(wizard->current_layout->parentWidget(), i18n("R Packages"));

	wizard->exec();

	for(int i = 0; i < wizard->items.size(); ++i) {
		wizard->items[i]->apply(wizard);
	}

	if (!wizard->packages_to_install.isEmpty()) {
		RKLoadLibsDialog::showInstallPackagesModal(wizard, 0, wizard->packages_to_install);
	}

	delete wizard;
/* TODO
	RKSettingsModulePlugins::knownUsablePluginCount();
	RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount(); */

/* TODO
 * consolidate with RKSettings::validateSettingsInteractive */
}

void RKSetupWizard::createStandardPage() {
	RK_TRACE (DIALOGS);
	current_page = new QWidget();
	current_layout = new QGridLayout(current_page);
	current_layout->setColumnStretch(1, 2);
	current_layout->setColumnStretch(2, 1);
	current_row = -1;
}

void RKSetupWizard::appendItem(RKSetupWizardItem* item) {
	RK_TRACE (DIALOGS);
	item->createWidget(current_layout, ++current_row);
	items.append(item);
}

void RKSetupWizard::markExternalPackageForInstallation(const QString& name, bool install) {
	RK_TRACE (DIALOGS);
	bool present = software_to_install.contains(name);
	if (install && !present) software_to_install.append(name);
	if (present && !install) software_to_install.removeAll(name);
}

void RKSetupWizard::markRPackageForInstallation(const QString& name, bool install) {
	RK_TRACE (DIALOGS);
	bool present = packages_to_install.contains(name);
	if (install && !present) packages_to_install.append(name);
	if (present && !install) packages_to_install.removeAll(name);
}
