/*
rdata - This file is part of RKWard (https://rkward.kde.org). Created: Sun Oct 01 2006
SPDX-FileCopyrightText: 2006-2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rdata.h"

#include <qstring.h>

#include "../debug.h"

RData::RData () {
	RK_TRACE (RBACKEND);
	datatype = NoData;
	data = nullptr;
}

RData::~RData () {
	RK_TRACE (RBACKEND);

	discardData ();
}

void RData::doAssert(RData::RDataType requested_type) const {
	RK_DEBUG (RBACKEND, DL_ERROR, "Requested data of type %d, while %p has type %d", requested_type, this, datatype);
}

void RData::discardData () {
	RK_TRACE (RBACKEND);

	if (datatype == StructureVector) {
		RDataStorage sdata = *(static_cast<RDataStorage *> (data));
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

	data = nullptr;
	datatype = RData::NoData;
}

unsigned int RData::getDataLength() const {
	if (datatype == RealVector) return (static_cast<RealStorage *> (data)->size ());
	if (datatype == IntVector) return (static_cast<IntStorage *> (data)->size ());
	if (datatype == StringVector) return (static_cast<StringStorage *> (data)->size ());
	if (datatype == StructureVector) return (static_cast<RDataStorage *> (data)->size ());
	return 0;
}

void RData::swallowData (RData &from) {
	data = from.data;
	datatype = from.datatype;

	from.data = nullptr;
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
	if (datatype == NoData) {
		qDebug ("%s: NoData, length %d", prefix.toLatin1().data(), getDataLength ());
	} else if (datatype == IntVector) {
		qDebug ("%s: IntVector, length %d", prefix.toLatin1().data(), getDataLength ());
		IntStorage data = intVector ();
		for (int i = 0; i < data.size (); ++i) {
			qDebug ("%s%d: %d", prefix.toLatin1().data(), i, data.at (i));
		}
	} else if (datatype == RealVector) {
		qDebug ("%s: RealVector, length %d", prefix.toLatin1().data(), getDataLength ());
		RealStorage data = realVector ();
		for (int i = 0; i < data.size (); ++i) {
			qDebug ("%s%d: %f", prefix.toLatin1().data(), i, data.at (i));
		}
	} else if (datatype == StringVector) {
		qDebug ("%s: StringVector, length %d", prefix.toLatin1().data(), getDataLength ());
		StringStorage data = stringVector ();
		for (int i = 0; i < data.size (); ++i) {
			qDebug ("%s%d: %s", prefix.toLatin1().data(), i, qPrintable (data.at (i)));
		}
	} else if (datatype == StructureVector) {
		qDebug ("%s: StructureVector, length %d", prefix.toLatin1().data(), getDataLength ());
		RDataStorage data = structureVector ();
		for (int i = 0; i < data.size (); ++i) {
			QString sub_prefix = prefix + QString::number (i);
			data.at (i)->printStructure (sub_prefix);
		}
	} else {
		qDebug ("%s: INVALID %d, length %d", prefix.toLatin1().data(), datatype, getDataLength ());
	}
	qDebug ("%s: END\n\n", prefix.toLatin1 ().data());
}

