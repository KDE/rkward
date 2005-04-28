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

/** This class (together with its base class REmbedInternal) takes care of the low level
	communication with the R-backend. It should only be used encapsulated in a thread, so
	commands get executed non-blocking.

	Only one REmbed-object can be used in an application.

	Don't use this class in RKWard directly. Use the interface provided by RInterface, instead.
	REmbed could really be merged with RThread. Don't wonder about strange up-and-down calls.
	Also REmbed and REmbedInternal are only separate for technical reasons (R-includes and Qt-includes clashing).

@author Thomas Friedrichsmeier
*/
class REmbed : public REmbedInternal {
public:
/** constructor. You need to specify a pointer to the parent thread. Don't create REmbed-instances. There should only ever be one instance of
REmbed and that is created in RThread::RThread */
	REmbed (RThread *thread);
/** destructor */
	~REmbed ();

/** runs the specified RCommand immediately. Also adds some information on the success/failure/status to the RCommand after running. */
	void runCommand (RCommand *command);

/** An enum describing whether initialization of the embedded R-process went well, or what errors occurred. */
	enum InitStatus {
		Ok=0,					/**< No error */
		LibLoadFail=1,		/**< Error while trying to load the rkward R package */
		SinkFail=2,			/**< Error while redirecting R's stdout and stderr to files to be read from rkward */
		OtherFail=4			/**< Other error while initializing the embedded R-process */
	};
	
/** initializes the R-backend. Returns an error-code that consists of a bit-wise or-conjunction of the REmbed::InitStatus -enum, REmbed::Ok on success.
Note that you should call initialize only once in a application */
	int initialize ();
	
/** this function is public for technical reasons, only. Don't use except from REmbedInternal! Called from REmbedInternal when the R backend
places a call to the frontend. The call will be directly passed up to RThread::doSubstack (). */
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
