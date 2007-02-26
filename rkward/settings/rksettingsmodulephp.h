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

class GetFileNameWidget;

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
public slots:
	void pathChanged ();
private:
	GetFileNameWidget *bin_choser;

	static QString php_bin;
};

#endif
