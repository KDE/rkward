/***************************************************************************
                          rsettings.h  -  description
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

#ifndef RSETTINGS_H
#define RSETTINGS_H

#include <rsettingsui.h>

#include <qstring.h>

/**
  *@author Thomas Friedrichsmeier
  */

class RKwardApp;

class RSettings : public RSettingsUi  {
	Q_OBJECT
public: 
	RSettings(RKwardApp *parent);
	~RSettings();
public slots:
	void slotCancel ();
	void slotReStart ();
	void slotBrowse ();
private:
	RKwardApp *_parent;
};

#endif
