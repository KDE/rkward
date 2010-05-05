/***************************************************************************
                          rksettingsmodulecommandeditor  -  description
                             -------------------
    begin                : Tue Oct 23 2007
    copyright            : (C) 2007, 2010 by Thomas Friedrichsmeier
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
#ifndef RKSETTINGSMODULECOMMANDEDITOR_H
#define RKSETTINGSMODULECOMMANDEDITOR_H

#include "rksettingsmodule.h"

class RKSpinBox;
class QCheckBox;
class QLineEdit;
class QGroupBox;

/**
configuration for the Command Editor windows

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleCommandEditor : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleCommandEditor (RKSettings *gui, QWidget *parent);

	~RKSettingsModuleCommandEditor ();
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();

/// min number of character to try code completion
	static int completionMinChars () { return completion_min_chars; };
	static int completionTimeout () { return completion_timeout; };
	static bool completionEnabled () { return completion_enabled; };

	static bool autosaveEnabled () { return autosave_enabled; };
	static bool autosaveKeep () { return autosave_keep; };
	static int autosaveInterval () { return autosave_interval; };
	static QString autosaveSuffix () { return ".rkward_autosave"; };
public slots:
	void settingChanged ();
private:
	static int completion_min_chars;
	static int completion_timeout;
	static bool completion_enabled;

	RKSpinBox* completion_min_chars_box;
	RKSpinBox* completion_timeout_box;
	QCheckBox* completion_enabled_box;

	static bool autosave_enabled;
	static bool autosave_keep;
	static int autosave_interval;
//	static QString autosave_suffix;

	QGroupBox* autosave_enabled_box;
	QCheckBox* autosave_keep_box;
	RKSpinBox* autosave_interval_box;
//	QLineEdit* autosave_suffix_edit;
};

#endif
