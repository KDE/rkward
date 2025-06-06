/*
rksettingsmoduledebug - This file is part of RKWard (https://rkward.kde.org). Created: Tue Oct 23 2007
SPDX-FileCopyrightText: 2007-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmoduledebug.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QTemporaryFile>
#include <QVBoxLayout>
#include <qlabel.h>
#include <qlayout.h>

#include "../misc/rkcommonfunctions.h"
#include "../misc/rkspinbox.h"
#include "../misc/rkstyle.h"
#include "rksettings.h"

#include "../debug.h"

class RKSettingsPageDebug : public RKSettingsModuleWidget {
  public:
	RKSettingsPageDebug(QWidget *parent, RKSettingsModule *parent_module) : RKSettingsModuleWidget(parent, parent_module, RKSettingsModuleDebug::page_id) {
		RK_TRACE(SETTINGS);

		setWindowTitle(i18n("Debug"));

		QVBoxLayout *main_vbox = new QVBoxLayout(this);

		main_vbox->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("<b>These settings are for debugging purposes, only.</b> It is safe to leave them untouched. Also, these settings will only apply to the current session, and will not be saved.")));

		main_vbox->addSpacing(2 * RKStyle::spacingHint());

		QLabel *label = new QLabel(i18n("Debug level"), this);
		debug_level_box = new RKSpinBox(this);
		debug_level_box->setIntMode(DL_TRACE, DL_FATAL, DL_FATAL - RK_Debug::RK_Debug_Level);
		connect(debug_level_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsPageDebug::change);
		main_vbox->addWidget(label);
		main_vbox->addWidget(debug_level_box);

		debug_flags_group = new QButtonGroup(this);
		debug_flags_group->setExclusive(false);
		QGroupBox *group = new QGroupBox(i18n("Debug flags"), this);
		QVBoxLayout *box_layout = new QVBoxLayout(group);

		debug_flags_group->addButton(new QCheckBox(QStringLiteral("APP"), group), APP);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("PLUGIN"), group), PLUGIN);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("OBJECTS"), group), OBJECTS);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("EDITOR"), group), EDITOR);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("SETTINGS"), group), SETTINGS);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("PHP"), group), PHP);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("RBACKEND"), group), RBACKEND);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("COMMANDEDITOR"), group), COMMANDEDITOR);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("MISC"), group), MISC);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("DIALOGS"), group), DIALOGS);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("OUTPUT"), group), OUTPUT);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("XML"), group), XML);
		debug_flags_group->addButton(new QCheckBox(QStringLiteral("GRAPHICS_DEVICE"), group), GRAPHICS_DEVICE);

		QList<QAbstractButton *> buttons = debug_flags_group->buttons();
		for (QList<QAbstractButton *>::const_iterator it = buttons.constBegin(); it != buttons.constEnd(); ++it) {
			box_layout->addWidget(*it);
			(*it)->setChecked(RK_Debug::RK_Debug_Flags & debug_flags_group->id(*it));
		}
		connect(debug_flags_group, &QButtonGroup::idClicked, this, &RKSettingsPageDebug::change);
		main_vbox->addWidget(group);

		label = new QLabel(i18n("Command timeout"), this);
		command_timeout_box = new RKSpinBox(this);
		command_timeout_box->setIntMode(0, 10000, RK_Debug::RK_Debug_CommandStep);
		connect(command_timeout_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsPageDebug::change);
		main_vbox->addWidget(label);
		main_vbox->addWidget(command_timeout_box);

		main_vbox->addStretch();

		if (RK_Debug::debug_file) {
			label = new QLabel(i18n("<i>Note:</i> Debug output is written to %1", RK_Debug::debug_file->fileName()));
			main_vbox->addWidget(label);
			main_vbox->addStretch();
		}
	}
	void applyChanges() override {
		RK_TRACE(SETTINGS);

		RK_Debug::RK_Debug_Level = DL_FATAL - debug_level_box->intValue();
		RK_Debug::RK_Debug_CommandStep = command_timeout_box->intValue();
		int flags = 0;
		QList<QAbstractButton *> buttons = debug_flags_group->buttons();
		for (QList<QAbstractButton *>::const_iterator it = buttons.constBegin(); it != buttons.constEnd(); ++it) {
			if ((*it)->isChecked()) flags |= debug_flags_group->id(*it);
		}
		RK_Debug::RK_Debug_Flags = flags;
	}

  private:
	RKSpinBox *command_timeout_box;
	RKSpinBox *debug_level_box;
	QButtonGroup *debug_flags_group;
};

RKSettingsModuleDebug::RKSettingsModuleDebug(QObject *parent) : RKSettingsModule(parent) {
	RK_TRACE(SETTINGS);
}

RKSettingsModuleDebug::~RKSettingsModuleDebug() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleDebug::createPages(RKSettings *parent) {
	parent->addSettingsPage(new RKSettingsPageDebug(parent, this));
}

void RKSettingsModuleDebug::syncConfig(KConfig *, RKConfigBase::ConfigSyncAction) {
	RK_TRACE(SETTINGS);
	// left empty on purpose
}
