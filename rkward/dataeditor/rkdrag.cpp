/***************************************************************************
                          rkdrag.cpp  -  description
                             -------------------
    begin                : Thu Oct 31 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#include "rkdrag.h"

#include "twintable.h"

RKDrag::RKDrag(TwinTable *dragSource, const char *name) : QDragObject (dragSource, name){
	data = dragSource->encodeSelection ();
}

RKDrag::~RKDrag(){
}

const char* RKDrag::format (int i) const {
	if (i == 0) {
		return "text/tab-separated-values";
	}
	if (i == 1) {
		return "text/plain";
	}
	return 0;
}

QByteArray RKDrag::encodedData (const char * mimeType) const {
	QString request = mimeType;
	if (request == format (0)) {
		qDebug ("hit----------------------------");
		return data;
	}
	if (request == format (1)) {
		qDebug ("hit++++++++++++++++++++++++++++");
		return data;
	}
	return data;
	return empty;
}

bool RKDrag::provides (const char *mimeType) {
	qDebug ("provides");
	if ((mimeType == format (0)) || (mimeType == format (1))) {
		return true;
	}
	return false;
}
