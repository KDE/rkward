/***************************************************************************
                          rkcheckbox  -  description
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
#ifndef RKCHECKBOX_H
#define RKCHECKBOX_H

#include <rkpluginwidget.h>

#include <qstring.h>

#define CHECKBOX_WIDGET 51 // comme le pastis


class QCheckBox;


/**
This RKPluginWidget provides a checkbox

@author Thomas Friedrichsmeier
*/
class RKCheckBox : public RKPluginWidget  {
	Q_OBJECT
public: 
	RKCheckBox (const QDomElement &element, QWidget *parent, RKPlugin *plugin);
	~RKCheckBox ();
	int type() {return CHECKBOX_WIDGET ; };
//  bool isOk ;
	void setEnabled(bool);
public slots:
	void changedState (int);
	void slotActive();
	void slotActive(bool);

  
private:
	QCheckBox *checkbox;
	QString value_if_checked;
	QString value_if_unchecked;
	QString depend;
protected:
/** Returns the value of the currently selected option. */
	QString value (const QString &modifier);

signals: // Signals
	void clicked();
};

#endif
