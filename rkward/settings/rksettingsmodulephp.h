/***************************************************************************
                          rksettingsmodulephp  -  description
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
#ifndef RKSETTINGSMODULEPHP_H
#define RKSETTINGSMODULEPHP_H

#include "rksettingsmodule.h"

class QPushButton;
class QLineEdit;

/**
configuration for the PHP-backend

@author Thomas Friedrichsmeier
*/
class RKSettingsModulePHP : public RKSettingsModule {
	Q_OBJECT
public:
    RKSettingsModulePHP (RKSettings *gui, QWidget *parent);

    ~RKSettingsModulePHP ();
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();

/// returns the filename of the php-binary
	static QString &phpBin () { return php_bin; };
/// returns the directory-name where supporting files such as the "common.php" reside
	static QString &filesPath () { return files_path; };
public slots:
	void browseBin ();
	void browseFiles ();
	void pathChanged (const QString &);
private:
	QPushButton *bin_browse_button;
	QLineEdit *bin_location_edit;
	
	QPushButton *files_browse_button;
	QLineEdit *files_location_edit;
	
	static QString php_bin;
	static QString files_path;
};

#endif
