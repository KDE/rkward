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

#include "../settings/rksettingsmoduleplugins.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../misc/rkcommonfunctions.h"
#include "../windows/katepluginintegration.h"
#include "../plugin/rkcomponentmap.h"
#include "../rkward.h"

#include "../debug.h"

#include <KLocalizedString>

RKSetupWizard::has_been_run = false;

RKSetupWizard::RKSetupWizard(QWidget* parent) : KAssistantDialog(parent) {
	RK_TRACE (DIALOGS);


	if (RKCommonFunctions::getRKWardDataDir ().isEmpty ()) {
		KMessageBox::error (this, i18n ("<p>RKWard either could not find its resource files at all, or only an old version of those files. The most likely cause is that the last installation failed to place the files in the correct place. This can lead to all sorts of problems, from single missing features to complete failure to function.</p><p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://rkward.kde.org/compiling\">http://rkward.kde.org/compiling</a>.</p>"), i18n ("Broken installation"), KMessageBox::Notify | KMessageBox::AllowLink);
	}

	if (RKComponentMap::getMap()->isEmpty()) {
		KMessageBox::information (0, i18n ("Plugins are needed: you may manage these through \"Settings->Manage R packages and plugins\".\n"), i18n ("No active plugin maps"));
		return;
	}

	
	RKSettingsModulePlugins::knownUsablePluginCount();
	RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount();

}

// TOOD: consolidate with RKSettings::validateSettingsInteractive

void RKSetupWizard::doAutoCheck() {
	RK_TRACE (DIALOGS);

	if (RKCommonFunctions::getRKWardDataDir ().isEmpty () || RKComponentMap::getMap()->isEmpty() || (RKWardMainWindow::getMain()->katePluginIntegration()->knownPluginCount() == 0)) {
		fullInteractiveCheck(ProblemsDetected);
	} else if (RKSettingsModuleGeneral::rkwardVersionChanged()) {
		fullInteractiveCheck(NewVersionRKWard);
	}
}

void RKSetupWizard::fullInteractiveCheck(InvokationReason reason) {
	RK_TRACE (DIALOGS);

	if (has_been_run && reason != ManualCheck) return;
	has_been_run = true;

	auto wizard = new RKSetupWizard(RKWardMainWindow::getMain());
	QString intro = i18n("<h1>RKWard Setup Assistant</h1><p>This dialog will guide you through a quick check of the basic setup of the required (or recommended) components.</p>");
	if (reason == NewVersionRKWard) {
		i18n("<p>The setup assistant has been invoked, automatically, because a new version of RKWard has been detected.</p");
	} else if (reason == NewVersionR) {
		i18n("<p>The setup assistant has been invoked, automatically, because a new version of RKWard has been detected.</p");
	} else if (reason == ProblemsDetected) {
		i18n("<p>The setup assistant has been invoked, automatically, because a problem has been detected in your setup.</p");
	}
	auto label = RKCommonFunctions::wordWrappedLabel();
}
