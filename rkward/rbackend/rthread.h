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
#include <qmutex.h>

#include "rcommand.h"

class REmbed;
class RInterface;

#define RCOMMAND_IN_EVENT 1001
#define RCOMMAND_OUT_EVENT 1002
#define RBUSY_EVENT 1003
#define RIDLE_EVENT 1004
#define RSTARTED_EVENT 2001
#define RERROR_SINKING_EVENT 2002

/** encapsulates the R-Backend in a separate thread. Use the inlines in RInterface and see documentation there.
@author Thomas Friedrichsmeier
*/
class RThread : public QThread
{
public:
    RThread (RInterface *parent);

    ~RThread();

	void issueCommand (RCommand *command, RCommand::CommandChain *chain);
	
	RCommand::CommandChain *startChain (RCommand::CommandChain *parent);
	RCommand::CommandChain *closeChain (RCommand::CommandChain *chain);
protected:
	void run ();
private:	
	RInterface *inter;
	RCommand::CommandChain *current_chain;
	RCommand::CommandChain *top_chain;
/** This is the last step in the chain of committing a command, and actually writes it */
	void doCommand (RCommand *command);
	REmbed *embeddedR;
	QMutex mutex;
};

#endif
