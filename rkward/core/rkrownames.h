/***************************************************************************
                          rkrownames  -  description
                             -------------------
    begin                : Tue Mar 21 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#ifndef RKROWNAMES_H
#define RKROWNAMES_H

#include "rkvariable.h"

/** This class represents the row names of a data frame that is open for editing.
This object is special in that it exists during editing, only. It is not represented in the RObjectList. */
class RKRowNames : public RKVariable {
public:
	RKRowNames (RContainerObject *parent);
	~RKRowNames ();

	QString getFullName () const;
/** Reimplemented to return "row.names" */
	QString getBaseName () const { return QString ("row.names"); };
/** Reimplemented to do nothing. There is no metadata on the rownames. */
	void writeMetaData (RCommandChain *) {};

/** Reimplemented to always try to write data as numbers, if possible */
	void writeData (int from_row, int to_row, RCommandChain *chain=0);
/** Reimplemented to check, whether the values are all 1:n, custom, or invalid. */
	void setText (int row, const QString &text);
protected:
/** Reimplemented to disable duplicate checks during the setText() calls within */
	void setCharacterFromR (int from_row, int to_row, QString *data);
private:
/** @returns: true if the text was already unique, false, if it had to be adjusted */
	bool makeUnique (QString *text, bool non_sequentials_only);
	int is_sequential_up_to_row;
	bool check_duplicates;
};

#endif
