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
#ifndef RKSETTINGSMODULE_H
#define RKSETTINGSMODULE_H

#include <qstring.h>
#include <qwidget.h>

class KConfig;
class RKwardApp;
class RKSettings;

/**
Base class for settings modules. Provides some pure virtual calls.

@author Thomas Friedrichsmeier
*/
class RKSettingsModule : public QWidget {
	Q_OBJECT
public:
    RKSettingsModule(RKSettings *gui, RKwardApp *parent);

    ~RKSettingsModule();

	virtual bool hasChanges () = 0;
	virtual void applyChanges () = 0;
	virtual void save (KConfig *config) = 0;
	
	virtual QString caption () = 0;
protected:
	void change ();

	bool changed;
	RKwardApp *rk;
private:
	RKSettings *gui;
};

#endif
