/*
rksettingsmodule - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmodule.h"

#include "../rkward.h"
#include "../debug.h"
#include "rksettings.h"
#include "../misc/rkspinbox.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>

#include <functional>

//static
RCommandChain* RKSettingsModule::chain = nullptr;

RKSettingsModule::RKSettingsModule(QObject *parent) : QObject(parent) {
	RK_TRACE(SETTINGS);
}

RKSettingsModule::~RKSettingsModule() {
	RK_TRACE(SETTINGS);
}

RKSettingsModuleWidget::RKSettingsModuleWidget(QWidget *parent, RKSettingsModule *parent_module, const RKSettingsModule::PageId pageid, const RKSettingsModule::PageId superpageid) :
	QWidget(parent),
	changed(false),
	pageid(pageid),
	superpageid(superpageid),
	parent_module(parent_module)
{
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleWidget::change() {
	changed = true;
	Q_EMIT settingsChanged();
}


template<>
QCheckBox* RKConfigValue<bool, bool>::makeCheckbox(const QString& label, RKSettingsModuleWidget* module) {
	QCheckBox *ret = new QCheckBox(label);
	ret->setChecked(value);
	QObject::connect(ret, &QCheckBox::stateChanged, module, &RKSettingsModuleWidget::change);
	QObject::connect(module, &RKSettingsModuleWidget::apply, module, [ret, this]() { this->value = ret->isChecked(); });
	return ret;
}

template<>
QGroupBox* RKConfigValue<bool, bool>::makeCheckableGroupBox(const QString& label, RKSettingsModuleWidget* module) {
	QGroupBox *ret = new QGroupBox(label);
	ret->setCheckable(true);
	ret->setChecked(value);
	QObject::connect(ret, &QGroupBox::toggled, module, &RKSettingsModuleWidget::change);
	QObject::connect(module, &RKSettingsModuleWidget::apply, module, [ret, this]() { this->value = ret->isChecked(); });
	return ret;
}


template<>
QAction* RKConfigValue<bool, bool>::makeAction(QObject *parent, const QString &label, std::function<void(bool)> handler) {
	QAction *ret = new QAction(label, parent);
	ret->setCheckable(true);
	ret->setChecked(value);
	QObject::connect(ret, &QAction::triggered, handler);
	QObject::connect(ret, &QAction::triggered, parent, [this](bool val) { value=val; });
	handler(value);
	return ret;
}

template<>
RKSpinBox* RKConfigValue<double, double>::makeSpinBox(double min, double max, RKSettingsModuleWidget* module) {
	RKSpinBox* ret = new RKSpinBox();
	ret->setRealMode(min, max, value, 1, 2);
	QObject::connect(ret, QOverload<int>::of(&QSpinBox::valueChanged), module, &RKSettingsModuleWidget::change);
	QObject::connect(module, &RKSettingsModuleWidget::apply, module, [ret, this]() { this->value = ret->realValue(); });
	return ret;
}

template<>
RKSpinBox* RKConfigValue<int, int>::makeSpinBox(int min, int max, RKSettingsModuleWidget* module) {
	RKSpinBox* ret = new RKSpinBox();
	ret->setIntMode(min, max, value);
	QObject::connect(ret, QOverload<int>::of(&QSpinBox::valueChanged), module, &RKSettingsModuleWidget::change);
	QObject::connect(module, &RKSettingsModuleWidget::apply, module, [ret, this]() { this->value = ret->intValue(); });
	return ret;
}
// Hmm... Boring dupe of the above
template<>
RKSpinBox* RKConfigValue<uint, uint>::makeSpinBox(uint min, uint max, RKSettingsModuleWidget* module) {
	RKSpinBox* ret = new RKSpinBox();
	ret->setIntMode(min, max, value);
	QObject::connect(ret, QOverload<int>::of(&QSpinBox::valueChanged), module, &RKSettingsModuleWidget::change);
	QObject::connect(module, &RKSettingsModuleWidget::apply, module, [ret, this]() { this->value = ret->intValue(); });
	return ret;
}

QComboBox* RKConfigBase::makeDropDownHelper(const LabelList &entries, RKSettingsModuleWidget* module, int initial, std::function<void(int)> setter) {
	RK_TRACE(SETTINGS);

	QComboBox *ret = new QComboBox();
	int index = -1;
	for (int i = 0; i < entries.size(); ++i) {
		auto key = entries[i].key;
		auto label = entries[i].label;
		ret->addItem(label, key);
		if (key == initial) index = i;
	}
	RK_ASSERT(index >= 0);
	ret->setCurrentIndex(index);
	QObject::connect(ret, QOverload<int>::of(&QComboBox::currentIndexChanged), module, [ret, setter, module]() {
		module->change();
		setter(ret->currentData().toInt());
	});

	return ret;
}

void linkHelperDummy() {
	RKConfigValue<bool>("", true).makeCheckbox(QString(), nullptr);
	RKConfigValue<int>("", 0).makeSpinBox(0, 1, nullptr);
	RKConfigValue<uint>("", 0).makeSpinBox(0, 1, nullptr);
	RKConfigValue<double>("", 0.0).makeSpinBox(0.0, 1.0, nullptr);
}
