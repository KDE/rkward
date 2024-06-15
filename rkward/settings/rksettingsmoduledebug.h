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
	RKSettingsModuleDebug(QObject *parent);
	~RKSettingsModuleDebug();

	void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction) override;
	QList<RKSettingsModuleWidget*> createPages(QWidget *parent) override;
	static constexpr PageId page_id = QLatin1String("debug");

	// static members are declared in debug.h and defined in main.cpp
};

#endif
