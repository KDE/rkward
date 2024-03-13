/*
rksettingsmodulewatch - This file is part of the RKWard project. Created: Thu Aug 26 2004
SPDX-FileCopyrightText: 2004-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
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
	RKSettingsModuleWatch (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleWatch ();

	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	void applyChanges () override;
	void validateGUI ();

	static bool shouldShowInput (RCommand *command);
	static bool shouldShowOutput (RCommand *command);
	static bool shouldShowError (RCommand *command);
	static bool shouldRaiseWindow (RCommand *command);

	static uint maxLogLines () { return max_log_lines; };

	QString caption() const override;
	QIcon icon() const override;
public Q_SLOTS:
	void changedSetting (int);
private:
	enum FilterType { ShowInput=1, ShowOutput=2, ShowError=4, RaiseWindow=8 };

	static RKConfigValue<int> plugin_filter;
	static RKConfigValue<int> app_filter;
	static RKConfigValue<int> sync_filter;
	static RKConfigValue<int> user_filter;
	
	struct FilterBoxes {
		QCheckBox *input;
		QCheckBox *output;
		QCheckBox *error;
		QCheckBox *raise;
	};
	
	FilterBoxes *plugin_filter_boxes;
	FilterBoxes *app_filter_boxes;
	FilterBoxes *sync_filter_boxes;
	FilterBoxes *user_filter_boxes;

	int getFilterSettings (FilterBoxes *boxes);
	FilterBoxes *addFilterSettings (QWidget *parent, QGridLayout *layout, int row, const QString &label, int state);

	static RKConfigValue<uint> max_log_lines;
};

#endif
