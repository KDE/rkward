/***************************************************************************
                          rkpluginwidget.h  -  description
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

#ifndef RKPLUGINWIDGET_H
#define RKPLUGINWIDGET_H

#include <qlayout.h>

class QWidget;
class QLabel;
class QDomElement;

/**
  *@author Thomas Friedrichsmeier
  */

class RKPluginWidget : public QBoxLayout {
public: 
	RKPluginWidget(const QDomElement &element, QWidget *parent);
	virtual ~RKPluginWidget();
	QWidget *parent () { return _parent; };
	QLabel *label;
private:
	QWidget *_parent;
};

#endif
