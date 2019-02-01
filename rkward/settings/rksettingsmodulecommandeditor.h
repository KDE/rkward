/***************************************************************************
                          rksettingsmodulecommandeditor  -  description
                             -------------------
    begin                : Tue Oct 23 2007
    copyright            : (C) 2007-2019 by Thomas Friedrichsmeier
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
#ifndef RKSETTINGSMODULECOMMANDEDITOR_H
#define RKSETTINGSMODULECOMMANDEDITOR_H

#include "rksettingsmodule.h"

class RKSpinBox;
class QCheckBox;
class QLineEdit;
class QGroupBox;
class QComboBox;

/**
configuration for the Command Editor windows

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleCommandEditor : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleCommandEditor (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleCommandEditor ();
	
	void applyChanges () override;
	void save (KConfig *config) override;

	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	static void validateSettingsInteractive (QList<RKSettingsWizardPage*>*) {};

	QString caption () override;

/// min number of character to try code completion
	static int completionMinChars () { return completion_min_chars; };
	static int completionTimeout () { return completion_timeout; };
	static bool completionEnabled () { return completion_enabled; };
	static bool argHintingEnabled () { return arghinting_enabled; };
	static int completionOptions () { return completion_options; };

	static bool autosaveEnabled () { return autosave_enabled; };
	static bool autosaveKeep () { return autosave_keep; };
	static int autosaveInterval () { return autosave_interval; };
	static QString autosaveSuffix () { return ".rkward_autosave"; };

	static int maxNumRecentFiles () { return num_recent_files; };
	static QString scriptFileFilter () { return script_file_filter; };
	static bool matchesScriptFileFilter (const QString &filename);
public slots:
	void settingChanged ();
private:
	static int completion_min_chars;
	static int completion_timeout;
	static bool completion_enabled;
	static bool arghinting_enabled;
	static int completion_options;

	RKSpinBox* completion_min_chars_box;
	RKSpinBox* completion_timeout_box;
	QCheckBox* completion_enabled_box;
	QCheckBox* arghinting_enabled_box;
	QComboBox* completion_list_member_operator_box;
	QComboBox* completion_slot_operator_box;
	QComboBox* completion_object_qualification_box;

	static bool autosave_enabled;
	static bool autosave_keep;
	static int autosave_interval;

	QGroupBox* autosave_enabled_box;
	QCheckBox* autosave_keep_box;
	RKSpinBox* autosave_interval_box;

	RKSpinBox* num_recent_files_box;
	QLineEdit* script_file_filter_box;
	static int num_recent_files;
	static QString script_file_filter;
};

#endif
