/***************************************************************************
                          rksettingsmoduleoutput  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004, 2010, 2011 by Thomas Friedrichsmeier
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
class QGroupBox;
class QComboBox;
class KIntSpinBox;
class RCommand;

/**
Allows to configure which types of commands should be "carbon copied" to the output window. Like the RKSettingsModules classes, this class encapsulates both, the setting itself,
and a widget to configure the settings.
@author Thomas Friedrichsmeier
*/
class RKCarbonCopySettings : public QWidget {
	Q_OBJECT
public:
	RKCarbonCopySettings (QWidget* parent);
	~RKCarbonCopySettings ();

	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);

	static bool shouldCarbonCopyCommand (const RCommand *command);
	static bool includeOutputInCarbonCopy () { return (cc_globally_enabled && cc_command_output); };
public slots:
	void applyChanges ();
signals:
	void changed ();
private slots:
	void settingChanged ();
private:
	// There can be multiple instances of this widget, which need to be kept in sync.
	static QList<RKCarbonCopySettings*> instances;
	void update ();

	QGroupBox *cc_globally_enabled_box;
	QCheckBox *cc_console_commands_box;
	QCheckBox *cc_script_commands_box;
	QCheckBox *cc_app_plugin_commands_box;
	QCheckBox *cc_command_output_box;

	static bool cc_globally_enabled;
	static bool cc_console_commands;
	static bool cc_script_commands;
	static bool cc_app_plugin_commands;
	static bool cc_command_output;
};

/**
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleOutput : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleOutput (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleOutput ();
	
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
	void boxChanged ();
private:
	QCheckBox *auto_show_box;
	QCheckBox *auto_raise_box;
	QComboBox *graphics_type_box;
	KIntSpinBox *graphics_width_box;
	KIntSpinBox *graphics_height_box;
	KIntSpinBox *graphics_jpg_quality_box;
	RKCarbonCopySettings *cc_settings;

	static bool auto_show;
	static bool auto_raise;
	static QString graphics_type;
	static int graphics_width;
	static int graphics_height;
	static int graphics_jpg_quality;
};

#endif
