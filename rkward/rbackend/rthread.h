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

#include "rcommand.h"
#include "rcommandstack.h"

class REmbed;
class RInterface;

#define RCOMMAND_IN_EVENT 10001
#define RCOMMAND_OUT_EVENT 10002
#define RBUSY_EVENT 10003
#define RIDLE_EVENT 10004
#define RSTARTED_EVENT 11001
#define R_EVAL_REQUEST_EVENT 12001
// don't use the number following RSTARTUP_ERROR_EVENT, because an error code will be added!
#define RSTARTUP_ERROR_EVENT 13000

/** encapsulates the R-Backend in a separate thread. Use the inlines in RInterface and see documentation there.
@author Thomas Friedrichsmeier
*/
class RThread : public QThread {
public:
    RThread (RInterface *parent);

    ~RThread();

	void unlock () { locked=false; };
	void lock () { locked=true; };
	void kill () { killed = true; };
	
	void doSubstack (char **call, int call_length);
	
	RCommand *current_command;
	
	void domsleep (int ms) { msleep (ms); };
protected:
	void run ();
private:
	RInterface *inter;
/** This is the last step in the chain of committing a command, and actually writes it */
	void doCommand (RCommand *command);
	REmbed *embeddedR;
	
	bool locked;
	bool killed;
};

#endif
