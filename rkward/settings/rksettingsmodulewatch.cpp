/*
rksettingsmodulewatch - This file is part of RKWard (https://rkward.kde.org). Created: Thu Aug 26 2004
SPDX-FileCopyrightText: 2004-2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmodulewatch.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QGridLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qlabel.h>

#include "../misc/rkcommonfunctions.h"
#include "../misc/rkcompatibility.h"
#include "../misc/rkspinbox.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkstyle.h"
#include "../rbackend/rcommand.h"
#include "rksettings.h"

#include "../debug.h"

// static
RKConfigValue<int> RKSettingsModuleWatch::plugin_filter{"plugin command filter", ShowInput | ShowError};
RKConfigValue<int> RKSettingsModuleWatch::app_filter{"app command filter", ShowInput | ShowError};
RKConfigValue<int> RKSettingsModuleWatch::sync_filter{"sync command filter", ShowError};
RKConfigValue<int> RKSettingsModuleWatch::user_filter{"user command filter", ShowInput | ShowOutput | ShowError | RaiseWindow};
RKConfigValue<uint> RKSettingsModuleWatch::max_log_lines{"max log lines", 1000};
int RKSettingsModuleWatch::all_suppressed = 0;

// static
bool RKSettingsModuleWatch::shouldShowInput(RCommand *command) {
	RK_TRACE(SETTINGS);
	if (all_suppressed) return false;
	if (command->type() & RCommand::EmptyCommand) return false;

	if (command->type() & (RCommand::Sync | RCommand::PriorityCommand)) {
		return (sync_filter & ShowInput);
	} else if (command->type() & RCommand::User) {
		return (user_filter & ShowInput);
	} else if (command->type() & RCommand::Plugin) {
		return (plugin_filter & ShowInput);
	} else if (command->type() & RCommand::App) {
		return (app_filter & ShowInput);
	}

	RK_ASSERT(false);
	return true;
}

// static
bool RKSettingsModuleWatch::shouldShowOutput(RCommand *command) {
	RK_TRACE(SETTINGS);

	if (all_suppressed) return false;
	if (command->type() & RCommand::EmptyCommand) return false;
	if (!shouldShowInput(command)) return false;

	if (command->type() & (RCommand::Sync | RCommand::PriorityCommand)) {
		return (sync_filter & ShowOutput);
	} else if (command->type() & RCommand::User) {
		return (user_filter & ShowOutput);
	} else if (command->type() & RCommand::Plugin) {
		return (plugin_filter & ShowOutput);
	} else if (command->type() & RCommand::App) {
		return (app_filter & ShowOutput);
	}

	RK_ASSERT(false);
	return true;
}

// static
bool RKSettingsModuleWatch::shouldShowError(RCommand *command) {
	RK_TRACE(SETTINGS);

	if (all_suppressed) return false;
	if (command->type() & (RCommand::Sync | RCommand::PriorityCommand)) {
		return (sync_filter & ShowError);
	} else if (command->type() & RCommand::User) {
		return (user_filter & ShowError);
	} else if (command->type() & RCommand::Plugin) {
		return (plugin_filter & ShowError);
	} else if (command->type() & RCommand::App) {
		return (app_filter & ShowError);
	}

	RK_ASSERT(false);
	return true;
}

bool RKSettingsModuleWatch::shouldRaiseWindow(RCommand *command) {
	RK_TRACE(SETTINGS);

	if (all_suppressed) return false;
	if (command->type() & (RCommand::Sync | RCommand::PriorityCommand)) {
		return (sync_filter & RaiseWindow);
	} else if (command->type() & RCommand::User) {
		return (user_filter & RaiseWindow);
	} else if (command->type() & RCommand::Plugin) {
		return (plugin_filter & RaiseWindow);
	} else if (command->type() & RCommand::App) {
		return (app_filter & RaiseWindow);
	}

	RK_ASSERT(false);
	return true;
}

class RKSettingsPageWatch : public RKSettingsModuleWidget {
  public:
	RKSettingsPageWatch(QWidget *parent, RKSettingsModule *parent_module) : RKSettingsModuleWidget(parent, parent_module, RKSettingsModuleWatch::page_id) {
		RK_TRACE(SETTINGS);

		setWindowTitle(i18n("Command log"));
		setWindowIcon(RKStandardIcons::getIcon(RKStandardIcons::WindowCommandLog));

		QVBoxLayout *vbox = new QVBoxLayout(this);

		QLabel *label = RKCommonFunctions::wordWrappedLabel(i18n("For now, settings only apply to new commands. All previous commands remain visible/invisible."));
		vbox->addWidget(label);
		vbox->addSpacing(10);

		QGridLayout *grid = new QGridLayout();
		vbox->addLayout(grid);

		label = RKCommonFunctions::wordWrappedLabel(i18n("always show command"));
		grid->addWidget(label, 0, 1);
		label = RKCommonFunctions::wordWrappedLabel(i18n("always show result"));
		grid->addWidget(label, 0, 2);
		label = RKCommonFunctions::wordWrappedLabel(i18n("show errors"));
		grid->addWidget(label, 0, 3);
		label = RKCommonFunctions::wordWrappedLabel(i18n("show/raise window"));
		grid->addWidget(label, 0, 4);

		user_filter_boxes = addFilterSettings(grid, 1, i18n("User commands"), RKSettingsModuleWatch::user_filter);
		plugin_filter_boxes = addFilterSettings(grid, 2, i18n("Plugin generated commands"), RKSettingsModuleWatch::plugin_filter);
		app_filter_boxes = addFilterSettings(grid, 3, i18n("Application commands"), RKSettingsModuleWatch::app_filter);
		sync_filter_boxes = addFilterSettings(grid, 4, i18n("Synchronization commands"), RKSettingsModuleWatch::sync_filter);

		vbox->addSpacing(2 * RKStyle::spacingHint());

		vbox->addWidget(new QLabel(i18n("Maximum number of paragraphs/lines to display in the Command Log (0 for no limit)")));
		vbox->addWidget(RKSettingsModuleWatch::max_log_lines.makeSpinBox(0, INT_MAX, this));

		vbox->addStretch();

		validateGUI();
	}
	void changedSetting(int) {
		RK_TRACE(SETTINGS);
		validateGUI();
		change();
	}
	void validateGUI() {
		RK_TRACE(SETTINGS);

		user_filter_boxes.output->setEnabled(user_filter_boxes.input->isChecked());
		plugin_filter_boxes.output->setEnabled(plugin_filter_boxes.input->isChecked());
		app_filter_boxes.output->setEnabled(app_filter_boxes.input->isChecked());
		sync_filter_boxes.output->setEnabled(sync_filter_boxes.input->isChecked());
	}
	void applyChanges() override {
		RK_TRACE(SETTINGS);

		RKSettingsModuleWatch::user_filter = getFilterSettings(&user_filter_boxes);
		RKSettingsModuleWatch::plugin_filter = getFilterSettings(&plugin_filter_boxes);
		RKSettingsModuleWatch::app_filter = getFilterSettings(&app_filter_boxes);
		RKSettingsModuleWatch::sync_filter = getFilterSettings(&sync_filter_boxes);
	}

  private:
	struct FilterBoxes {
		QCheckBox *input;
		QCheckBox *output;
		QCheckBox *error;
		QCheckBox *raise;
	};

	FilterBoxes plugin_filter_boxes;
	FilterBoxes app_filter_boxes;
	FilterBoxes sync_filter_boxes;
	FilterBoxes user_filter_boxes;

	int getFilterSettings(FilterBoxes *boxes) {
		RK_TRACE(SETTINGS);

		int ret = 0;
		if (boxes->input->isChecked()) ret |= RKSettingsModuleWatch::ShowInput;
		if (boxes->output->isChecked()) ret |= RKSettingsModuleWatch::ShowOutput;
		if (boxes->error->isChecked()) ret |= RKSettingsModuleWatch::ShowError;
		if (boxes->raise->isChecked()) ret |= RKSettingsModuleWatch::RaiseWindow;
		return ret;
	}
	FilterBoxes addFilterSettings(QGridLayout *layout, int row, const QString &label, int state) {
		RK_TRACE(SETTINGS);

		FilterBoxes filter_boxes;

		layout->addWidget(new QLabel(label), row, 0);

		filter_boxes.input = new QCheckBox();
		filter_boxes.input->setChecked(state & RKSettingsModuleWatch::ShowInput);
		connect(filter_boxes.input, RKCompatibility_QCheckBox_checkStateChanged, this, &RKSettingsPageWatch::changedSetting);
		layout->addWidget(filter_boxes.input, row, 1);

		filter_boxes.output = new QCheckBox();
		filter_boxes.output->setChecked(state & RKSettingsModuleWatch::ShowOutput);
		connect(filter_boxes.output, RKCompatibility_QCheckBox_checkStateChanged, this, &RKSettingsPageWatch::changedSetting);
		layout->addWidget(filter_boxes.output, row, 2);

		filter_boxes.error = new QCheckBox();
		filter_boxes.error->setChecked(state & RKSettingsModuleWatch::ShowError);
		connect(filter_boxes.error, RKCompatibility_QCheckBox_checkStateChanged, this, &RKSettingsPageWatch::changedSetting);
		layout->addWidget(filter_boxes.error, row, 3);

		filter_boxes.raise = new QCheckBox();
		filter_boxes.raise->setChecked(state & RKSettingsModuleWatch::RaiseWindow);
		connect(filter_boxes.raise, RKCompatibility_QCheckBox_checkStateChanged, this, &RKSettingsPageWatch::changedSetting);
		layout->addWidget(filter_boxes.raise, row, 4);

		return filter_boxes;
	}
};

RKSettingsModuleWatch *RKSettingsModuleWatch::_instance = nullptr;
RKSettingsModuleWatch::RKSettingsModuleWatch(QObject *parent) : RKSettingsModule(parent) {
	RK_TRACE(SETTINGS);
	_instance = this;
}

RKSettingsModuleWatch::~RKSettingsModuleWatch() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleWatch::createPages(RKSettings *parent) {
	parent->addSettingsPage(new RKSettingsPageWatch(parent, this));
}

// static
void RKSettingsModuleWatch::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group(QStringLiteral("RInterface Watch Settings"));
	user_filter.syncConfig(cg, a);
	plugin_filter.syncConfig(cg, a);
	app_filter.syncConfig(cg, a);
	sync_filter.syncConfig(cg, a);
	max_log_lines.syncConfig(cg, a);
}
