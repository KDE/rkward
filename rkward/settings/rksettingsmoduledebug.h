/*
rksettingsmoduledebug - This file is part of the RKWard project. Created: Tue Oct 23 2007
SPDX-FileCopyrightText: 2007-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULEDEBUG_H
#define RKSETTINGSMODULEDEBUG_H

#include "rksettingsmodule.h"

class RKSpinBox;
class QButtonGroup;
class QFile;

/**
configuration for the Command Editor windows

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleDebug : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleDebug (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleDebug ();

	void applyChanges () override;
	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	QString caption() const override;

	// static members are declared in debug.h and defined in main.cpp
public Q_SLOTS:
	void settingChanged (int);
private:
	RKSpinBox* command_timeout_box;
	RKSpinBox* debug_level_box;
	QButtonGroup* debug_flags_group;
};

#endif
