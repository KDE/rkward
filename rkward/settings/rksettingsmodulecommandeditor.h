/*
rksettingsmodulecommandeditor - This file is part of the RKWard project. Created: Tue Oct 23 2007
SPDX-FileCopyrightText: 2007-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULECOMMANDEDITOR_H
#define RKSETTINGSMODULECOMMANDEDITOR_H

#include "rksettingsmodule.h"
#include "../core/robject.h"

class RKSpinBox;
class QLineEdit;
class QGroupBox;
class QComboBox;
class QGridLayout;
namespace KTextEditor { class ConfigPage; }

class RKCodeCompletionSettingsWidget;
class RKCodeCompletionSettings {
public:
	RKCodeCompletionSettings() {};
	~RKCodeCompletionSettings() {};

	void syncConfig(KConfigGroup &cg, RKConfigBase::ConfigSyncAction a) { group.syncConfig(cg, a); };

	// NOTE: Don't insert values inbetween existing values, without also adjusting the sloppy config load/save/apply code
	enum CompletionCategories {
		Calltip = 0,
		Arghint,
		Object,
		Filename,
		AutoWord,
		MouseOver,
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
	bool completionForAllFileTypes() const { return completion_all_filetypes; };
private:
friend class RKCodeCompletionSettingsWidget;
friend class RKSettingsModuleConsole;
	RKConfigValue<bool> auto_completion_enabled {"Completion enabled", true};
	RKConfigValue<int> auto_completion_min_chars {"Completion min chars", 2};
	RKConfigValue<int> auto_completion_timeout {"Completion timeout", 250};
	RKConfigValue<bool> auto_completion_cursor_activated {"Auto completion on cursor navigation", false};
	RKConfigValue<bool> tabkey_invokes_completion {"Tabkey invokes completion", false};
	RKConfigValue<bool> completion_type_enabled[N_COMPLETION_CATEGORIES] {{"Calltips", true}, {"Argument completion", true}, {"Object completion", true}, {"Filename completion", true}, {"Auto word completion", true}, {"mouseover", true}};
	RKConfigValue<bool> cursor_navigates_completions {"Cursor navigate completions", true};
	RKConfigValue<int> completion_options {"Completion option flags", (int) RObject::IncludeEnvirIfMasked};
	RKConfigGroup dummyoptions = RKConfigGroup(nullptr, N_COMPLETION_CATEGORIES, completion_type_enabled);
	RKConfigValue<bool> completion_all_filetypes {"Completion all filetypes", true};
	RKConfigGroup group {"Completion", { &dummyoptions, &auto_completion_enabled, &auto_completion_min_chars, &auto_completion_timeout, &auto_completion_cursor_activated, &tabkey_invokes_completion, &cursor_navigates_completions, &completion_options, &completion_all_filetypes }};
};

class RKCodeCompletionSettingsWidget : public RKSettingsModuleWidget {
public:
	RKCodeCompletionSettingsWidget(QWidget *parent, RKSettingsModule *module, RKCodeCompletionSettings *settings, bool show_common);
	~RKCodeCompletionSettingsWidget() {};
	void applyChanges() override;
private:
	void makeCompletionTypeBoxes (const QStringList& labels, QGridLayout* layout);

	QGroupBox* auto_completion_enabled_box;

	RKCodeCompletionSettings *settings;
};

class RKSettingsModuleKTextEditorConfigWrapper : public RKSettingsModule {
public:
	RKSettingsModuleKTextEditorConfigWrapper(RKSettings* gui, QWidget* parent, KTextEditor::ConfigPage* wrapped);
	~RKSettingsModuleKTextEditorConfigWrapper();
	void applyChanges() override;
	void save(KConfig *) override { };
	static void validateSettingsInteractive(QList<RKSetupWizardItem*>*) {};
	QString caption() const override;
	QString longCaption() const override;
	QIcon icon() const override;
private:
	KTextEditor::ConfigPage* page;
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
	void save (KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	QString caption() const override;
	QIcon icon() const override;

	static const RKCodeCompletionSettings* completionSettings() { return &completion_settings; };

	static bool autosaveEnabled () { return autosave_enabled; };
	static bool autosaveKeep () { return autosave_keep; };
	static int autosaveInterval () { return autosave_interval; };
	static QString autosaveSuffix () { return ".rkward_autosave"; };

	static QString scriptFileFilter () { return script_file_filter; };
	static bool matchesScriptFileFilter (const QString &filename);

	static QList<RKSettingsModuleKTextEditorConfigWrapper*> kateConfigPages(RKSettings* gui, QWidget* parent);
private:
	static RKCodeCompletionSettings completion_settings;
	static RKConfigValue<bool> autosave_enabled;
	static RKConfigValue<bool> autosave_keep;
	static RKConfigValue<int> autosave_interval;

	RKCodeCompletionSettingsWidget *completion_settings_widget;
	QGroupBox* autosave_enabled_box;
	QLineEdit* script_file_filter_box;

	static RKConfigValue<QString> script_file_filter;
};

#endif
