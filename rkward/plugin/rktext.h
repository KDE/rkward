/***************************************************************************
                          rktext.h  -  description
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

#ifndef RKTEXT_H
#define RKTEXT_H

#include <rkpluginwidget.h>

/**
  *@author Thomas Friedrichsmeier
  */

class RKText : public RKPluginWidget  {
public: 
	RKText(const QDomElement &element, QWidget *parent, RKPlugin *plugin);
	~RKText();
};

#endif
