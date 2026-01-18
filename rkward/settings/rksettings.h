/*
rksettings - This file is part of the RKWard project. Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGS_H
#define RKSETTINGS_H

#include <KPageDialog>
#include <QMap>

#include "rksettingsmodule.h"

class RKSettingsModule;
class KConfig;
class RKWardMainWindow;
class RKSettingsTracker;
class RCommandChain;
class RKSetupWizardItem;

/**
The main settings-dialog. Contains subsections (tabs) for different modules. Use configureSettings () to invoke or raise the settings dialog

@author Thomas Friedrichsmeier
*/
class RKSettings : public KPageDialog {
	Q_OBJECT
  public:
	static void configureSettings(const RKSettingsModule::PageId page, QWidget *parent = nullptr, RCommandChain *chain = nullptr);

	static void loadSettings(KConfig *config);
	static void saveSettings(KConfig *config);
	/** Perform any settings validation that may need user interaction (and should happen after a GUI is available, and R has started up) */
	static QList<RKSetupWizardItem *> validateSettingsInteractive();

	void enableApply();
	void addSettingsPage(RKSettingsModuleWidget *which);
  public Q_SLOTS:
	void pageChange(KPageWidgetItem *current, KPageWidgetItem *before);

  protected:
	explicit RKSettings(QWidget *parent = nullptr);
	~RKSettings() override;

	void done(int result) override;
  private Q_SLOTS:
	void applyAll();
	void helpClicked();

  private:
	friend class RKWardCoreTest;
	static QList<RKSettingsModule *> modules;
	void initDialogPages();
	QList<KPageWidgetItem *> pages;
	KPageWidgetItem *findPage(const RKSettingsModule::PageId id) const;

	static RKSettings *settings_dialog;

	friend class RKSettingsModuleKatePlugins;
	/** dynamically remove the given page from the dialog (for pages provided by plugins, which might get unloaded) */
	void removeSettingsPage(RKSettingsModuleWidget *which);
};

#endif
