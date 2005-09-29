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

#include "../debug.h"

RKDrag::RKDrag(TwinTable *dragSource, const char *name) : QDragObject (dragSource, name){
	RK_TRACE (EDITOR);

	data = dragSource->encodeSelection ();
}

RKDrag::~RKDrag(){
	RK_TRACE (EDITOR);
}

const char* RKDrag::format (int i) const {
	RK_TRACE (EDITOR);
	if (i == 0) {
		return "text/tab-separated-values";
	}
	if (i == 1) {
		return "text/plain";
	}
	return 0;
}

QByteArray RKDrag::encodedData (const char * mimeType) const {
	RK_TRACE (EDITOR);
	QString request = mimeType;
	if (request == format (0)) {
		return data;
	}
	if (request == format (1)) {
		return data;
	}
	return data;
	return empty;
}

bool RKDrag::provides (const char *mimeType) {
	RK_TRACE (EDITOR);
	if ((mimeType == format (0)) || (mimeType == format (1))) {
		return true;
	}
	return false;
}
