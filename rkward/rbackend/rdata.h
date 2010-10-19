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

#ifndef RDATA_H
#define RDATA_H

class QString;
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

/** returns the type of data contained */
	RDataType getDataType () { return datatype; };
/** returns the length (size) of the data array. @see RCommand::GetStringVector @see RCommand::GetRealVector @see RCommand::GetIntVector @see RCommand:GetStructure */
	unsigned int getDataLength () { return length; };
/** returns an array of double, if that is the type of data contained (else 0). The array is owned by the RCommand! @see RCommand::GetRealVector @see RData::detachData () @see RData::getDataLength () @see RData::getDataType () */
	double *getRealVector () const;
/** returns an array of int, if that is the type of data contained (else 0). The array is owned by the RCommand! @see RCommand::GetIntVector @see RData::detachData () @see RData::getDataLength () @see RData::getDataType () */
	int *getIntVector () const;
/** returns an array of QString, if that is the type of data contained (else 0). The array is owned by the RCommand! @see RCommand::GetStringVector @see RData::detachData () @see RData::getDataLength () @see RData::getDataType () */
	QString *getStringVector () const;
/** returns an array of RData*, if that is the type of data contained (else 0). The array is owned by the RCommand! @see RCommand::GetStructureVector @see RData::detachData () @see RData::getDataLength () @see RData::getDataType () */
	RData **getStructureVector () const;
/** The data contained in the RData structure is owned by RData, and will usually be deleted at the end of the lifetime of the RData object. If you want to keep the data, call detachData () to prevent this deletion. You will be responsible for deletion of the data yourself. */
	void detachData ();
	void discardData ();
/** purely for debugging! */
	void printStructure (const QString &prefix);

/** public for technical reasons only. Do not use! Move data from the given RData to this RData. The source RData is emptied! */
	void setData (RData &from);
/** public for technical reasons only. Do not use! */
	RDataType datatype;
/** public for technical reasons only. Do not use! */
	void *data;
/** public for technical reasons only. Do not use! */
	unsigned int length;
};

#endif
