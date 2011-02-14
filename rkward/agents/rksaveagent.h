/***************************************************************************
                          rksaveagent  -  description
                             -------------------
    begin                : Sun Aug 29 2004
    copyright            : (C) 2004, 2009, 2011 by Thomas Friedrichsmeier
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
#include <QObject>

class RCommandChain;

/**
This class basically provides a mechanism to let the user save a workspace, find out whether saving was successful and - if it was not - to ask for a new filename or the like.

@author Thomas Friedrichsmeier
*/
class RKSaveAgent : public RCommandReceiver, public QObject {
public:
	enum DoneAction { DoNothing=0, Load=1 };

/** creates a new RKSaveAgent. If when_done == Quit, the RKSaveAgent will quit the application as soon as saving was successful (or it asked to by the user). Similarly, if when_done==Load, it will load a new workspace after saving (specify the url in load_url). If url is given (not empty), and not save_file_as, the agent will try to save to the given url, else it will ask the user to specify a url. RKSaveAgent will self destruct when done. */
	explicit RKSaveAgent (KUrl url, bool save_file_as=false, DoneAction when_done=DoNothing, KUrl load_url=KUrl());
	
	~RKSaveAgent ();
protected:
	void rCommandDone (RCommand *command);
private:
	bool askURL ();
	void done ();
	RCommandChain *save_chain;
	KUrl save_url;
	KUrl load_url;
	KUrl previous_url;
	DoneAction when_done;
};

#endif
