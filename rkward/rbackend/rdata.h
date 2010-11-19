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
/** returns an array of double, if that is the type of data contained (else 0). @see RCommand::GetRealVector @see RData::getDataLength () @see RData::getDataType () */
	RealStorage &getRealVector () const;
/** returns an array of int, if that is the type of data contained (else 0). @see RCommand::GetIntVector @see RData::getDataLength () @see RData::getDataType () */
	IntStorage &getIntVector () const;
/** returns an array of QString, if that is the type of data contained (else 0). @see RCommand::GetStringVector @see RData::getDataLength () @see RData::getDataType () */
	StringStorage &getStringVector () const;
/** returns an array of RData*, if that is the type of data contained (else 0). @see RCommand::GetStructureVector @see RData::getDataLength () @see RData::getDataType () */
	RDataStorage &getStructureVector () const;
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
	RDataType datatype;
	void *data;
};

#endif
