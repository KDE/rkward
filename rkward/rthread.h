/***************************************************************************
                          rthread  -  description
                             -------------------
    begin                : Mon Aug 2 2004
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
#ifndef RTHREAD_H
#define RTHREAD_H

#include <qthread.h>
#include <qptrlist.h>

class REmbed;
class RInterface;
class RCommand;

#define RCOMMAND_IN_EVENT 1001
#define RCOMMAND_OUT_EVENT 1002
#define RBUSY_EVENT 1003
#define RIDLE_EVENT 1004

/** encapsulated the R-Backend in a separate thread.
@author Thomas Friedrichsmeier
*/
class RThread : public QThread
{
public:
    RThread(RInterface *parent);

    ~RThread();
public:
	void issueCommand (RCommand *command);
protected:
	void run ();
private:
	RInterface *inter;
	QPtrList<RCommand> command_stack;
/** This is the last step in the chain of committing a command, and actually writes it */
	void write (RCommand *command);
	REmbed *embeddedR;
};

#endif
