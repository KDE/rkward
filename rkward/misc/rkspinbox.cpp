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
#include "rkspinbox.h"

#include <qvalidator.h>
#include <qlineedit.h>

#include <math.h>

#include "../debug.h"

RKSpinBox::RKSpinBox (QWidget *parent) : QSpinBox (parent) {
	validator = 0;
	mode = Integer;
	updating = updating_b = false;
	real_value = 0;
}

RKSpinBox::~RKSpinBox () {
	delete validator;
}

void RKSpinBox::setRealValue (double new_value) {
	real_value = new_value;
	setValue (0);
};

void RKSpinBox::interpretText () {
	if (updating) return;
	updating = true;

	if (mode == Real) {
		bool ok;
		double new_value = text ().toFloat (&ok);
		if (ok) real_value = new_value;
		valueChange ();
	} else {
		QSpinBox::interpretText ();
	}

	updating = false;
}

void RKSpinBox::updateDisplay () {
	if (updating_b) return;
	updating_b = true;

	if (mode == Real) {
		if (value () != 0) {
			int change = value ();
			setValue (0);

			int power = (int) log10 (real_value) - default_precision;
			if (power < (-default_precision)) power = -default_precision;
			if (power > 10) power = 10;
			double step = pow (10, power);

			real_value += change * step;
			if (real_value > real_max) real_value = real_max;
			if (real_value < real_min) real_value = real_min;
		}
		setUpdatesEnabled (false);
		QSpinBox::updateDisplay ();	// need this to enable/disable the button correctly
		editor ()->setText (QString ().setNum (real_value));
		setUpdatesEnabled (true);
	} else {
		QSpinBox::updateDisplay ();

		int power = (int) log10 (value ());
		int step = (int) pow (10, power-1);
		if (step < 1) step = 1;
		setSteps (step, 10*step);
	}

	updating_b = false;
}

void RKSpinBox::setRealMode (double min, double max, double initial, int default_precision, int max_precision) {
	RK_ASSERT ((max_precision >= default_precision) && (max_precision <= 8));

	mode = Real;
	QValidator *new_validator = new QDoubleValidator (min, max, max_precision, this);
	setValidator (new_validator);
	delete validator;
	validator = new_validator;

	/* the integer value and boundaries are mostly meaningless in real mode. Effectively, what we do is:
		1) set the value to 0
		2) whenever the value has changed, change the real value by that many steps (updateDisplay ())
		3) goto 1 */
	setMinValue (-1000);
	setMaxValue (1000);
	setSteps (1, 10);

	real_value = initial;
	real_min = min;
	real_max = max;
	RKSpinBox::default_precision = default_precision;

	setValue (0);
}

void RKSpinBox::setIntMode (int min, int max, int initial) {
	QValidator *new_validator = new QIntValidator (min, max, this);

	setMinValue (min);
	setMaxValue (max);
	setValue (initial);

	setValidator (new_validator);
	delete validator;
	validator = new_validator;
	mode = Integer;

	updateDisplay ();
}

#include "rkspinbox.moc"
