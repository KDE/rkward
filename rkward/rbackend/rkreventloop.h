/***************************************************************************
                          rkreventloop  -  description
                             -------------------
    begin                : Tue Apr 23 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier
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

#ifndef RKREVENTLOOP_H
#define RKREVENTLOOP_H

namespace RKREventLoop {
	void processX11Events ();
	void setEventHandler (void (* handler) ());
};

#endif
