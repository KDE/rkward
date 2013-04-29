/***************************************************************************
                          rksettingsmodulegraphics  -  description
                             -------------------
    begin                : Mon Sep 13 2010
    copyright            : (C) 2010, 2013 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
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
class KIntSpinBox;
class QCheckBox;
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
	
	void applyChanges ();
	void save (KConfig *config);

/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();

/** Configured to (attempt to) use KDE printing dialog? */
	static bool kdePrintingEnabled () { return options_kde_printing; };

	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();
	QString helpURL () { return ("rkward://page/rkward_plot_history#scd_settings"); };

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
	KIntSpinBox *graphics_hist_max_length_box;
	KIntSpinBox *graphics_hist_max_plotsize_box;

	RKSpinBox *graphics_height_box;
	RKSpinBox *graphics_width_box;

	QCheckBox *kde_printing_box;

	static DefaultDevice default_device;
	static QString default_device_other;
	static StandardDevicesMode replace_standard_devices;

	static bool graphics_hist_enable;
	static int graphics_hist_max_length;
	static int graphics_hist_max_plotsize;

	static double graphics_height;
	static double graphics_width;

	static bool options_kde_printing;
};

#endif
