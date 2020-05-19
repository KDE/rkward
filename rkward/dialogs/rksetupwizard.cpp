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
#include "../windows/katepluginintegration.h"
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
	void createWidget(QGridLayout *layout, int row) {
		QString icon_id;
		if (status == Good) icon_id = QLatin1String("dialog-positive");
		else if (status == Warning) icon_id = QLatin1String("dialog-warning");
		else icon_id = QLatin1String("dialog_error");
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
	void addOption(const QString &shortlabel, const QString &longlabel, std::function<void()> callback) {
		options.append(Option(shortlabel, longlabel, callback));
	}
	void setStatus(Status _status, const QString &_shortstatuslabel) { status = _status; shortstatuslabel = _shortstatuslabel; };
	void setShortLabel(const QString &label) { shortlabel = label; };
	void setLongLabel(const QString &label) { longlabel = label; };
	void apply() {
		if (options.isEmpty()) return;
		int opt = 0;
		if (box) opt = box->currentIndex();
		options[opt].callback();
	};
private:
	struct Option {
		Option(const QString &shortlabel, const QString &longlabel, std::function<void()> callback) : shortlabel(shortlabel), longlabel(longlabel), callback(callback) {};
		QString shortlabel;
		QString longlabel;
		std::function<void()> callback;
	};
	QList<Option> options;
	Status status;
	QString shortstatuslabel;
	QString shortlabel;
	QString longlabel;
	QComboBox *box;
};

RKSetupWizard::RKSetupWizard(QWidget* parent) : KAssistantDialog(parent) {
	RK_TRACE (DIALOGS);
/*
	if (RKCommonFunctions::getRKWardDataDir ().isEmpty ()) {
		KMessageBox::error (this, i18n ("<p>RKWard either could not find its resource files at all, or only an old version of those files. The most likely cause is that the last installation failed to place the files in the correct place. This can lead to all sorts of problems, from single missing features to complete failure to function.</p><p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://rkward.kde.org/compiling\">http://rkward.kde.org/compiling</a>.</p>"), i18n ("Broken installation"), KMessageBox::Notify | KMessageBox::AllowLink);
	}

	if (RKComponentMap::getMap()->isEmpty()) {
		KMessageBox::information (0, i18n ("Plugins are needed: you may manage these through \"Settings->Manage R packages and plugins\".\n"), i18n ("No active plugin maps"));
		return;
	} */
}

RKSetupWizard::~RKSetupWizard() {
	RK_TRACE (DIALOGS);
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

	auto page = new QWidget();
	auto layout = new QGridLayout(page);
	int row = -1;
	layout->setColumnStretch(1, 2);
	layout->setColumnStretch(2, 1);

	bool reinstallation_required = false;
	RKSetupWizardItem idir(i18n("Installation directory"));
	if (RKCommonFunctions::getRKWardDataDir ().isEmpty ()) {
		idir.setStatus(RKSetupWizardItem::Error, i18n("Not found."));
		// TODO: test, whether links work, here
		idir.setLongLabel("<p>RKWard either could not find its resource files at all, or only an old version of those files. The most likely cause is that the last installation failed to place the files in the correct place. This can lead to all sorts of problems, from single missing features to complete failure to function.</p><p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://rkward.kde.org/compiling\">http://rkward.kde.org/compiling</a>.</p>");
		idir.addOption(i18n("Reinstallation required"), i18n("This problem cannot be corrected, automatically. You will have to reinstall RKWard."), []() {});
		reinstallation_required = true;
	} else {
		idir.setStatus(RKSetupWizardItem::Good, i18n("Found."));
	}
	idir.createWidget(layout, ++row);

	RKSetupWizardItem pluginmaps(i18n("RKWard plugins"));
	if (RKSettingsModulePlugins::pluginMaps().isEmpty()) {
		// TODO: Move to RKSettingsModulePlugins::validateSettingsInteractive
		pluginmaps.setLongLabel(i18n("<p>No plugins are enabled. This is probably not intended.</p>"));
		pluginmaps.setStatus(RKSetupWizardItem::Warning, i18n("None selected"));
		pluginmaps.addOption(i18n("Restore defaults"), i18n("Enable the default plugins"), []() { RKSettingsModulePlugins::registerDefaultPluginMaps(RKSettingsModulePlugins::AddIfDefault); });
		pluginmaps.addOption(i18n("No change"), i18n("Proceed without plugins"), []() {});

		// TODO: Also offer help, if a suspicioulsy small share of plugins is active? RKSettingsModulePlugins::knownUsablePluginCount();
	} else {
		pluginmaps.setStatus(RKSetupWizardItem::Good, i18n("Found."));
	}
	pluginmaps.createWidget(layout, ++row);

	RKSetupWizardItem kateplugins(i18n("Kate plugins"));
	int kateplugincount = RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount();
	if (kateplugincount < 1) {
		// TODO: Move to RKSettingsModuleKatePlugins::validateSettingsInteractive
		kateplugins.setLongLabel(i18n("<p>Important functionality in RKWard is provided by kate plugins. It looks like none are installed on this system. On Linux/BSD, this can usually be fixed by installing kate.</p>"));
		kateplugins.setStatus(RKSetupWizardItem::Error, i18n("None found"));
		kateplugins.addOption(i18n("(Attempt to) install kate"), i18n("Attempt to install kate using the muon package manager. If this is not available, please install kate, manuelly"), []() {}); // TODO
		kateplugins.addOption(i18n("No change"), i18n("Proceed without plugins"), []() {});
	} else {
		kateplugins.setStatus(RKSetupWizardItem::Good, i18n("Found %1 plugins.", kateplugincount));
	}
	kateplugins.createWidget(layout, ++row);

	layout->setRowStretch(++row, 1);
	wizard->addPage(page, i18n("Basic installation"));

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

	wizard->exec();

	// TODO: apply all
	pluginmaps.apply();

	delete wizard;
/* TODO
	RKSettingsModulePlugins::knownUsablePluginCount();
	RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount(); */

/* TODO
 * consolidate with RKSettings::validateSettingsInteractive */
}
