/***************************************************************************
                          rksettingsmodulegraphics  -  description
                             -------------------
    begin                : Mon Sep 13 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

class QGroupBox;
class RKSpinBox;
class KIntSpinBox;

/**
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleGraphics : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleGraphics (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleGraphics ();
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);

/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();
	
	static bool plotHistoryEnabled () { return graphics_hist_enable; };
public slots:
	void boxChanged ();
private:
	QGroupBox *graphics_hist_box;
	KIntSpinBox *graphics_hist_max_length_box;
	KIntSpinBox *graphics_hist_max_plotsize_box;

	RKSpinBox *graphics_height_box;
	RKSpinBox *graphics_width_box;

	static bool graphics_hist_enable;
	static int graphics_hist_max_length;
	static int graphics_hist_max_plotsize;

	static double graphics_height;
	static double graphics_width;
};

#endif
