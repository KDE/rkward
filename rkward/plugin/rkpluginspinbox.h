/***************************************************************************
                          rkpluginspinbox  -  description
                             -------------------
    begin                : Wed Aug 11 2004
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
#ifndef RKPLUGINSPINBOX_H
#define RKPLUGINSPINBOX_H
#define SPINBOX_WIDGET 3 // comme ma note d'interro

#include <rkpluginwidget.h>

class RKSpinBox;

/**
@author Thomas Friedrichsmeier
*/
class RKPluginSpinBox : public RKPluginWidget {
	Q_OBJECT
public:
	RKPluginSpinBox(const QDomElement &element, QWidget *parent, RKPlugin *plugin);

	~RKPluginSpinBox();
	QString value (const QString &modifier);
  int type() {return SPINBOX_WIDGET ;} ;
  void setEnabled(bool);
public slots:
	void valueChanged (int);
  void active();
  void active(bool);
  
private:
	RKSpinBox *spinbox;
  QString depend;
};

#endif
