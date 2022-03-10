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
#include "rksettings.h"

#include <QCheckBox>

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
	settingsChanged();
}


template<>
template<typename TT, typename std::enable_if<std::is_same<TT, bool>::value>::type* = nullptr> QCheckBox* RKConfigValue<bool>::makeCheckbox(const QString& label, RKSettingsModuleWidget* module) {
	QCheckBox *ret = new QCheckBox(label);
	ret->setChecked(value);
	QObject::connect(ret, &QCheckBox::stateChanged, module, &RKSettingsModuleWidget::change);
	QObject::connect(module, &RKSettingsModuleWidget::apply, [ret, this]() { this->value = ret->isChecked(); });
	return ret;
}

void linkHelperDummy() {
	RKConfigValue<bool>("", true).makeCheckbox(QString(), nullptr);
}
