/***************************************************************************
                          rinterface.h  -  description
                             -------------------
    begin                : Fri Nov 1 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#ifndef RINTERFACE_H
#define RINTERFACE_H

#include <kprocess.h>
#include <qstrlist.h>
#include <qstring.h>

/**
  *@author Thomas Friedrichsmeier
  */

class RInterface : public KProcess  {
	Q_OBJECT
public: 
	RInterface();
	~RInterface();
	bool startR (QStrList &commandline);
	void issueCommand (const QString &command);
	bool commandRunning () { return command_running; };
signals:
	void receivedReply (QString result);
private:
	QString r_output;
	char *command_buffer;
	QString end_tag;
	bool command_running;
private slots:
	void gotROutput (KProcess *proc, char *buffer, int buflen);
};

#endif
