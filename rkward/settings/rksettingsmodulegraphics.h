/*
rksettingsmodulegraphics - This file is part of the RKWard project. Created: Mon Sep 13 2010
SPDX-FileCopyrightText: 2010-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULEGRAPHICS_H
#define RKSETTINGSMODULEGRAPHICS_H

#include "rksettingsmodule.h"

#include <QStringList>

/**
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleGraphics : public RKSettingsModule {
	Q_OBJECT
  public:
	explicit RKSettingsModuleGraphics(QObject *parent);
	~RKSettingsModuleGraphics() override;

	/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands();

	/** Configured to (attempt to) use KDE printing dialog? */
	static bool kdePrintingEnabled() { return options_kde_printing; };

	void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction) override;
	void createPages(RKSettings *parent) override;
	static constexpr PageId page_id = QLatin1String("graphics");

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

	static bool plotHistoryEnabled() { return graphics_hist_enable; };

  private:
	friend class RKSettingsPageGraphics;
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
