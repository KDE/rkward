/***************************************************************************
                          rksettingsmodulewatch  -  description
                             -------------------
    begin                : Thu Aug 26 2004
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
#ifndef RKSETTINGSMODULEWATCH_H
#define RKSETTINGSMODULEWATCH_H

#include "rksettingsmodule.h"

class RCommand;
class QCheckBox;
class QGridLayout;

/**
Settings module for the RInterface-watch. Allows you to configure what kind of commands you would like to see/hide.

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleWatch : public RKSettingsModule
{
Q_OBJECT
public:
	RKSettingsModuleWatch (RKSettings *gui, QWidget *parent);

	~RKSettingsModuleWatch ();
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);

	static bool shouldShowInput (RCommand *command);
	static bool shouldShowOutput (RCommand *command);
	static bool shouldShowError (RCommand *command);
	static bool shouldRaiseWindow (RCommand *command);

	QString caption ();
public slots:
	void changedSetting (int);
private:
	enum FilterType { ShowInput=1, ShowOutput=2, ShowError=4, RaiseWindow=8 };

	static int plugin_filter;
	static int app_filter;
	static int sync_filter;
	static int user_filter;
	
	struct FilterBoxes {
		QCheckBox *input;
		QCheckBox *output;
		QCheckBox *error;
		QCheckBox *raise;
	};
	
	FilterBoxes *plugin_filter_boxes;
	FilterBoxes *app_filter_boxes;
	FilterBoxes *sync_filter_boxes;
	FilterBoxes *user_filter_boxes;
	
	int getFilterSettings (FilterBoxes *boxes);
	FilterBoxes *addFilterSettings (QWidget *parent, QGridLayout *layout, int row, const QString &label, int state);
};

#endif
