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

#include <QSpinBox>
#include <qstring.h>

class QValidator;

/** A Spinbox that can operate in either integer or real number mode. Step-sizes are adjusted automatically according to the current value

After constructing use one of setRealMode or setIntMode to initialize the Spinbox.

@author Thomas Friedrichsmeier
*/
class RKSpinBox : public QSpinBox {
	Q_OBJECT
public:
/** constructor. Use on of setIntMode or setRealMode to initialize the Spinbox! */
	RKSpinBox (QWidget *parent = 0);
/** dtor */
	~RKSpinBox ();

/** set the spinbox to operate on real numbers. When operating on real numbers, QSpinBox::value () is absolutely meaningless! Use realValue () to retrieve the value in this case
@param min minimum acceptable value
@param max maximum acceptable value
@param initial initial value
@param default_precision default precision of steps. E.g. 2 to make the second sub-decimal digit the one that is change by pressing up/down arrows, when the value is 0
@param max_precison maximum acceptable precision */
	void setRealMode (double min, double max, double initial, int default_precision, int max_precision);
/** set the spinbox to operate on integer numbers (like a regular QSpinBox, but step sizes are adjusted automatically
@param min minimum acceptable value
@param max maximum acceptable value
@param initial initial value */
	void setIntMode (int min, int max, int initial);
/** Only meaningful, when in real mode! Returns the current value
@returns the value if in real mode */
	double realValue () { return real_value; };
/** Only meaningful, when in int mode! Returns the current value
@returns the value if in int mode */
	int intValue () { return int_value; };
/** Only meaningful, when in real mode! Sets the new value
@param new_value the new value */
	void setRealValue (double new_value);
/** Only meaningful, when in int mode! Sets the new value
@param new_value the new value */
	void setIntValue (int int_value);
protected:
/** reimplemented from QSpinBox to always return the text suitable for the *internal* value. The value given as parameter is ignored. */
	QString textFromValue (int) const;
/** reimplemented from QSpinBox to update steps and value whenever the text was changed. WARNING: not really const */
	int valueFromText (const QString & text) const;
/** reimplemented from QSpinBox to adjust the internal value */
	void stepBy (int steps);
	QValidator::State validate (QString &input, int &pos ) const;
private slots:
	void updateValue (int value);
	void emitValueChange ();
private:
	enum Mode { Integer=0, Real=1 };
	Mode mode;
	double real_value;
	double real_min;
	double real_max;
	int int_value;
	int int_min;
	int int_max;
	int default_precision;
	QValidator *validator;
};

#endif
