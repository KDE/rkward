/***************************************************************************
                          rcommandreceiver  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef RCOMMANDRECEIVER_H
#define RCOMMANDRECEIVER_H

/**
Use this class as a base for all classes that need to handle RCommands. Provides a pure virtual function for handling of RCommand-results.

@author Thomas Friedrichsmeier
*/
class RCommandReceiver {
public:
    RCommandReceiver () {};

    virtual ~RCommandReceiver () {};
protected:
	friend class RCommand;
	virtual void rCommandDone (RCommand *command) = 0;
};

#endif
