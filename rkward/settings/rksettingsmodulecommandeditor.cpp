/*
rksettingsmodulecommandeditor - This file is part of RKWard (https://rkward.kde.org). Created: Tue Oct 23 2007
SPDX-FileCopyrightText: 2007-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmodulecommandeditor.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <KTextEditor/Editor>
#include <KTextEditor/ConfigPage>

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
#include "../misc/rkstandardicons.h"
#include "../core/robject.h"
#include "../misc/rkstyle.h"

#include "../debug.h"

// static members
RKCodeCompletionSettings RKSettingsModuleCommandEditor::completion_settings;
RKConfigValue<bool> RKSettingsModuleCommandEditor::autosave_enabled { "Autosave enabled", true };
RKConfigValue<bool> RKSettingsModuleCommandEditor::autosave_keep { "Autosave keep saves", false };
RKConfigValue<int> RKSettingsModuleCommandEditor::autosave_interval {"Autosave interval", 5 };
RKConfigValue<QString> RKSettingsModuleCommandEditor::script_file_filter { "Script file filter", "*.R *.S *.q *.Rhistory" };

RKCodeCompletionSettingsWidget::RKCodeCompletionSettingsWidget(QWidget *parent, RKSettingsModule *module, RKCodeCompletionSettings *settings, bool show_common) : RKSettingsModuleWidget(parent, module), settings(settings) {
	RK_TRACE (SETTINGS);
	QVBoxLayout* main_vbox = new QVBoxLayout (this);
	main_vbox->setContentsMargins(0,0,0,0);

	QGroupBox* group = new QGroupBox (i18n ("Code Completion / Code Hints"), this);
	QVBoxLayout* box_layout = new QVBoxLayout (group);

	QGridLayout *g_layout = new QGridLayout ();
	box_layout->addLayout (g_layout);
	makeCompletionTypeBoxes (QStringList () << i18n ("Function call tip") << i18n ("Function argument completion") << i18n ("Object name completion") << i18n ("Filename completion") << i18n ("Auto word completion") << i18n("Mouseover object info"), g_layout);

	auto_completion_enabled_box = new QGroupBox (i18n ("Start code completions/hints, automatically"), group);
	auto_completion_enabled_box->setCheckable (true);
	auto_completion_enabled_box->setChecked (settings->auto_completion_enabled);
	connect (auto_completion_enabled_box, &QGroupBox::toggled, this, &RKCodeCompletionSettingsWidget::change);
	box_layout->addWidget (auto_completion_enabled_box);

	QFormLayout* form_layout = new QFormLayout (auto_completion_enabled_box);
	auto auto_completion_min_chars_box = settings->auto_completion_min_chars.makeSpinBox(1, INT_MAX, this);
	form_layout->addRow (i18n("Minimum number of characters"), auto_completion_min_chars_box);

	auto auto_completion_timeout_box = settings->auto_completion_timeout.makeSpinBox(0, INT_MAX, this);
	form_layout->addRow (i18n ("Timeout (milliseconds)"), auto_completion_timeout_box);

	form_layout->addRow(i18n("(Attempt to) start completion whenever the cursor position changes"), settings->auto_completion_cursor_activated.makeCheckbox(QString(), this));

	form_layout = new QFormLayout ();
	box_layout->addLayout (form_layout);

	auto tabkey_invokes_completion_box = settings->tabkey_invokes_completion.makeCheckbox(QString(), this);
	RKCommonFunctions::setTips(i18n("Note: Further shortcuts can be assigned, and by default, Ctlr+Space invokes completions, in addition to this. Further, pressing the Tab key, while completions are shown, performs partial completion (if possible), independent of this setting."), tabkey_invokes_completion_box);
	form_layout->addRow(i18n("Tab key invokes code completion"), tabkey_invokes_completion_box);

	auto cursor_navigates_completions_box = settings->cursor_navigates_completions.makeDropDown(RKConfigBase::LabelList(
		{{1, i18n("Up/down cursor keys")}, {0, i18n("Alt+Up/down cursor keys")}}
	), this);
	RKCommonFunctions::setTips (i18n ("If you wish to avoid ambiguity between navigation completion options and navigating the document, you can set the behavior such that completion items are navigate using Alt+up / Alt+down, instead of just the up/down cursor keys."), cursor_navigates_completions_box);
	form_layout->addRow (i18n ("Keyboard navigation of completion items"), cursor_navigates_completions_box);

	if (show_common) {
		auto completion_list_member_operator_box = settings->completion_options.makeDropDown(RKConfigBase::LabelList(
			{{RObject::DollarExpansion, i18n("'$'-operator (list$member)")}, {0, i18n("'[['-operator (list[[\"member\"]])")}}
		), this, RObject::DollarExpansion);
		form_layout->addRow (i18nc ("Note: list() and data.frame() are programming terms in R, and should not be translated, here", "Operator for access to members of list() and data.frame() objects"), completion_list_member_operator_box);

		auto completion_slot_operator_box = settings->completion_options.makeDropDown(RKConfigBase::LabelList(
			{{0, i18n("'@'-operator (object@member)")}, {RObject::ExplicitSlotsExpansion, i18n("'slot()'-function (slot(object, member))")}}
		), this, RObject::ExplicitSlotsExpansion);
		form_layout->addRow (i18nc ("Note: S4-slot() is a programming term in R, and should not be translated, here", "Operator for access to S4-slot()s"), completion_slot_operator_box);

		auto completion_object_qualification_box = settings->completion_options.makeDropDown(RKConfigBase::LabelList(
			{{RObject::IncludeEnvirIfMasked, i18n("For masked objects, only")}, {RObject::IncludeEnvirIfNotGlobalEnv, i18n("For objects outside of <i>.GlobalEnv</i>, only")}, {RObject::IncludeEnvirIfNotGlobalEnv | RObject::IncludeEnvirForGlobalEnv, i18n("Always")}}
		), this, RObject::IncludeEnvirIfNotGlobalEnv | RObject::IncludeEnvirForGlobalEnv | RObject::IncludeEnvirIfMasked);
		form_layout->addRow (i18n ("Include environment for objects on the search path:"), completion_object_qualification_box);

		auto completion_all_filetypes = settings->completion_all_filetypes.makeCheckbox(QString(), this);
		form_layout->addRow(i18n("Offer completions in all file types (not just R scripts)"), completion_all_filetypes);
	} else {
		box_layout->addWidget(RKCommonFunctions::linkedWrappedLabel(i18n("<b>Note: </b>Additional (common) completion options are available at the <a href=\"rkward://settings/editor\">script editor settings</a>")));
	}

	main_vbox->addWidget(group);
}

void RKCodeCompletionSettingsWidget::applyChanges() {
	settings->auto_completion_enabled = auto_completion_enabled_box->isChecked ();
}

void RKCodeCompletionSettingsWidget::makeCompletionTypeBoxes(const QStringList& labels, QGridLayout* layout) {
	RK_ASSERT(labels.count() == RKCodeCompletionSettings::N_COMPLETION_CATEGORIES);
	for (int i = 0; i < RKCodeCompletionSettings::N_COMPLETION_CATEGORIES; ++i) {
		auto *box = settings->completion_type_enabled[i].makeCheckbox(labels[i], this);
		layout->addWidget(box, i / 2, i % 2);
	}
}

RKSettingsModuleCommandEditor::RKSettingsModuleCommandEditor (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout* main_vbox = new QVBoxLayout (this);
	main_vbox->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("Settings marked with (*) do not take effect until you restart RKWard")));
	main_vbox->addSpacing (2 * RKStyle::spacingHint ());

	main_vbox->addWidget (completion_settings_widget = new RKCodeCompletionSettingsWidget (this, this, &completion_settings, true));

	main_vbox->addSpacing (2 * RKStyle::spacingHint ());

	QGroupBox *group = autosave_enabled_box = new QGroupBox (i18n ("Autosaves"), this);
	autosave_enabled_box->setCheckable (true);
	autosave_enabled_box->setChecked (autosave_enabled);
	connect (autosave_enabled_box, &QGroupBox::toggled, this, &RKSettingsModule::change);
	QFormLayout *form_layout = new QFormLayout (group);

	form_layout->addRow(i18n("Autosave interval (minutes)"), autosave_interval.makeSpinBox(1, INT_MAX, this));

	form_layout->addRow(autosave_keep.makeCheckbox(i18n("Keep autosave file after manual save"), this));

	main_vbox->addWidget (group);

	main_vbox->addSpacing (2 * RKStyle::spacingHint ());

	script_file_filter_box = new QLineEdit();
	script_file_filter_box->setText(script_file_filter);
	RKCommonFunctions::setTips(i18n("<p>A list of filters (file name extensions) that should be treated as R script files. Most importantly, files matching one of these filters will always be opened with R syntax highlighting.</p><p>Filters are case insensitive.</p>"), script_file_filter_box);
	connect(script_file_filter_box, &QLineEdit::textChanged, this, &RKSettingsModule::change);
	main_vbox->addWidget(new QLabel(i18n("R script file filters (separated by spaces)")));
	main_vbox->addWidget(script_file_filter_box);

	main_vbox->addStretch ();
}

RKSettingsModuleCommandEditor::~RKSettingsModuleCommandEditor () {
	RK_TRACE (SETTINGS);
}

QString RKSettingsModuleCommandEditor::caption() const {
	RK_TRACE(SETTINGS);
	return(i18n("Script editor"));
}

QIcon RKSettingsModuleCommandEditor::icon() const {
	RK_TRACE(SETTINGS);
	return RKStandardIcons::getIcon(RKStandardIcons::WindowCommandEditor);
}

void RKSettingsModuleCommandEditor::applyChanges () {
	RK_TRACE (SETTINGS);

	completion_settings_widget->applyChanges ();
	autosave_enabled = autosave_enabled_box->isChecked ();
	script_file_filter = script_file_filter_box->text ();
}

QString completionTypeToConfigKey (int cat) {
	if (cat == RKCodeCompletionSettings::Calltip) return "Calltips";
	if (cat == RKCodeCompletionSettings::Arghint) return "Argument completion";
	if (cat == RKCodeCompletionSettings::Object) return "Object completion";
	if (cat == RKCodeCompletionSettings::Filename) return "Filename completion";
	if (cat == RKCodeCompletionSettings::AutoWord) return "Auto word completion";
	RK_ASSERT(false);
	return QString ();
}

void RKSettingsModuleCommandEditor::syncConfig(KConfig* config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group("Command Editor Windows");
	completion_settings.syncConfig(cg, a);

	autosave_enabled.syncConfig(cg, a);
	autosave_keep.syncConfig(cg, a);
	autosave_interval.syncConfig(cg, a);

	script_file_filter.syncConfig(cg, a);
}

// static
bool RKSettingsModuleCommandEditor::matchesScriptFileFilter (const QString &filename) {
	RK_TRACE (SETTINGS);

	const QStringList exts = script_file_filter.get().split(' ');
	for (const QString& ext : exts) {
		auto reg = QRegularExpression::fromWildcard(ext, Qt::CaseInsensitive);
		if (reg.match (filename).hasMatch()) return true;
	}
	return false;
}


QList<RKSettingsModuleKTextEditorConfigWrapper *> RKSettingsModuleCommandEditor::kateConfigPages(RKSettings* gui, QWidget* parent) {
	RK_TRACE(SETTINGS);

	QList<RKSettingsModuleKTextEditorConfigWrapper *> ret;
	auto ed = KTextEditor::Editor::instance();
	int n = ed->configPages();
	for (int i = 0; i < n; ++i) {
		ret.append(new RKSettingsModuleKTextEditorConfigWrapper(gui, parent, ed->configPage(i, parent)));
	}
	return ret;
}


RKSettingsModuleKTextEditorConfigWrapper::RKSettingsModuleKTextEditorConfigWrapper(RKSettings* gui, QWidget* parent, KTextEditor::ConfigPage* wrapped) : RKSettingsModule(gui, parent), page(wrapped) {
	RK_TRACE(SETTINGS);
	auto vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0,0,0,0);
	vbox->addWidget(wrapped);
	connect(wrapped, &KTextEditor::ConfigPage::changed, this, &RKSettingsModuleKTextEditorConfigWrapper::change);
}

RKSettingsModuleKTextEditorConfigWrapper::~RKSettingsModuleKTextEditorConfigWrapper() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleKTextEditorConfigWrapper::applyChanges() {
	page->apply();
}

QString RKSettingsModuleKTextEditorConfigWrapper::caption() const {
	return page->name();
}

QString RKSettingsModuleKTextEditorConfigWrapper::longCaption() const {
	return page->fullName();
}

QIcon RKSettingsModuleKTextEditorConfigWrapper::icon() const {
	return page->icon();
}
