/***************************************************************************
                          rksettingsmodulekateplugins  -  description
                             -------------------
    begin                : Thu Mar 26 2010
    copyright            : (C) 2020 by Thomas Friedrichsmeier
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
#ifndef RKSETTINGSMODULEKATEPLUGINS_H
#define RKSETTINGSMODULEKATEPLUGINS_H

#include "rksettingsmodule.h"

#include <QStringList>

class QTreeWidget;

/** The settings-module for kate plugin specific settings
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleKatePlugins : public RKSettingsModule {
public:
	RKSettingsModuleKatePlugins(RKSettings *gui, QWidget *parent);
	~RKSettingsModuleKatePlugins();

	void applyChanges() override;
	void save(KConfig *config) override;

	static void saveSettings(KConfig *config);
	static void loadSettings(KConfig *config);
	static void validateSettingsInteractive(QList<RKSettingsWizardPage*>*) {};

	QString caption() override;

	static QStringList pluginsToLoad() { return plugins_to_load; };
	static void loadPlugins();
private:
	QTreeWidget *plugin_table;

	static QStringList plugins_to_load;
};

#endif
