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
class QGridLayout;

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

	enum CompletionCategories {
		Calltip = 0,
		Arghint,
		Object,
		Filename,
		AutoWord,
		N_COMPLETION_CATEGORIES
	};

/// min number of character to try code completion
	static int autoCompletionMinChars () { return auto_completion_min_chars; };
	static int autoCompletionTimeout () { return auto_completion_timeout; };
	static bool autoCompletionEnabled () { return auto_completion_enabled; };
	static bool autoCompletionCursorActivated () { return (auto_completion_enabled && auto_completion_cursor_activated); };
	static bool argHintingEnabled () { return isCompletionEnabled (Arghint); };  // TODO: remove me
	static int completionOptions () { return completion_options; };
	static bool isCompletionEnabled (CompletionCategories cat) { return completion_type_enabled[cat]; };
	static bool cursorNavigatesCompletions () { return cursor_navigates_completions; };

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
	void makeCompletionTypeBoxes (const QStringList& label, QGridLayout* layout);

	static int auto_completion_min_chars;
	static int auto_completion_timeout;
	static bool auto_completion_enabled;
	static bool auto_completion_cursor_activated;
	static bool completion_type_enabled[N_COMPLETION_CATEGORIES];
	static bool cursor_navigates_completions;

	RKSpinBox* auto_completion_min_chars_box;
	RKSpinBox* auto_completion_timeout_box;
	QGroupBox* auto_completion_enabled_box;
	QCheckBox* auto_completion_cursor_activated_box;
	QCheckBox* completion_type_enabled_box[N_COMPLETION_CATEGORIES];
	QCheckBox* cursor_navigates_completions_box;

	static int completion_options;

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
