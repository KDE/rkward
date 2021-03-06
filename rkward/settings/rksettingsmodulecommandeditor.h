/***************************************************************************
                          rksettingsmodulecommandeditor  -  description
                             -------------------
    begin                : Tue Oct 23 2007
    copyright            : (C) 2007-2020 by Thomas Friedrichsmeier
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
#include "../core/robject.h"

class RKSpinBox;
class QCheckBox;
class QLineEdit;
class QGroupBox;
class QComboBox;
class QGridLayout;

class RKCodeCompletionSettingsWidget;
class RKCodeCompletionSettings {
public:
	RKCodeCompletionSettings() {};
	~RKCodeCompletionSettings() {};

	void loadSettings(KConfigGroup &config) { group.loadConfig(config); };
	void saveSettings(KConfigGroup &config) { group.saveConfig(config); };

	// NOTE: Don't insert values inbetween existing values, without also adjusting the sloppy config load/save/apply code
	enum CompletionCategories {
		Calltip = 0,
		Arghint,
		Object,
		Filename,
		AutoWord,
		N_COMPLETION_CATEGORIES
	};

/// min number of character to try code completion
	int autoMinChars() const { return auto_completion_min_chars; };
	int autoTimeout() const { return auto_completion_timeout; };
	bool autoEnabled() const { return auto_completion_enabled; };
	bool autoCursorActivated() const { return (auto_completion_enabled && auto_completion_cursor_activated); };
	bool argHintingEnabled() const { return isEnabled(Arghint); };  // TODO: remove me
	int options() const { return completion_options; };
	bool isEnabled(CompletionCategories cat) const { return completion_type_enabled[cat]; };
	bool cursorNavigatesCompletions() const { return cursor_navigates_completions; };
	bool tabKeyInvokesCompletion() const { return tabkey_invokes_completion; };
private:
friend class RKCodeCompletionSettingsWidget;
friend class RKSettingsModuleConsole;
	RKConfigValue<bool> auto_completion_enabled {"Completion enabled", true};
	RKConfigValue<int> auto_completion_min_chars {"Completion min chars", 2};
	RKConfigValue<int> auto_completion_timeout {"Completion timeout", 250};
	RKConfigValue<bool> auto_completion_cursor_activated {"Auto completion on cursor navigation", false};
	RKConfigValue<bool> tabkey_invokes_completion {"Tabkey invokes completion", false};
	RKConfigValue<bool> completion_type_enabled[N_COMPLETION_CATEGORIES] {{"Calltips", true}, {"Argument completion", true}, {"Object completion", true}, {"Filename completion", true}, {"Auto word completion", true}};
	RKConfigValue<bool> cursor_navigates_completions {"Cursor navigate completions", false};
	RKConfigValue<int> completion_options {"Completion option flags", (int) RObject::IncludeEnvirIfMasked};
	RKConfigGroup dummyoptions = RKConfigGroup(0, N_COMPLETION_CATEGORIES, completion_type_enabled);
	RKConfigGroup group {"Completion", { &dummyoptions, &auto_completion_enabled, &auto_completion_min_chars, &auto_completion_timeout, &auto_completion_cursor_activated, &tabkey_invokes_completion, &cursor_navigates_completions, &completion_options }};
};

class RKCodeCompletionSettingsWidget : public RKSettingsModuleWidget {
public:
	RKCodeCompletionSettingsWidget(QWidget *parent, RKSettingsModule *module, RKCodeCompletionSettings *settings, bool show_common);
	~RKCodeCompletionSettingsWidget() {};
	void applyChanges() override;
private:
	void makeCompletionTypeBoxes (const QStringList& labels, QGridLayout* layout);

	RKSpinBox* auto_completion_min_chars_box;
	RKSpinBox* auto_completion_timeout_box;
	QGroupBox* auto_completion_enabled_box;
	QCheckBox* auto_completion_cursor_activated_box;
	QCheckBox* tabkey_invokes_completion_box;
	QCheckBox* completion_type_enabled_box[RKCodeCompletionSettings::N_COMPLETION_CATEGORIES];
	QComboBox* cursor_navigates_completions_box;
	QComboBox* completion_list_member_operator_box;
	QComboBox* completion_slot_operator_box;
	QComboBox* completion_object_qualification_box;

	RKCodeCompletionSettings *settings;
	bool show_common;
};

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
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	QString caption () override;

	static const RKCodeCompletionSettings* completionSettings() { return &completion_settings; };

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
	static RKCodeCompletionSettings completion_settings;
	static bool autosave_enabled;
	static bool autosave_keep;
	static int autosave_interval;

	RKCodeCompletionSettingsWidget *completion_settings_widget;
	QGroupBox* autosave_enabled_box;
	QCheckBox* autosave_keep_box;
	RKSpinBox* autosave_interval_box;

	RKSpinBox* num_recent_files_box;
	QLineEdit* script_file_filter_box;
	static int num_recent_files;
	static QString script_file_filter;
};

#endif
