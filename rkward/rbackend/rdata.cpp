/***************************************************************************
                          rdata  -  description
                             -------------------
    begin                : Sun Oct 01 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

double *RData::getRealVector () {
	return (static_cast<double *> (data));
}

int *RData::getIntVector () {
	return (static_cast<int *> (data));
}

QString *RData::getStringVector () {
	return (static_cast<QString *> (data));
}

RData **RData::getStructureVector () {
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

void RData::setData (RData *from) {
	data = from->data;
	length = from->length;
	datatype = from->datatype;

	from->detachData ();
	delete from;
}
