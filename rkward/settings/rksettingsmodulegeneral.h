/***************************************************************************
                          rksettingsmodulegeneral  -  description
                             -------------------
    begin                : Fri Jul 30 2004
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
#ifndef RKSETTINGSMODULEGENERALFILES_H
#define RKSETTINGSMODULEGENERALFILES_H

#include "rksettingsmodule.h"
#include "../dialogs/startupdialog.h"

class GetFileNameWidget;
class QComboBox;
class QButtonGroup;

/**
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleGeneral : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleGeneral (RKSettings *gui, QWidget *parent);

	~RKSettingsModuleGeneral ();

	enum WorkplaceSaveMode {	// don't change the int values of this enum, or you'll ruin users saved settings. Append new values at the end
		SaveWorkplaceWithWorkspace=0,
		SaveWorkplaceWithSession=1,
		DontSaveWorkplace=2
	};

	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();

/// returns the directory-name where the logfiles should reside
	static QString &filesPath () { return files_path; };
	static StartupDialog::Result startupAction () { return startup_action; };
	static void setStartupAction (StartupDialog::Result action) { startup_action = action; };
	static WorkplaceSaveMode workplaceSaveMode () { return workplace_save_mode; };
/** retrieve the saved workplace description. Meaningful only is workplaceSaveMode () == SaveWorkplaceWithSession */
	static QString getSavedWorkplace (KConfig *config);
/** set the saved workplace description. Meaningful only is workplaceSaveMode () == SaveWorkplaceWithSession */
	static void setSavedWorkplace (const QString &description, KConfig *config);
public slots:
	void pathChanged ();
	void boxChanged (int);
private:
	GetFileNameWidget *files_choser;
	QComboBox *startup_action_choser;
	QButtonGroup *workplace_save_chooser;

	static StartupDialog::Result startup_action;
	static QString files_path;
/** since changing the files_path can not easily be done while in an active session, the setting should only take effect on the next start. This string stores a changed setting, while keeping the old one intact as long as RKWard is running */
	static QString new_files_path;
	static WorkplaceSaveMode workplace_save_mode;
};

#endif

