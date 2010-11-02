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
}

RData::~RData () {
	RK_TRACE (RBACKEND);

	discardData ();
}

RData::RealStorage &RData::getRealVector () const {
	return (*static_cast<RealStorage *> (data));
}

RData::IntStorage &RData::getIntVector () const {
	return (*static_cast<IntStorage *> (data));
}

RData::StringStorage &RData::getStringVector () const {
	return (*static_cast<StringStorage *> (data));
}

RData::RDataStorage &RData::getStructureVector () const {
	return (*static_cast<RDataStorage *> (data));
}

void RData::discardData () {
	RK_TRACE (RBACKEND);

	if (datatype == StructureVector) {
		RDataStorage sdata = getStructureVector ();
		for (int i=sdata.size ()-1; i >= 0; --i) {
			delete (sdata[i]);
		}
		delete (static_cast<RDataStorage *> (data));
	} else if (datatype == IntVector) {
		delete (static_cast<IntStorage *> (data));
	} else if (datatype == RealVector) {
		delete (static_cast<RealStorage *> (data));
	} else if (datatype == StringVector) {
		delete (static_cast<StringStorage *> (data));
	} else {
		RK_ASSERT (datatype == NoData);
	}

	data = 0;
	datatype = RData::NoData;
}

unsigned int RData::getDataLength() const {
	if (datatype == RealVector) return (getRealVector ().size ());
	if (datatype == IntVector) return (getIntVector ().size ());
	if (datatype == StringVector) return (getStringVector ().size ());
	if (datatype == StructureVector) return (getStructureVector ().size ());
	return 0;
}

void RData::swallowData (RData &from) {
	data = from.data;
	datatype = from.datatype;

	from.data = 0;
	from.datatype = RData::NoData;
}

void RData::setData (const RDataStorage &from) {
	data = new RDataStorage (from);
	datatype = RData::StructureVector;
}

void RData::setData (const IntStorage &from) {
	data = new IntStorage (from);
	datatype = RData::IntVector;
}

void RData::setData (const RealStorage &from) {
	data = new RealStorage (from);
	datatype = RData::RealVector;
}

void RData::setData (const StringStorage &from) {
	data = new StringStorage (from);
	datatype = RData::StringVector;
}

void RData::printStructure (const QString &prefix) {
	switch (datatype) {
		case NoData:
			qDebug ("%s: NoData, length %d", prefix.toLatin1().data(), getDataLength ());
			break;
		case IntVector:
			qDebug ("%s: IntVector, length %d", prefix.toLatin1().data(), getDataLength ());
			for (unsigned int i = 0; i < getDataLength (); ++i) {
				qDebug ("%s%d: %d", prefix.toLatin1().data(), i, getIntVector ()[i]);
			}
			break;
		case RealVector:
			qDebug ("%s: RealVector, length %d", prefix.toLatin1().data(), getDataLength ());
			for (unsigned int i = 0; i < getDataLength (); ++i) {
				qDebug ("%s%d: %f", prefix.toLatin1().data(), i, getRealVector ()[i]);
			}
			break;
		case StringVector:
			qDebug ("%s: StringVector, length %d", prefix.toLatin1().data(), getDataLength ());
			for (unsigned int i = 0; i < getDataLength (); ++i) {
				qDebug ("%s%d: %s", prefix.toLatin1().data(), i, getStringVector ()[i].toLatin1().data());
			}
			break;
		case StructureVector:
			qDebug ("%s: StructureVector, length %d", prefix.toLatin1().data(), getDataLength ());
			for (unsigned int i = 0; i < getDataLength (); ++i) {
				QString sub_prefix = prefix + QString::number (i);
				getStructureVector ()[i]->printStructure (sub_prefix);
			}
			break;
		default:
			qDebug ("%s: INVALID %d, length %d", prefix.toLatin1().data(), datatype, getDataLength ());
	}
	qDebug ("%s: END\n\n", prefix.toLatin1 ().data());
}

