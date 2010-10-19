/***************************************************************************
                          rdata  -  description
                             -------------------
    begin                : Sun Oct 01 2006
    copyright            : (C) 2006, 2010 by Thomas Friedrichsmeier
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

#include "rdata.h"

#include <qstring.h>

#include "../debug.h"

RData::RData () {
	RK_TRACE (RBACKEND);
	datatype = NoData;
	data = 0;
	length = 0; 
}

RData::~RData () {
	RK_TRACE (RBACKEND);

	discardData ();
}

double *RData::getRealVector () const {
	return (static_cast<double *> (data));
}

int *RData::getIntVector () const {
	return (static_cast<int *> (data));
}

QString *RData::getStringVector () const {
	return (static_cast<QString *> (data));
}

RData **RData::getStructureVector () const {
	return (static_cast<RData **> (data));
}

void RData::detachData () {
	data = 0;
	length = 0;
	datatype = NoData;
}

void RData::discardData () {
	RK_TRACE (RBACKEND);

	if (datatype == StructureVector) {
		RData **sdata = getStructureVector ();
		for (int i=length-1; i >= 0; --i) {
			delete (sdata[i]);
		}
		delete [] sdata;
	} else if (datatype == IntVector) {
		int *idata = getIntVector ();
		delete [] idata;
	} else if (datatype == RealVector) {
		double *rdata = getRealVector ();
		delete [] rdata;
	} else if (datatype == StringVector) {
		QString *stdata = getStringVector ();
		delete [] stdata;
	} else {
		RK_ASSERT (datatype == NoData);
	}

	detachData ();
}

void RData::setData (RData &from) {
	data = from.data;
	length = from.length;
	datatype = from.datatype;

	from.detachData ();
}

void RData::printStructure (const QString &prefix) {
	switch (datatype) {
		case NoData:
			qDebug ("%s: NoData, length %d", prefix.toLatin1().data(), length);
			break;
		case IntVector:
			qDebug ("%s: IntVector, length %d", prefix.toLatin1().data(), length);
			for (unsigned int i = 0; i < length; ++i) {
				qDebug ("%s%d: %d", prefix.toLatin1().data(), i, getIntVector ()[i]);
			}
			break;
		case RealVector:
			qDebug ("%s: RealVector, length %d", prefix.toLatin1().data(), length);
			for (unsigned int i = 0; i < length; ++i) {
				qDebug ("%s%d: %f", prefix.toLatin1().data(), i, getRealVector ()[i]);
			}
			break;
		case StringVector:
			qDebug ("%s: StringVector, length %d", prefix.toLatin1().data(), length);
			for (unsigned int i = 0; i < length; ++i) {
				qDebug ("%s%d: %s", prefix.toLatin1().data(), i, getStringVector ()[i].toLatin1().data());
			}
			break;
		case StructureVector:
			qDebug ("%s: StructureVector, length %d", prefix.toLatin1().data(), length);
			for (unsigned int i = 0; i < length; ++i) {
				QString sub_prefix = prefix + QString::number (i);
				getStructureVector ()[i]->printStructure (sub_prefix);
			}
			break;
		default:
			qDebug ("%s: INVALID %d, length %d", prefix.toLatin1().data(), datatype, length);
	}
	qDebug ("%s: END\n\n", prefix.toLatin1 ().data());
}

