/***************************************************************************
                          rsettings.cpp  -  description
                             -------------------
    begin                : Wed Nov 6 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#include "rsettings.h"

#include "rkward.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include <kfiledialog.h>
#include <klocale.h>

RSettings::RSettings(RKwardApp *parent) : RSettingsUi (parent) {
	_parent = parent;
	no_save->setChecked (parent->opt_r_nosave);
	slave->setChecked (parent->opt_r_slave);
	path_to_r->setText (parent->path_to_r);
	connect (restart_r, SIGNAL (clicked ()), this, SLOT (slotReStart ()));
	connect (cancel, SIGNAL (clicked ()), this, SLOT (slotCancel ()));
	connect (browse_for_r, SIGNAL (clicked ()), this, SLOT (slotBrowse ()));
}

RSettings::~RSettings(){
}

void RSettings::slotCancel () {
	_parent->fetchRSettings(this, false);
}

void RSettings::slotReStart () {
	_parent->fetchRSettings(this, true);
}

void RSettings::slotBrowse () {
	QString temp = KFileDialog::getOpenFileName ("/usr/bin", "R", this, i18n ("Select R-executable"));
	if (temp != "") {
		path_to_r->setText (temp);		
	}
}
