/*
rksetupwizard - This file is part of RKWard (https://rkward.kde.org). Created: Fri May 25 20200
SPDX-FileCopyrightText: 2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rksetupwizard.h"

#include <functional>

#include <QLabel>
#include <QApplication>
#include <QComboBox>
#include <QGridLayout>
#include <QIcon>
#include <QPushButton>
#include <QTimer>
#include <QFileInfo>
#include <QStandardPaths>
#include <QRadioButton>
#include <QGroupBox>

#include <KLocalizedString>
#include <KMessageBox>

#include "../settings/rksettingsmoduleplugins.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettings.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardicons.h"
#include "../dialogs/rkloadlibsdialog.h"
#include "../windows/katepluginintegration.h"
#include "../rbackend/rksessionvars.h"
#include "../rbackend/rkrinterface.h"

#include "../rkward.h"

#include "../debug.h"

class RKSetupWizardPage : public QWidget {
public:
	RKSetupWizardPage(RKSetupWizard* wizard, const QString& caption) : QWidget(), wizard(wizard), current_row(-1), glayout(nullptr) {
		RK_TRACE (DIALOGS);
		ref = wizard->addPage(this, caption);
	};

	void lazyInitOnce(std::function<void(RKSetupWizardPage*)> initfun) {
		auto refcopy = ref;
		QObject::connect(wizard, &KPageDialog::currentPageChanged, this, [this, refcopy, initfun](KPageWidgetItem* current, KPageWidgetItem*) mutable {
			if (current && current == refcopy) {
				initfun(this);
				refcopy = nullptr;
			}
		});
	}
	void lazyInitRepeated(std::function<void(RKSetupWizardPage*)> initfun) {
		QObject::connect(wizard, &KPageDialog::currentPageChanged, this, [this, initfun](KPageWidgetItem* current, KPageWidgetItem*) {
			if (current && current == ref) {
				initfun(this);
			}
		});
	}

	RKSetupWizard *wizard;
	int current_row;
	QGridLayout *glayout;
	KPageWidgetItem *ref;
	void appendItem(RKSetupWizardItem* item) {
		RK_TRACE (DIALOGS);

		if (!glayout) {
			RK_ASSERT(!layout());  // must now mix with different type of layout
			glayout = new QGridLayout(this);
			glayout->setColumnStretch(1, 2);
			glayout->setColumnStretch(2, 1);
		}

		item->createWidget(glayout, ++current_row);
		wizard->items.append(item);
	};
	void addStretch() {
		RK_ASSERT(glayout); // Offered for grid-layout pages, only, for now
		glayout->setRowStretch(++current_row, 1);
	}
};

bool RKSetupWizard::has_been_run = false;

static auto iconForStatus(RKSetupWizardItem::Status status) {
	QString icon_id;
	if (status == RKSetupWizardItem::Good) icon_id = QLatin1String("dialog-positive");
	else if (status == RKSetupWizardItem::Warning) icon_id = QLatin1String("dialog-warning");
	else icon_id = QLatin1String("dialog-error");
	return (QIcon::fromTheme(icon_id).pixmap(32, 32));  // TODO: Correct way to not hardcode size?
}

void RKSetupWizardItem::createWidget(QGridLayout *layout, int row) {
	auto label = new QLabel();
	label->setPixmap(iconForStatus(status));
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
			details += QString("<p><b>%1</b>: %2</p>").arg(options[i].shortlabel, options[i].longlabel);
		}
		auto info = new QPushButton();
		info->setIcon(RKStandardIcons::getIcon(RKStandardIcons::WindowHelp));
		QObject::connect(info, &QPushButton::clicked, layout, [details, layout]() { KMessageBox::information(layout->parentWidget(), details, QString(), QString(), KMessageBox::Notify | KMessageBox::AllowLink); });
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
	auto page = new RKSetupWizardPage(this, i18n("RKWard Setup Assistant"));
	auto l = new QVBoxLayout(page);
	QString intro = i18n("<p>This dialog will guide you through a quick check of the basic setup of the required (or recommended) components.</p>");
	if (reason == NewVersionRKWard) {
		intro += i18n("<p>The setup assistant has been invoked, automatically, because a new version of RKWard has been detected.</p>");
	} else if (reason == NewVersionR) {
		// TODO: invoke this!
		intro += i18n("<p>The setup assistant has been invoked, automatically, because a new version of R has been detected.</p>");
	} else if (reason == ProblemsDetected) {
		intro += i18n("<p>The setup assistant has been invoked, automatically, because a problem has been detected in your setup.</p>");
	}
	l->addWidget(RKCommonFunctions::wordWrappedLabel(intro));
	l->addStretch();

	// Basic installation page
	page = new RKSetupWizardPage(this, i18n("Basic installation"));
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
	page->appendItem(idir);

	auto pluginmaps = new RKSetupWizardItem(i18n("RKWard plugins"));
	if (RKSettingsModulePlugins::pluginMaps().isEmpty()) {
		pluginmaps->setLongLabel(i18n("<p>No plugins are enabled. This is probably not intended.</p>"));
		pluginmaps->setStatus(RKSetupWizardItem::Warning, i18n("None selected"));
		pluginmaps->addOption(i18n("Restore defaults"), i18n("Enable the default plugins"), [](RKSetupWizard*) { RKSettingsModulePlugins::registerDefaultPluginMaps(RKSettingsModulePlugins::AutoActivate); });
		pluginmaps->addOption(i18n("No change"), i18n("Proceed without plugins"), [](RKSetupWizard*) {});

		// TODO: Also offer help, if a suspiciously small share of plugins is active? RKSettingsModulePlugins::knownUsablePluginCount();
	} else {
		pluginmaps->setStatus(RKSetupWizardItem::Good, i18n("Found."));
	}
	page->appendItem(pluginmaps);

	auto kateplugins = new RKSetupWizardItem(i18n("Kate plugins"));
	int kateplugincount = RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount();
	if (kateplugincount < 1) {
		kateplugins->setLongLabel(i18n("<p>Important functionality in RKWard is provided by kate plugins. It looks like none are installed on this system. On Linux/BSD, this can usually be fixed by installing kate.</p>"));
		kateplugins->setStatus(RKSetupWizardItem::Error, i18n("None found"));
		addSoftwareInstallOptions(kateplugins, QStringLiteral("kate"), "https://kate.kde.org");
	} else {
		kateplugins->setStatus(RKSetupWizardItem::Good, i18np("Found %1 plugin.", "Found %1 plugins.", kateplugincount));
	}
	page->appendItem(kateplugins);

	// TODO: Remove, eventually
	auto legacy_output = new RKSetupWizardItem(i18n("Pre 0.7.3 output file"));
	QString legacy_output_path = RKSettingsModuleGeneral::filesPath() + "rk_out.html";
	if (QFileInfo::exists(legacy_output_path)) {
		legacy_output->setStatus(RKSetupWizardItem::Warning, i18n("Exists"));
		legacy_output->setLongLabel(QString("<p>An output file from before RKWard version 0.7.3 was found (%1). You will probably want to convert this to the new format. Alternatively, if it is no longer needed, you can delete it, manually.</p>").arg(legacy_output_path));
		legacy_output->addOption(i18n("Import"), i18n("Import to the session, so you can save in the new output format."), [](RKSetupWizard* wizard) { wizard->r_commands_to_run.append("rk.import.legacy.output()\n"); });
		legacy_output->addOption(i18n("No action"), i18n("Ignore (and keep) the file. You can import it manually, at any time, using rk.import.legacy.output()"), [](RKSetupWizard*) {});
	} else {
		legacy_output->setStatus(RKSetupWizardItem::Good, i18n("Found."));
	}
	page->appendItem(legacy_output);

	for (int i = 0; i < settings_items.size(); ++i) {
		page->appendItem(settings_items[i]);
	}
	page->addStretch();

	// The following pages need the R backend, and are thus initialized lazily
	// R backend page
	page = new RKSetupWizardPage(this, i18n("R Backend"));
	page->lazyInitOnce([this](RKSetupWizardPage *p) {
		auto l = new QVBoxLayout(p);
		l->addWidget(new QLabel(i18n("RKWard is currently using the R installation at <nobr>%1</nobr>.", RKSessionVars::RBinary())));
		auto h = new QHBoxLayout();
		l->addLayout(h);
		auto rstatus_label = RKCommonFunctions::wordWrappedLabel(i18n("<b>Waiting for R backend...</b>"));
		auto rstatus_icon = new QLabel();
		auto anim = RKStandardIcons::busyAnimation(rstatus_icon, [rstatus_icon](const QIcon& icon) {
			rstatus_icon->setPixmap(icon.pixmap(32, 32));
		});
		h->addWidget(rstatus_icon);
		h->addWidget(rstatus_label);
		h->setStretch(1, 2);
		l->addStretch();
		auto pageref = p->ref;
		setValid(pageref, false);
		auto statuslambda = [this, pageref, rstatus_label, rstatus_icon, anim, l]() {
			if (!(RInterface::instance()->backendIsDead() || RInterface::instance()->backendIsIdle())) return;

			anim->stop();
			if (RInterface::instance()->backendIsDead()) {
				rstatus_icon->setPixmap(iconForStatus(RKSetupWizardItem::Error));
				rstatus_label->setText(i18n("<b>R backend has crashed.</b>"));
			} else {
				RK_ASSERT(RInterface::instance()->backendIsIdle());
				QString statustext = i18n("<b>R version %1 started, successfully.</b>", RKSessionVars::RVersion(false));
				if (RKSessionVars::isPathInAppImage(RKSessionVars::RBinary())) {
					rstatus_icon->setPixmap(iconForStatus(RKSetupWizardItem::Warning));
					statustext.append(i18n("<p>You are using the R version bundled inside the RKWard AppImage.</p>"
					                       "<p>This version comes with several technical limitations. Importantly, "
					                       "yu will not be able to install most R addon packages. "
					                       "In general, it is therefore recommended to select a system-installed "
					                       "version of R, instead, below.</p>"));
				} else {
					rstatus_icon->setPixmap(iconForStatus(RKSetupWizardItem::Good));
				}
				rstatus_label->setText(statustext);
				setValid(pageref, true);
			}

			auto box = new QGroupBox(i18n("R version to use"));
			l->addWidget(box);
			auto bl = new QVBoxLayout(box);
			auto button = new QRadioButton(i18n("Keep current version"), box); bl->addWidget(button);
			button = new QRadioButton(i18n("Some dummy option, TODO"), box); bl->addWidget(button);
		};
		connect(RInterface::instance(), &RInterface::backendStatusChanged, this, statuslambda);
		statuslambda();
	});

	// R packages page
	page = new RKSetupWizardPage(this, i18n("R Packages"));
	page->lazyInitOnce([](RKSetupWizardPage *p) {
		p->appendItem(makeRPackageCheck("R2HTML", i18n("The R2HTML package is used by nearly all RKWard output functions, and thus required."), RKSetupWizardItem::Error));
		p->appendItem(makeRPackageCheck("rmarkdown", i18n("The rmarkdown package is required for rendering .Rmd files (including preview rendering), which is an optional but recommended feature."), RKSetupWizardItem::Warning));
		p->addStretch();
	});

	// external software page
	page = new RKSetupWizardPage(this, i18n("External software"));
	page->lazyInitOnce([](RKSetupWizardPage *p) {
		p->appendItem(makeSoftwareCheck("pandoc", i18n("The pandoc software is needed for rendering (or previewing) R markdown (.Rmd) files. This is optional but recommended."), "https://pandoc.org/installing.html", RKSetupWizardItem::Warning));
		p->appendItem(makeSoftwareCheck("kbibtex", i18n("The kbibtex software is useful for managing citations while writing articles. It integrates into RKWard via the Document Preview kate plugin."), "https://userbase.kde.org/KBibTeX", RKSetupWizardItem::Warning));
		p->addStretch();
	});

	// summary page
	page = new RKSetupWizardPage(this, i18n("Summary of the next steps"));
	l = new QVBoxLayout(page);
	auto last_page_label = RKCommonFunctions::linkedWrappedLabel("");
	l->addWidget(last_page_label);
	l->addStretch();
	page->lazyInitRepeated([this, last_page_label](RKSetupWizardPage *) {
		software_to_install.clear();
		packages_to_install.clear();
		r_commands_to_run.clear();;

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
	});
}

RKSetupWizard::~RKSetupWizard() {
	RK_TRACE (DIALOGS);
	for(int i = 0; i < items.size(); ++i) {
		delete items[i];
	}
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
			RKLoadLibsDialog::showInstallPackagesModal(wizard, nullptr, wizard->packages_to_install);
		}

#if 0 && (defined(Q_OS_LINUX) || defined(Q_OS_UNIX))  // D'uh: muon (5.8.0) does not have an "install" command line option or equivalent
		if (!wizard->software_to_install.isEmpty()) {
			QString muonexe = QStandardPaths::findExecutable("muon");
			if(!muonexe.isEmpty()) {
				auto proc = new QProcess::startDetached("muon", QStringList() << "install" << wizard->software_to_install);
			}
		}
#endif
		for(int i = 0; i < wizard->r_commands_to_run.size(); ++i) {
			RInterface::issueCommand(wizard->r_commands_to_run[i], RCommand::App);
		}
	}

	delete wizard;
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
