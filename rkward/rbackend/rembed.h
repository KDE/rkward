/***************************************************************************
                          rembed  -  description
                             -------------------
    begin                : Mon Jul 26 2004
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
#ifndef REMBED_H
#define REMBED_H

#include "rembedinternal.h"

#include "rcommand.h"

#include <qfile.h>

class RThread;

/** This class (together with its base class REmbedInternal) takes care of the
	communication with the R-backend. It should only be used encapsulated in a thread, so
	commands get executed non-blocking.
	Only one REmbed-object can be used in an application.
	Don't use this class in RKWard directly. Use the interface provided by RInterface, instead.
	REmbed should really be merged with RThread. Don't wonder about strange up-and-down calls.
@author Thomas Friedrichsmeier
*/
class REmbed : public REmbedInternal {
public:
    REmbed (RThread *thread);

    ~REmbed ();
	
	void runCommand (RCommand *command);
	
	enum InitStatus { Ok=0, LibLoadFail=1, SinkFail=2, OtherFail=4 };
	
	/** initializes the R-backend. Returns an error-code that consists of a bit-wise or-conjunction of the InitStatus-enum. 0 on success.
	Note that you should call initialize only once in a application */
	int initialize ();
	
/// these functions are public for technical reasons, only. Don't use except from REmbedInternal!
	void handleSubstackCall (char **call, int call_length);
	//char **handleGetValueCall (char **call, int call_length, int *reply_length);
private:
	QIODevice::Offset outfile_offset;
	QIODevice::Offset errfile_offset;
	
	QFile outfile;
	QFile errfile;
	
	RThread *thread;
};

#endif
