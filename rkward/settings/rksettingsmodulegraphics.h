/***************************************************************************
                          rksettingsmodulegraphics  -  description
                             -------------------
    begin                : Mon Sep 13 2010
    copyright            : (C) 2010-2022 by Thomas Friedrichsmeier
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
#ifndef RKSETTINGSMODULEGRAPHICS_H
#define RKSETTINGSMODULEGRAPHICS_H

#include "rksettingsmodule.h"

#include <QStringList>

class QLineEdit;
class QGroupBox;
class RKSpinBox;
class QSpinBox;
class QButtonGroup;
class QRadioButton;

/**
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleGraphics : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleGraphics (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleGraphics ();
	
	void applyChanges () override;

/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();

/** Configured to (attempt to) use KDE printing dialog? */
	static bool kdePrintingEnabled () { return options_kde_printing; };

	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	QString caption() const override;
	QUrl helpURL () override { return QUrl ("rkward://page/rkward_plot_history#scd_settings"); };

	enum DefaultDevice {
		RKDevice,
		PlatformDevice,
		OtherDevice
	};
	enum StandardDevicesMode {
		ReplaceDevice,
		EmbedDevice,
		LeaveDevice
	};
	
	static bool plotHistoryEnabled () { return graphics_hist_enable; };
public slots:
	void boxChanged ();
private:
	void updateControls ();

	QButtonGroup *default_device_group;
	QLineEdit *default_device_other_edit;
	QButtonGroup *replace_standard_devices_group;

	QGroupBox *graphics_hist_box;

	static RKConfigValue<DefaultDevice, int> default_device;
	static RKConfigValue<QString> default_device_other;
	static RKConfigValue<StandardDevicesMode, int> replace_standard_devices;

	static RKConfigValue<bool> graphics_hist_enable;
	static RKConfigValue<int> graphics_hist_max_length;
	static RKConfigValue<int> graphics_hist_max_plotsize;

	static RKConfigValue<double> graphics_height;
	static RKConfigValue<double> graphics_width;

	static RKConfigValue<bool> options_kde_printing;
};

#endif
