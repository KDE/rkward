/***************************************************************************
                          rksettingsmoduleoutput  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004, 2010 by Thomas Friedrichsmeier
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
#ifndef RKSETTINGSMODULEOUTPUT_H
#define RKSETTINGSMODULEOUTPUT_H

#include "rksettingsmodule.h"

#include <QStringList>

class QCheckBox;
class QComboBox;
class KIntSpinBox;

/**
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleOutput : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleOutput (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleOutput ();
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);

/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();
	
	static bool autoShow () { return auto_show; };
	static bool autoRaise () { return auto_raise; };
public slots:
	void boxChanged (int);
private:
	QCheckBox *auto_show_box;
	QCheckBox *auto_raise_box;
	QComboBox *graphics_type_box;
	KIntSpinBox *graphics_width_box;
	KIntSpinBox *graphics_height_box;

	static bool auto_show;
	static bool auto_raise;
	static QString graphics_type;
	static int graphics_width;
	static int graphics_height;
};

#endif
