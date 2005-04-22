/***************************************************************************
                          rksettingsmodule  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
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

RKSettingsModule::RKSettingsModule(RKSettings *gui, QWidget *parent) : QWidget (parent) {
	changed = false;
	RKSettingsModule::gui = gui;
}

RKSettingsModule::~RKSettingsModule() {
}

void RKSettingsModule::change () {
	changed = true;
	gui->enableApply ();
}
