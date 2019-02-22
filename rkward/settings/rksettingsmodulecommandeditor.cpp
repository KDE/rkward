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
#include "rksettingsmodulecommandeditor.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>

#include "../misc/rkspinbox.h"
#include "../misc/rkcommonfunctions.h"
#include "../core/robject.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
int RKSettingsModuleCommandEditor::auto_completion_min_chars;
int RKSettingsModuleCommandEditor::auto_completion_timeout;
bool RKSettingsModuleCommandEditor::auto_completion_enabled;
bool RKSettingsModuleCommandEditor::completion_type_enabled[RKSettingsModuleCommandEditor::N_COMPLETION_CATEGORIES];
int RKSettingsModuleCommandEditor::completion_options;
bool RKSettingsModuleCommandEditor::cursor_navigates_completions;
bool RKSettingsModuleCommandEditor::autosave_enabled;
bool RKSettingsModuleCommandEditor::autosave_keep;
int RKSettingsModuleCommandEditor::autosave_interval;
int RKSettingsModuleCommandEditor::num_recent_files;
QString RKSettingsModuleCommandEditor::script_file_filter;

RKSettingsModuleCommandEditor::RKSettingsModuleCommandEditor (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout* main_vbox = new QVBoxLayout (this);
	QLabel *label = new QLabel (i18n ("Settings marked with (*) do not take effect until you restart RKWard"), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);
	main_vbox->addSpacing (2 * RKGlobals::spacingHint ());

	QGroupBox* group = new QGroupBox (i18n ("Code Completion / Code Hints"), this);
	QVBoxLayout* box_layout = new QVBoxLayout (group);

	QGridLayout *g_layout = new QGridLayout ();
	box_layout->addLayout (g_layout);
	makeCompletionTypeBoxes (QStringList () << i18n ("Function call tip") << i18n ("Function argument completion") << i18n ("Object name completion") << i18n ("Filename completion") << i18n ("Auto word completion"), g_layout);

	auto_completion_enabled_box = new QGroupBox (i18n ("Start code completions/hints, automatically"), group);
	auto_completion_enabled_box->setCheckable (true);
	auto_completion_enabled_box->setChecked (auto_completion_enabled);
	connect (auto_completion_enabled_box, &QGroupBox::toggled, this, &RKSettingsModuleCommandEditor::settingChanged);
	box_layout->addWidget (auto_completion_enabled_box);

	QFormLayout* form_layout = new QFormLayout (auto_completion_enabled_box);
	auto_completion_min_chars_box = new RKSpinBox (auto_completion_enabled_box);
	auto_completion_min_chars_box->setIntMode (1, INT_MAX, auto_completion_min_chars);
	auto_completion_min_chars_box->setEnabled (auto_completion_enabled);
	connect (auto_completion_min_chars_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow ("Minimum number of characters", auto_completion_min_chars_box);

	auto_completion_timeout_box = new RKSpinBox (auto_completion_enabled_box);
	auto_completion_timeout_box->setIntMode (0, INT_MAX, auto_completion_timeout);
	auto_completion_timeout_box->setEnabled (auto_completion_enabled);
	connect (auto_completion_timeout_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow (i18n ("Timeout (milliseconds)"), auto_completion_timeout_box);

	form_layout = new QFormLayout ();
	box_layout->addLayout (form_layout);

	cursor_navigates_completions_box = new QCheckBox (i18n ("Up/down cursor keys navigate completion items"));
	cursor_navigates_completions_box->setChecked (cursor_navigates_completions);
	RKCommonFunctions::setTips (i18n ("Should the up / down cursor keys be used to navigate among the completion items, while code completion is active? If this option is unchecked, Alt+up/down will navigate completion items, while up / down will behave as if no completion was active."), cursor_navigates_completions_box);
	connect (cursor_navigates_completions_box, &QCheckBox::stateChanged, this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow (cursor_navigates_completions_box);

	completion_list_member_operator_box = new QComboBox (group);
	completion_list_member_operator_box->addItem (i18n ("'$'-operator (list$member)"));
	completion_list_member_operator_box->addItem (i18n ("'[['-operator (list[[\"member\"]])"));
	completion_list_member_operator_box->setCurrentIndex ((completion_options & RObject::DollarExpansion) ? 0 : 1);
	connect (completion_list_member_operator_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow (i18nc ("Note: list() and data.frame() are programming terms in R, and should not be translated, here", "Operator for access to members of list() and data.frame() objects"), completion_list_member_operator_box);

	completion_slot_operator_box = new QComboBox (group);
	completion_slot_operator_box->addItem (i18n ("'@'-operator (object@smember)"));
	completion_slot_operator_box->addItem (i18n ("'slot()'-function (slot(object, member))"));
	completion_slot_operator_box->setCurrentIndex ((completion_options & RObject::ExplicitSlotsExpansion) ? 1 : 0);
	connect (completion_slot_operator_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow (i18nc ("Note: S4-slot() is a programming term in R, and should not be translated, here", "Operator for access to S4-slot()s"), completion_slot_operator_box);

	completion_object_qualification_box = new QComboBox (group);
	completion_object_qualification_box->addItem (i18n ("For masked objects, only"));
	completion_object_qualification_box->addItem (i18n ("For objects outside of <i>.GlobalEnv</i>, only"));
	completion_object_qualification_box->addItem (i18n ("Always"));
	if (completion_options & (RObject::IncludeEnvirIfNotGlobalEnv)) {
		if (completion_options & (RObject::IncludeEnvirIfNotGlobalEnv)) completion_object_qualification_box->setCurrentIndex (2);
		else completion_object_qualification_box->setCurrentIndex (1);
	}
	connect (completion_object_qualification_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow (i18n ("Include environment for objects on the search path:"), completion_object_qualification_box);

	main_vbox->addWidget (group);

	main_vbox->addSpacing (2 * RKGlobals::spacingHint ());

	group = autosave_enabled_box = new QGroupBox (i18n ("Autosaves"), this);
	autosave_enabled_box->setCheckable (true);
	autosave_enabled_box->setChecked (autosave_enabled);
	connect (autosave_enabled_box, &QGroupBox::toggled, this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout = new QFormLayout (group);

	autosave_interval_box = new RKSpinBox (group);
	autosave_interval_box->setIntMode (1, INT_MAX, autosave_interval);
	connect (autosave_interval_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow (i18n ("Autosave interval (minutes)"), autosave_interval_box);

	autosave_keep_box = new QCheckBox (i18n ("Keep autosave file after manual save"), group);
	autosave_keep_box->setChecked (autosave_keep);
	connect (autosave_keep_box, &QCheckBox::stateChanged, this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow (autosave_keep_box);

	main_vbox->addWidget (group);

	main_vbox->addSpacing (2 * RKGlobals::spacingHint ());

	group = new QGroupBox (i18n ("Opening script files"), this);
	form_layout = new QFormLayout (group);
	num_recent_files_box = new RKSpinBox (group);
	num_recent_files_box->setIntMode (1, INT_MAX, num_recent_files);
	RKCommonFunctions::setTips (i18n ("<p>The number of recent files to remember (in the Open Recent R Script File menu).</p>") + RKCommonFunctions::noteSettingsTakesEffectAfterRestart (), num_recent_files_box, label);
	connect (num_recent_files_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow (i18n ("Number of scripts in recent file lists (*)"), num_recent_files_box);

	script_file_filter_box = new QLineEdit (group);
	script_file_filter_box->setText (script_file_filter);
	RKCommonFunctions::setTips (i18n ("<p>A list of filters (file name extensions) that should be treated as R script files. Most importantly, files matching one of these filters will always be opened with R syntax highlighting.</p><p>Filters are case insensitive.</p>"), script_file_filter_box, label);
	connect (script_file_filter_box, &QLineEdit::textChanged, this, &RKSettingsModuleCommandEditor::settingChanged);
	form_layout->addRow (i18n ("R script file filters (separated by spaces)"), script_file_filter_box);

	main_vbox->addWidget (group);

	main_vbox->addStretch ();
}

RKSettingsModuleCommandEditor::~RKSettingsModuleCommandEditor () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleCommandEditor::makeCompletionTypeBoxes (const QStringList& labels, QGridLayout* layout) {
	RK_ASSERT (labels.count () == N_COMPLETION_CATEGORIES);
	for (int i = 0; i < N_COMPLETION_CATEGORIES; ++i) {
		QCheckBox *box = new QCheckBox(labels[i]);
		box->setChecked (completion_type_enabled[i]);
		completion_type_enabled_box[i] = box;
		layout->addWidget (completion_type_enabled_box[i], i / 2, i % 2);
		connect (box, &QCheckBox::stateChanged, this, &RKSettingsModuleCommandEditor::settingChanged);
	}
}

void RKSettingsModuleCommandEditor::settingChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModuleCommandEditor::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Script editor"));
}

void RKSettingsModuleCommandEditor::applyChanges () {
	RK_TRACE (SETTINGS);

	auto_completion_enabled = auto_completion_enabled_box->isChecked ();
	auto_completion_min_chars = auto_completion_min_chars_box->intValue ();
	auto_completion_timeout = auto_completion_timeout_box->intValue ();
	for (int i = 0; i < N_COMPLETION_CATEGORIES; ++i) {
		completion_type_enabled[i] = completion_type_enabled_box[i]->isChecked ();
	}
	cursor_navigates_completions = cursor_navigates_completions_box->isChecked ();

	completion_options = 0;
	if (completion_list_member_operator_box->currentIndex () == 0) completion_options += RObject::DollarExpansion;
	if (completion_slot_operator_box->currentIndex () == 1) completion_options += RObject::ExplicitSlotsExpansion;
	if (completion_object_qualification_box->currentIndex () == 2) completion_options += RObject::IncludeEnvirForGlobalEnv | RObject::IncludeEnvirIfNotGlobalEnv;
	else if (completion_object_qualification_box->currentIndex () == 1) completion_options += RObject::IncludeEnvirIfNotGlobalEnv;
	else completion_options += RObject::IncludeEnvirIfMasked;

	autosave_enabled = autosave_enabled_box->isChecked ();
	autosave_keep = autosave_keep_box->isChecked ();
	autosave_interval = autosave_interval_box->intValue ();

	num_recent_files = num_recent_files_box->intValue ();
	script_file_filter = script_file_filter_box->text ();
}

void RKSettingsModuleCommandEditor::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

QString completionTypeToConfigKey (int cat) {
	if (cat == RKSettingsModuleCommandEditor::Calltip) return "Calltips";
	if (cat == RKSettingsModuleCommandEditor::Arghint) return "Argument completion";
	if (cat == RKSettingsModuleCommandEditor::Object) return "Object completion";
	if (cat == RKSettingsModuleCommandEditor::Filename) return "Filename completion";
	if (cat == RKSettingsModuleCommandEditor::AutoWord) return "Auto word completion";
	RK_ASSERT(false);
	return QString ();
}

void RKSettingsModuleCommandEditor::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Command Editor Windows");
	cg.writeEntry ("Completion enabled", auto_completion_enabled);
	cg.writeEntry ("Completion min chars", auto_completion_min_chars);
	cg.writeEntry ("Completion timeout", auto_completion_timeout);
	cg.writeEntry ("Completion option flags", completion_options);
	cg.writeEntry ("Cursor navigate completions", cursor_navigates_completions);
	for (int i = 0; i < N_COMPLETION_CATEGORIES; ++i) {
		cg.writeEntry (completionTypeToConfigKey (i), completion_type_enabled[i]);
	}

	cg.writeEntry ("Autosave enabled", autosave_enabled);
	cg.writeEntry ("Autosave keep saves", autosave_keep);
	cg.writeEntry ("Autosave interval", autosave_interval);

	cg.writeEntry ("Max number of recent files", num_recent_files);
	cg.writeEntry ("Script file filter", script_file_filter);
}

void RKSettingsModuleCommandEditor::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Command Editor Windows");
	auto_completion_enabled = cg.readEntry ("Completion enabled", true);
	auto_completion_min_chars = cg.readEntry ("Completion min chars", 2);
	auto_completion_timeout = cg.readEntry ("Completion timeout", 250);
	completion_options = cg.readEntry ("Completion option flags", (int) RObject::IncludeEnvirIfMasked);
	cursor_navigates_completions = cg.readEntry ("Cursor navigate completions", false);
	for (int i = 0; i < N_COMPLETION_CATEGORIES; ++i) {
		completion_type_enabled[i] = cg.readEntry (completionTypeToConfigKey (i), true);
	}

	autosave_enabled = cg.readEntry ("Autosave enabled", true);
	autosave_keep = cg.readEntry ("Autosave keep saves", false);
	autosave_interval = cg.readEntry ("Autosave interval", 5);

	num_recent_files = cg.readEntry ("Max number of recent files", 10);
	script_file_filter = cg.readEntry ("Script file filter", "*.R *.S *.q *.Rhistory");
}

// static
bool RKSettingsModuleCommandEditor::matchesScriptFileFilter (const QString &filename) {
	RK_TRACE (SETTINGS);

	const QStringList exts = script_file_filter.split (' ');
	foreach (const QString& ext, exts) {
		QRegExp reg (ext, Qt::CaseInsensitive, QRegExp::Wildcard);
		if (reg.exactMatch (filename)) return true;
	}
	return false;
}

