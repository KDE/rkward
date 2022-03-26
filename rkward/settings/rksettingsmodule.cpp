/***************************************************************************
                          rksettingsmodule  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004-2022 by Thomas Friedrichsmeier
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
#include "rksettingsmodule.h"

#include "../rkward.h"
#include "../debug.h"
#include "rksettings.h"
#include "../misc/rkspinbox.h"

#include <QCheckBox>
#include <QComboBox>

#include <functional>

//static
RCommandChain* RKSettingsModule::chain = 0;

RKSettingsModule::RKSettingsModule(RKSettings *gui, QWidget *parent) : RKSettingsModuleWidget (parent, nullptr) {
	connect(this, &RKSettingsModule::settingsChanged, gui, &RKSettings::enableApply);
}

RKSettingsModule::~RKSettingsModule() {
}

RKSettingsModuleWidget::RKSettingsModuleWidget(QWidget *parent, RKSettingsModule *parent_module) : QWidget(parent), changed(false) {
	if (parent_module) {
		connect(this, &RKSettingsModuleWidget::settingsChanged, parent_module, &RKSettingsModuleWidget::change);
		connect(parent_module, &RKSettingsModule::apply, this, &RKSettingsModuleWidget::doApply);
	}
}

void RKSettingsModuleWidget::change() {
	changed = true;
	emit settingsChanged();
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
