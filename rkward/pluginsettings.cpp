/***************************************************************************
                          pluginsettings.cpp  -  description
                             -------------------
    begin                : Sun Nov 10 2002
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

#include "pluginsettings.h"

#include <qstring.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include <kfiledialog.h>
#include <klocale.h>

#include "rkward.h"

PluginSettings::PluginSettings(RKwardApp *parent, const char *name ) : PluginSettingsUi(parent,name) {
	_parent = parent;
	pathEdit->setText (_parent->plugin_dir);
	connect (loadButton, SIGNAL (clicked ()), this, SLOT (slotReLoad ()));
	connect (cancelButton, SIGNAL (clicked ()), this, SLOT (slotCancel ()));
	connect (browseButton, SIGNAL (clicked ()), this, SLOT (slotBrowse ()));
}

PluginSettings::~PluginSettings(){
}

void PluginSettings::slotCancel () {
	_parent->fetchPluginSettings(this, false);
}

void PluginSettings::slotReLoad () {
	_parent->fetchPluginSettings(this, true);
}

void PluginSettings::slotBrowse () {
	QString temp = KFileDialog::getExistingDirectory (pathEdit->text (), this, i18n ("Select Plugin-directory"));
	if (temp != "") {
		pathEdit->setText (temp);
	}
}
