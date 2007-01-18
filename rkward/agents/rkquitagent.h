/***************************************************************************
                          rkquitagent  -  description
                             -------------------
    begin                : Thu Jan 18 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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
#ifndef RKQUITAGENT_H
#define RKQUITAGENT_H

#include <qobject.h>
#include "../rbackend/rcommandreceiver.h"

class RKProgressControl;

/** The purpose of RKQuitAgent is to delay the actual destruction of the app until all commands have finished in the backend. The quit agent can NOT handle queries for saving some more data, or similar things. Do not call before you really want to quit the application.
@author Thomas Friedrichsmeier
*/
class RKQuitAgent : public QObject, public RCommandReceiver {
	Q_OBJECT
public:
/** Constructor. As soon as you contruct an object of this type, the RKWard application *will* quit (but maybe with a short delay)! */
	RKQuitAgent (QObject *parent);

	~RKQuitAgent ();
	static bool quittingInProgress () { return quitting; };
public slots:
	void doQuitNow ();
	void showWaitDialog ();
protected:
	void rCommandDone (RCommand *command);
private:
	RKProgressControl *cancel_dialog;
	static bool quitting;
};

#endif
