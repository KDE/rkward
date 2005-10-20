/***************************************************************************
                          rksettingsmodulelogfiles  -  description
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
#ifndef RKSETTINGSMODULELOGFILES_H
#define RKSETTINGSMODULELOGFILES_H

#include "rksettingsmodule.h"

class GetFileNameWidget;

/**
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleLogfiles : public RKSettingsModule {
	Q_OBJECT
public:
    RKSettingsModuleLogfiles (RKSettings *gui, QWidget *parent);

    ~RKSettingsModuleLogfiles ();
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();

/// returns the directory-name where the logfiles should reside
	static QString &filesPath () { return files_path; };
public slots:
	void pathChanged ();
private:
	GetFileNameWidget *files_choser;
	
	static QString files_path;
/** since changing the files_path can not easily be done while in an active session, the setting should only take effect on the next start. This string stores a changed setting, while keeping the old one intact as long as RKWard is running */
	static QString new_files_path;
};

#endif

