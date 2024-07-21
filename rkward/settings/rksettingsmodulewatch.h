/*
rksettingsmodulewatch - This file is part of the RKWard project. Created: Thu Aug 26 2004
SPDX-FileCopyrightText: 2004-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULEWATCH_H
#define RKSETTINGSMODULEWATCH_H

#include "rksettingsmodule.h"

class RCommand;
class QCheckBox;
class QGridLayout;
class QSpinBox;

/**
Settings module for the RInterface-watch. Allows you to configure what kind of commands you would like to see/hide.

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleWatch : public RKSettingsModule
{
Q_OBJECT
public:
	explicit RKSettingsModuleWatch(QObject *parent);
	~RKSettingsModuleWatch() override;

	void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction) override;
	void createPages(RKSettings *parent) override;
	static constexpr PageId page_id = QLatin1String("commandlog");

	static bool shouldShowInput (RCommand *command);
	static bool shouldShowOutput (RCommand *command);
	static bool shouldShowError (RCommand *command);
	static bool shouldRaiseWindow (RCommand *command);

	static uint maxLogLines () { return max_log_lines; };

	static RKSettingsModuleWatch *instance() { return _instance; };
private:
friend class RKSettingsPageWatch;
	enum FilterType { ShowInput=1, ShowOutput=2, ShowError=4, RaiseWindow=8 };

	static RKConfigValue<int> plugin_filter;
	static RKConfigValue<int> app_filter;
	static RKConfigValue<int> sync_filter;
	static RKConfigValue<int> user_filter;

	static RKConfigValue<uint> max_log_lines;
	static RKSettingsModuleWatch* _instance;
};

#endif
