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

#include "../rcommand.h"

#include <qfile.h>

/** This class (together with its base class REmbedInternal) takes care of the
	communication with the R-backend. It should eventually be converted to
	run in a separate thread, in order to allow non-blocking access and stopping/restarting
	the backend.
@author Thomas Friedrichsmeier
*/
class REmbed : public REmbedInternal {
public:
    REmbed();

    ~REmbed();
	
	void runCommand (RCommand *command);
	
	bool initialize ();
private:
	QIODevice::Offset outfile_offset;
	QIODevice::Offset errfile_offset;
	
	QFile outfile;
	QFile errfile;
};

#endif
