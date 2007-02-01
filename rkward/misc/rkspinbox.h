/***************************************************************************
                          rkspinbox  -  description
                             -------------------
    begin                : Wed Aug 11 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
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
#ifndef RKSPINBOX_H
#define RKSPINBOX_H

#include <qspinbox.h>
#include <qstring.h>

class QValidator;

/** After constructing use one of setRealMode or setIntMode to initialize the Spinbox.
@author Thomas Friedrichsmeier
*/
class RKSpinBox : public QSpinBox {
	Q_OBJECT
public:
	RKSpinBox (QWidget *parent = 0);
	~RKSpinBox ();
	
	void setRealMode (double min, double max, double initial, int default_precision, int max_precision);
	void setIntMode (int min, int max, int initial);
	double realValue () { return real_value; };
	void setRealValue (double new_value);
protected:
	void updateDisplay ();
	void interpretText ();
private:
	enum Mode { Integer=0, Real=1 };
	Mode mode;
	bool updating;
	bool updating_b;
	double real_value;
	double real_min;
	double real_max;
	int default_precision;
	QValidator *validator;
};

#endif
