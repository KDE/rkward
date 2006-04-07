/***************************************************************************
                          rkdrag.h  -  description
                             -------------------
    begin                : Thu Oct 31 2002
    copyright            : (C) 2002, 2006 by Thomas Friedrichsmeier
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

#ifndef RKDRAG_H
#define RKDRAG_H

#include <qdragobject.h>

class TwinTableMember;

/** A QDragObject that gets its data from a TwinTableMember. Used for all copy/paste/drag operations involving TwinTableMembers.
  *@author Thomas Friedrichsmeier
  */
class RKDrag : public QDragObject  {
public: 
	RKDrag (TwinTableMember *drag_source=0);
	~RKDrag ();
	const char* format (int i=0) const;
	QByteArray encodedData (const char * mimeType) const;
	bool provides (const char *mimeType);
private:
	QCString data;
};

#endif
