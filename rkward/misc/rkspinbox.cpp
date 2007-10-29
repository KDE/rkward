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
#include <QTimer>

#include <math.h>
#include <limits.h>
#include <stdlib.h>

#include "../debug.h"

RKSpinBox::RKSpinBox (QWidget *parent) : QSpinBox (parent) {
	RK_TRACE (MISC);

	validator = 0;
	mode = Integer;
	real_value = 0;
	int_value = 0;
	default_precision = 0;
	int_min = INT_MIN;
	int_max = INT_MAX;

	connect (this, SIGNAL (valueChanged(int)), this, SLOT (updateValue(int)));
}

RKSpinBox::~RKSpinBox () {
	RK_TRACE (MISC);

	delete validator;
}

void RKSpinBox::setRealValue (double new_value) {
	RK_TRACE (MISC);

	real_value = new_value;
	setValue (0);
}

void RKSpinBox::setIntValue (int new_value) {
	RK_TRACE (MISC);

	int_value = new_value;
	setValue (0);
}


QString RKSpinBox::textFromValue (int) const {
	if (mode == Real) {
		return (QString ().setNum (real_value));
	} else {
		return (QString ().setNum (int_value));
	}
}

int RKSpinBox::valueFromText (const QString & text) const {
	if (mode == Real) {
		bool ok;
		double new_value = text.toDouble (&ok);
		if (ok && (new_value != real_value)) {
			double *cheat = const_cast<double*> (&real_value);
			*cheat = new_value;
			QTimer::singleShot (0, const_cast<RKSpinBox*>(this), SLOT (emitValueChange()));
		}
		return 0;
	} else {
		bool ok;
		int new_value = text.toInt (&ok);
		if (ok && (new_value != int_value)) {
			int *cheat = const_cast<int*> (&int_value);
			*cheat = new_value;
			QTimer::singleShot (0, const_cast<RKSpinBox*>(this), SLOT (emitValueChange()));
		}
		return 0;
	}
}

void RKSpinBox::stepBy (int steps) {
	RK_TRACE (MISC);
	setValue (steps);
}

QValidator::State RKSpinBox::validate (QString &input, int &pos) const {
	RK_TRACE (MISC);
	return (validator->validate (input, pos));
}

void RKSpinBox::emitValueChange () {
	RK_TRACE (MISC);

	emit (valueChanged (0));
}

void RKSpinBox::updateValue (int change) {
	RK_TRACE (MISC);
	if (change == 0) return;

	if (mode == Real) {
		int power;
		if (real_value != 0) {
			power = (int) log10 (fabs (real_value)) - default_precision;
			if (power < (-default_precision)) power = -default_precision;
			if (power > 10) power = 10;
		} else {
			power = -default_precision;
		}
		double step = pow (10, power);

		real_value += change * step;
		if (real_value > real_max) real_value = real_max;
		if (real_value < real_min) real_value = real_min;
	} else {
		int power;
		if (int_value != 0) {
			power = (int) log10 (abs (int_value));
		} else {
			power = -default_precision;
		}
		int step = (int) pow (10, power);
		if (step < 1) step = 1;

		int_value += change * step;
		if (int_value > int_max) int_value = int_max;
		if (int_value < int_min) int_value = int_min;
	}

	// update display and reset
	setValue (0);
}

void RKSpinBox::setRealMode (double min, double max, double initial, int default_precision, int max_precision) {
	RK_TRACE (MISC);
	RK_ASSERT ((max_precision >= default_precision) && (max_precision <= 8));

	mode = Real;
	QValidator *new_validator = new QDoubleValidator (min, max, max_precision, this);
	delete validator;
	validator = new_validator;

	/* the integer value and boundaries are mostly meaningless in real mode. Effectively, what we do is:
		1) set the value to 0
		2) whenever the value has changed, change the real value by that many steps (updateDisplay ())
		3) goto 1 */
	setMinValue (-1000);
	setMaxValue (1000);
	setSingleStep (1);

	real_value = initial;
	real_min = min;
	real_max = max;
	RK_ASSERT (real_min < real_max);
	RKSpinBox::default_precision = default_precision;

	setValue (0);
}

void RKSpinBox::setIntMode (int min, int max, int initial) {
	RK_TRACE (MISC);
	QValidator *new_validator = new QIntValidator (min, max, this);
	delete validator;
	validator = new_validator;

	/* see setRealMode for comments */

	setMinValue (-1000);
	setMaxValue (1000);
	setSingleStep (1);

	int_min = min;
	int_max = max;
	RK_ASSERT (int_min < int_max);
	int_value = initial;
	default_precision = 0;

	mode = Integer;

	setValue (0);
}

#include "rkspinbox.moc"
