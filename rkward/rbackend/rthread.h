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

#define RCOMMAND_IN_EVENT 10001
#define RCOMMAND_OUT_EVENT 10002
#define RBUSY_EVENT 10003
#define RIDLE_EVENT 10004
#define RSTARTED_EVENT 11001
// don't use the number following RSTARTUP_ERROR_EVENT, because an error code will be added!
#define RSTARTUP_ERROR_EVENT 12000

/** encapsulates the R-Backend in a separate thread. Use the inlines in RInterface and see documentation there.
@author Thomas Friedrichsmeier
*/
class RThread : public QThread
{
public:
    RThread (RInterface *parent);

    ~RThread();

	void issueCommand (RCommand *command, RCommandChain *chain);
	
	RCommandChain *startChain (RCommandChain *parent);
	RCommandChain *closeChain (RCommandChain *chain);
	
	void unlock () { locked=false; };
protected:
	void run ();
private:	
	RInterface *inter;
	RCommandChain *current_chain;
	RCommandChain *top_chain;
/** This is the last step in the chain of committing a command, and actually writes it */
	void doCommand (RCommand *command);
	REmbed *embeddedR;
	QMutex mutex;
	
	bool locked;
};

#endif
