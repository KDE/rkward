/***************************************************************************
                          rksaveagent  -  description
                             -------------------
    begin                : Sun Aug 29 2004
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
#ifndef RKSAVEAGENT_H
#define RKSAVEAGENT_H

#include "../rbackend/rcommandreceiver.h"

#include <kurl.h>

class RCommandChain;

/**
This class basically provides a mechanism to let the user save a workspace, find out whether saving was successful and - if it was not - to ask for a new filename or the like.

@author Thomas Friedrichsmeier
*/
class RKSaveAgent : public RCommandReceiver {
public:
	enum DoneAction { DoNothing=0, Quit=1, Load=2 };

/** creates a new RKSaveAgent. If when_done == Quit, the RKSaveAgent will quit the application as soon as saving was successful (or it asked to by the user). Similarily, if when_done==Load, it will load a new workspace after saving (specify the url in load_url). If url is given (not empty), and not save_file_as, the agent will try to save to the given url, else it will ask the user to specify a url. RKSaveAgent will self destruct when done. */
	RKSaveAgent (KURL url, bool save_file_as=false, DoneAction when_done=DoNothing, KURL load_url="");
	
	~RKSaveAgent ();
protected:
	void rCommandDone (RCommand *command);
private:
	bool askURL ();
	void done ();
	RCommandChain *save_chain;
	KURL save_url;
	KURL load_url;
	DoneAction when_done;
};

#endif
