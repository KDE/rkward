/***************************************************************************
                          rkradio.h  -  description
                             -------------------
    begin                : Thu Nov 7 2002
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

#ifndef RKRADIO_H
#define RKRADIO_H

#include <rkpluginwidget.h>

#include <qmap.h>

class QWidget;
class QButtonGroup;
class QDomElement;
class QRadioButton;

/**
  *@author Thomas Friedrichsmeier
  */

class RKRadio : public RKPluginWidget  {
public: 
	RKRadio(const QDomElement &element, QWidget *parent, RKPlugin *plugin);
	~RKRadio();
private:
	QButtonGroup *group;
	typedef QMap<QRadioButton *, QString> OptionsMap;
	OptionsMap options;
protected:
	QString value ();
};

#endif
