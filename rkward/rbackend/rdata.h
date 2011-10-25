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

#ifndef RDATA_H
#define RDATA_H

#include <QVector>
#include <QStringList>

/** Class to represent data (other than output/erros) passed from the R backend to the main thread. Data is usually a vector of type int, double or QString, but can also contain a hierarchy of RData*s. RCommand is a subclass of this */
class RData {
public:
	RData ();
	~RData ();
	enum RDataType {
		StructureVector=0,
		IntVector=1,
		RealVector=2,
		StringVector=3,
		NoData=4
	};

	typedef QVector<qint32> IntStorage;
	typedef QVector<double> RealStorage;
	typedef QVector<RData*> RDataStorage;
	typedef QStringList StringStorage;
/** returns the type of data contained */
	RDataType getDataType () const { return datatype; };
/** returns the length (size) of the data array. @see RCommand::GetStringVector @see RCommand::GetRealVector @see RCommand::GetIntVector @see RCommand:GetStructure */
	unsigned int getDataLength () const;
/** returns a vector of double, if that is the type of data contained (else, an assert is raised, and an empty vector is returned). Can be used safely on a null RData pointer (but raises an assert in this case). @see RCommand::GetRealVector @see RData::getDataType () */
	const RealStorage realVector () const {
		if (this && (datatype == RealVector)) {
			return (*static_cast<RealStorage *> (data));
		}
		doAssert (RealVector);
		return RealStorage ();
	}
/** returns a vector of int, if that is the type of data contained (else, an assert is raised, and an empty vector is returned). Can be used safely on a null RData pointer (but raises an assert in this case). @see RCommand::GetIntVector @see RData::getDataType () */
	const IntStorage intVector () const {
		if (this && (datatype == IntVector)) {
			return (*static_cast<IntStorage *> (data));
		}
		doAssert (IntVector);
		return IntStorage ();
	}
/** returns a QStringList, if that is the type of data contained (else, an assert is raised, and an empty vector is returned). Can be used safely on a null RData pointer (but raises an assert in this case). @see RCommand::GetStringVector @see RData::getDataType () */
	const StringStorage stringVector () const {
		if (this && (datatype == StringVector)) {
			return (*static_cast<StringStorage *> (data));
		}
		doAssert (StringVector);
		return StringStorage ();
	}
/** returns a vector of RData*, if that is the type of data contained (else, an assert is raised, and an empty vector is returned). Can be used safely on a null RData pointer (but raises an assert in this case). @see RCommand::GetStructureVector @see RData::getDataType () */
	const RDataStorage structureVector () const {
		if (this && (datatype == StructureVector)) {
			return (*static_cast<RDataStorage *> (data));
		}
		doAssert (StructureVector);
		return RDataStorage ();
	}
	void discardData ();
/** purely for debugging! */
	void printStructure (const QString &prefix);

	void setData (const RDataStorage &from);
	void setData (const IntStorage &from);
	void setData (const RealStorage &from);
	void setData (const StringStorage &from);
/** public for technical reasons only. Do not use! Move data from the given RData to this RData. The source RData is emptied! */
	void swallowData (RData &from);
private:
	void doAssert (RDataType requested_type) const;
	RDataType datatype;
	void *data;
};

#endif
