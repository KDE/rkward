/***************************************************************************
                          rksettingsmoduler  -  description
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
#ifndef RKSETTINGSMODULER_H
#define RKSETTINGSMODULER_H

#include "rksettingsmodule.h"

#include <qstring.h>
#include <qstringlist.h>

class QPushButton;
class QLineEdit;
class QCheckBox;

/**
Configure the R-backend

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleR : public RKSettingsModule {
	Q_OBJECT
public:
    RKSettingsModuleR (RKSettings *gui, QWidget *parent);

    ~RKSettingsModuleR ();
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();
	
	static QString &rHomeDir () { return r_home_dir; };
	static QString &pagerApp () { return pager_app; };
	static bool rNosave () { return r_nosave; };
	static bool rSlave () { return r_slave; };
	static QStringList getOptionList ();
public slots:
	void browsePager ();
	void pathChanged (const QString &);
	void boxChanged (int);
private:
	QPushButton *pager_browse_button;
	QLineEdit *pager_location_edit;
	QCheckBox *nosave_box;
	QCheckBox *slave_box;

friend class RInterface;
	static bool r_nosave;
	static bool r_slave;
	static QString r_home_dir;
	static QString pager_app;
};

#endif
