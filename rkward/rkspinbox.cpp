/***************************************************************************
                          rkspinbox  -  description
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
#include "rkspinbox.h"

#include <qvalidator.h>

#include <math.h>

#include "debug.h"

RKSpinBox::RKSpinBox (QWidget *parent) : QSpinBox (parent) {
	validator = 0;
	mode = Integer;
}

RKSpinBox::~RKSpinBox () {
	delete validator;
}

int RKSpinBox::mapTextToValue (bool *ok) {
	if (mode == Real) {
		RK_DO (qDebug ("ttv %s -> %d", text ().latin1 (), (int) (divisor * text ().toFloat (ok))), PLUGIN, DL_DEBUG);
		return (round (divisor * text ().toFloat (ok)));
	} else {
		return QSpinBox::mapTextToValue (ok);
	}
}

QString RKSpinBox::mapValueToText (int v) {
	if (mode == Real) {
		QString dummy;
		RK_DO (qDebug ("vtt %d", v), PLUGIN, DL_DEBUG);
		RK_DO (qDebug (QString ("%1.%2").arg (v / divisor).arg (v % divisor, 2).latin1 ()), PLUGIN, DL_DEBUG);
		return (QString ().setNum ((double) v / double (divisor)));
	} else {
		return QSpinBox::mapValueToText (v);
	}
}

void RKSpinBox::setRealMode (double min, double max, double initial, int default_precision, int max_precision) {
	RK_ASSERT ((max_precision >= default_precision) && (max_precision <= 6) && (default_precision >= 0));
	RK_DO (qDebug ("min %f max %f initial %f defp %d maxp %d", min, max, initial, default_precision, max_precision), PLUGIN, DL_DEBUG);

	mode = Real;
	QValidator *new_validator = new QDoubleValidator (min, max, max_precision, this);
	divisor = (int) (pow (10, max_precision));
	setValidator (new_validator);
	delete validator;
	validator = new_validator;
	
	setMinValue ((int) (min * divisor));
	setMaxValue ((int) (max * divisor));
	setSteps ((int) (pow (10, default_precision)), (int) (pow (10, default_precision + 1)));
	setValue (round ((double) initial * divisor));
	RK_DO (qDebug ("minint %d maxint %d stepint %d pageint %d initialint %d", minValue (), maxValue (), lineStep (), pageStep (), round (initial * divisor)), PLUGIN, DL_DEBUG);
}

void RKSpinBox::setIntMode (int min, int max, int initial) {
	QValidator *new_validator = new QIntValidator (min, max, this);
	
	int range_power = (int) (log10 (max - min));
	if (range_power <= 0) {
		range_power = 1;
	}
	setMinValue (min);
	setMaxValue (max);
	setSteps ((int) (pow (10, range_power-1)), (int) (pow (10, range_power)));
	setValue (initial);

	setValidator (new_validator);
	delete validator;
	validator = new_validator;
	mode = Integer;
}

#include "rkspinbox.moc"
