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
#include <QStandardPaths>

#include "../settings/rksettingsmoduleplugins.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettings.h"
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

void RKSetupWizardItem::createWidget(QGridLayout *layout, int row) {
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
		QObject::connect(info, &QPushButton::clicked, [details, layout]() { KMessageBox::information(layout->parentWidget(), details, QString(), QString(), KMessageBox::Notify | KMessageBox::AllowLink); });
		layout->addWidget(info, row, 3);
	}
}

void RKSetupWizardItem::apply(RKSetupWizard *wizard) {
	if (options.isEmpty()) return;
	int opt = 0;
	if (box) opt = box->currentIndex();
	options[opt].callback(wizard);
};

RKSetupWizardItem* makeRPackageCheck(const QString &packagename, const QString &explanation, RKSetupWizardItem::Status status_if_missing) {
	RK_TRACE (DIALOGS);

	auto ret = new RKSetupWizardItem(packagename, explanation);
	if (!RKSessionVars::instance()->installedPackages().contains(packagename)) {
		ret->setStatus(status_if_missing, i18n("Not installed"));
		ret->addOption(i18n("Install %1", packagename), i18n("(Attempt to) install the package %1 and its dependencies for this version of R. In most cases, this requires a working internet connection.", packagename), [packagename](RKSetupWizard *wizard) { wizard->markRPackageForInstallation(packagename, true); });
		ret->addOption(i18n("No change"), i18n("Proceed without the package. You will again be given the option to install the package, when the package is needed."), [packagename](RKSetupWizard *wizard) { wizard->markRPackageForInstallation(packagename, false); });
	} else {
		ret->setStatus(RKSetupWizardItem::Good, i18n("Installed"));
	}
	return ret;
}

void addSoftwareInstallOptions(RKSetupWizardItem* item, const QString &exename, const QString &downloadurl) {
	RK_TRACE (DIALOGS);

	item->addOption(i18n("Install %1", exename), i18n("Mark %1 for installation (actual installation is not yet supported, but you will be prompted with a link to a download page at the last page of this dialog)", exename), [exename, downloadurl](RKSetupWizard *wizard) { wizard->markSoftwareForInstallation(exename, downloadurl, true); });
	item->addOption(i18n("No change"), i18n("Proceed without %1. You will be missing some functionality.", exename), [exename, downloadurl](RKSetupWizard *wizard) { wizard->markSoftwareForInstallation(exename, downloadurl, false); });
}

RKSetupWizardItem* makeSoftwareCheck(const QString &exename, const QString& explanation, const QString &downloadurl, RKSetupWizardItem::Status status_if_missing) {
	RK_TRACE (DIALOGS);

	auto ret = new RKSetupWizardItem(exename, explanation);
	if (QStandardPaths::findExecutable(exename).isEmpty()) {
		ret->setStatus(status_if_missing, i18n("Not found"));
		addSoftwareInstallOptions(ret, exename, downloadurl);
	} else {
		ret->setStatus(RKSetupWizardItem::Good, i18n("Installed"));
	}
	return ret;
}

RKSetupWizard::RKSetupWizard(QWidget* parent, InvokationReason reason, const QList<RKSetupWizardItem*> &settings_items) : KAssistantDialog(parent) {
	RK_TRACE (DIALOGS);

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
	auto firstpageref = addPage (firstpage, i18n("RKWard Setup Assistant"));
	setValid(firstpageref, false);

	// Basic installation page
	createStandardPage();
	reinstallation_required = false;
	auto idir = new RKSetupWizardItem(i18n("Installation directory"));
	if (RKCommonFunctions::getRKWardDataDir ().isEmpty ()) {
		idir->setStatus(RKSetupWizardItem::Error, i18n("Not found."));
		idir->setLongLabel("<p>RKWard either could not find its resource files at all, or only an old version of those files. The most likely cause is that the last installation failed to place the files in the correct place. This can lead to all sorts of problems, from single missing features to complete failure to function.</p><p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"https://rkward.kde.org/Building_RKWard_From_Source.html\">https://rkward.kde.org/Building_RKWard_From_Source.html</a>.</p>");
		idir->addOption(i18n("Reinstallation required"), i18n("This problem cannot be corrected, automatically. You will have to reinstall RKWard."), [](RKSetupWizard*) {});
		reinstallation_required = true;
	} else {
		idir->setStatus(RKSetupWizardItem::Good, i18n("Found."));
	}
	appendItem(idir);

	auto pluginmaps = new RKSetupWizardItem(i18n("RKWard plugins"));
	if (RKSettingsModulePlugins::pluginMaps().isEmpty()) {
		pluginmaps->setLongLabel(i18n("<p>No plugins are enabled. This is probably not intended.</p>"));
		pluginmaps->setStatus(RKSetupWizardItem::Warning, i18n("None selected"));
		pluginmaps->addOption(i18n("Restore defaults"), i18n("Enable the default plugins"), [](RKSetupWizard*) { RKSettingsModulePlugins::registerDefaultPluginMaps(RKSettingsModulePlugins::AddIfDefault); });
		pluginmaps->addOption(i18n("No change"), i18n("Proceed without plugins"), [](RKSetupWizard*) {});

		// TODO: Also offer help, if a suspiciously small share of plugins is active? RKSettingsModulePlugins::knownUsablePluginCount();
	} else {
		pluginmaps->setStatus(RKSetupWizardItem::Good, i18n("Found."));
	}
	appendItem(pluginmaps);

	auto kateplugins = new RKSetupWizardItem(i18n("Kate plugins"));
	int kateplugincount = RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount();
	if (kateplugincount < 1) {
		kateplugins->setLongLabel(i18n("<p>Important functionality in RKWard is provided by kate plugins. It looks like none are installed on this system. On Linux/BSD, this can usually be fixed by installing kate.</p>"));
		kateplugins->setStatus(RKSetupWizardItem::Error, i18n("None found"));
		addSoftwareInstallOptions(kateplugins, QStringLiteral("kate"), "https://kate.kde.org");
	} else {
		kateplugins->setStatus(RKSetupWizardItem::Good, i18np("Found %1 plugin.", "Found %1 plugins.", kateplugincount));
	}
	appendItem(kateplugins);

	for (int i = 0; i < settings_items.size(); ++i) {
		appendItem(settings_items[i]);
	}

	current_layout->setRowStretch(++current_row, 1);
	addPage(current_page, i18n("Basic installation"));

	// Wait for R Interface, then start dialog
	setWindowModality(Qt::ApplicationModal);
	show();
	while (!RKGlobals::rInterface()->backendIsIdle()) {
		if (RKGlobals::rInterface()->backendIsDead()) {
			waiting_to_start_label->setText(i18n("<b>R backend has crashed. Click \"Cancel\" to exit setup assistant.</b>"));
		} else {
			QApplication::processEvents(QEventLoop::AllEvents, 1000);
		}
	}
	waiting_to_start_label->setText(i18n("<b>R backend has started. Click \"Next\" to continue.</b>"));
	setValid(firstpageref, true);

	// R packages page
	// This must be created _after_ the backend has started, for obvious reasons.
	createStandardPage();

	appendItem(makeRPackageCheck("R2HTML", i18n("The R2HTML package is used by nearly all RKWard output functions, and thus required."), RKSetupWizardItem::Error));
	appendItem(makeRPackageCheck("rmarkdown", i18n("The rmarkdown package is required for rendering .Rmd files (including preview rendering), which is an optional but recommended feature."), RKSetupWizardItem::Warning));

	current_layout->setRowStretch(++current_row, 1);
	addPage(current_page, i18n("R Packages"));

	// external software page
	createStandardPage();

	appendItem(makeSoftwareCheck("pandoc", i18n("The pandoc software is needed for rendering (or previewing) R markdown (.Rmd) files. This is optional but recommended."), "https://pandoc.org/installing.html", RKSetupWizardItem::Warning));
	appendItem(makeSoftwareCheck("kbibtex", i18n("The kbibtex software is useful for managing citations while writing articles. It integrates into RKWard via the Document Preview kate plugin."), "https://userbase.kde.org/KBibTeX", RKSetupWizardItem::Warning));

	current_layout->setRowStretch(++current_row, 1);
	second_to_last_page_ref = addPage(current_page, i18n("External software"));

	// summary page
	createStandardPage();
	last_page_label = RKCommonFunctions::linkedWrappedLabel("");
	current_layout->addWidget(last_page_label, 0, 0, 0, 3);
	current_layout->setRowStretch(1, 1);
	addPage(current_page, i18n("Summary of the next steps"));
}

RKSetupWizard::~RKSetupWizard() {
	RK_TRACE (DIALOGS);
	for(int i = 0; i < items.size(); ++i) {
		delete items[i];
	}
}

void RKSetupWizard::next() {
	RK_TRACE (DIALOGS);

	if (currentPage() == second_to_last_page_ref) {
		// NOTE: This is not quite clean: Some settings get applied before clicking finish, this way.
		//       However, I don't really want to pop up a separate dialog for a summary page, either.
		for(int i = 0; i < items.size(); ++i) {
			items[i]->apply(this);
		}
		QString label_text;
		if (reinstallation_required) {
			label_text.append(i18n("<h1>Reinstallation required</h1><p>Your installation of RKWard is broken, and cannot be fixed, automatically. You will have to reinstall RKWard!</p>"));
		}

		label_text = i18n("<h2>Software to install</h2>");
		if (!software_to_install.isEmpty()) {
			QString install_info;
			for (int i = 0; i < software_to_install.size(); ++i) {
				install_info.append("<ul>* <a href=\"");
				install_info.append(software_to_install_urls.value(i));
				install_info.append("\">");
				install_info.append(software_to_install[i]);
				install_info.append("</a></ul>");
			}
			label_text.append(i18n("<p>The following software is recommended for installation, but automatic installation is not (yet) supported. Click on the links, below, for download information:</p><li>%1</li>", install_info));
		} else {
			label_text.append(i18n("No software to install"));
		}
		label_text.append(i18n("<h2>R packages to install</h2>"));
		if (!packages_to_install.isEmpty()) {
			label_text.append(i18n("<p>%1 R packages are marked for installation. The R package installation dialog will be started when you press finish. You may be prompted to select a download mirror.</p>", packages_to_install.size()));
		} else {
			label_text.append(i18n("No R packages to install"));
		}

		// TODO: This height calculation is not quite correct, somehow, but good enough for now.
		int spare_height = height() - last_page_label->parentWidget()->sizeHint().height();
		last_page_label->setText(label_text);
		int new_height = qMax(height(), spare_height+last_page_label->minimumSizeHint().height());
		resize(width(), new_height);
	}
	KAssistantDialog::next();
}

void RKSetupWizard::doAutoCheck() {
	RK_TRACE (DIALOGS);

	// query settings modules for any problems
	QList<RKSetupWizardItem*> settings_items = RKSettings::validateSettingsInteractive();
	// check for those, and some cheap-but-important basics
	if (RKCommonFunctions::getRKWardDataDir ().isEmpty () || RKSettingsModulePlugins::pluginMaps().isEmpty() || (RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount() == 0) || !settings_items.isEmpty()) {
		fullInteractiveCheck(ProblemsDetected, settings_items);
	} else if (RKSettingsModuleGeneral::rkwardVersionChanged()) {
		fullInteractiveCheck(NewVersionRKWard, settings_items);
	}
}

void RKSetupWizard::manualCheck() {
	RK_TRACE (DIALOGS);
	fullInteractiveCheck(ManualCheck);
}

void RKSetupWizard::fullInteractiveCheck(InvokationReason reason, const QList<RKSetupWizardItem*> &settings_items) {
	RK_TRACE (DIALOGS);

	if (has_been_run && reason != ManualCheck) return;
	has_been_run = true;

	auto wizard = new RKSetupWizard(RKWardMainWindow::getMain(), reason, settings_items);

	auto res = wizard->exec();
	if (res == QDialog::Accepted) {
		if (!wizard->packages_to_install.isEmpty()) {
			RKLoadLibsDialog::showInstallPackagesModal(wizard, 0, wizard->packages_to_install);
		}

#if 0 && (defined(Q_OS_LINUX) || defined(Q_OS_UNIX))  // D'uh: muon (5.8.0) does not have an "install" command line option or equivalent
		if (!wizard->software_to_install.isEmpty()) {
			QString muonexe = QStandardPaths::findExecutable("muon");
			if(!muonexe.isEmpty()) {
				auto proc = new QProcess::startDetached("muon", QStringList() << "install" << wizard->software_to_install);
			}
		}
#endif
	}

	delete wizard;
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

void RKSetupWizard::markSoftwareForInstallation(const QString& name, const QString& downloadurl, bool install) {
	RK_TRACE (DIALOGS);
	bool present = software_to_install.contains(name);
	if (install && !present) {
		software_to_install.append(name);
		software_to_install_urls.append(downloadurl);
	}
	if (present && !install) {
		software_to_install.removeAll(name);
		software_to_install_urls.removeAll(downloadurl);
	}
}

void RKSetupWizard::markRPackageForInstallation(const QString& name, bool install) {
	RK_TRACE (DIALOGS);
	bool present = packages_to_install.contains(name);
	if (install && !present) packages_to_install.append(name);
	if (present && !install) packages_to_install.removeAll(name);
}
