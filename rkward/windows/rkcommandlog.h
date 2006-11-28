/***************************************************************************
                          rkcommandlog  -  description
                             -------------------
    begin                : Sun Nov 3 2002
    copyright            : (C) 2002, 2006 by Thomas Friedrichsmeier
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

#ifndef RKCOMMANDLOG_H
#define RKCOMMANDLOG_H

#include <qstring.h>
#include <kmdichildview.h>

#include "../rbackend/rcommandreceiver.h"

class RCommand;
class ROutput;
class QPushButton;
class QTextEdit;
class RKCommandEditor;
class QBoxLayout;

/**
	\brief This widget shows all executed commands and their result
@author Thomas Friedrichsmeier
*/

class RKCommandLog : public KMdiChildView, public RCommandReceiver {
	Q_OBJECT
public: 
/** Adds input to the watch-window (i.e. commands issued) */
	void addInput (RCommand *command);
/** Adds output to the watch-window (i.e. replies received) */
	void newOutput (RCommand *command, ROutput *output_fragment);

	static void create ();
	static void destroy ();
	static RKCommandLog *getLog () { return rkcommand_log; };
signals:
/** the watch emits this, when it should be raised (apparently this can only be done from the main frame) */
	void raiseWatch ();
protected:
/** Command has finished. If the command has failed, it may be neccessary to print some more information */
	void rCommandDone (RCommand *command);
	RKCommandLog ();
	~RKCommandLog ();
public slots:
/** configures the watch-window */
	void configureWatch ();
/** clears the watch-window */
	void clearWatch ();
private:
	void addInputNoCheck (RCommand *command);
	void addOutputNoCheck (RCommand *command, const QString &output);
	void checkRaiseWatch (RCommand *command);
/** internal helper function, called whenever a line/lines have been added. Check whether log is longer than maximum setting. Scroll to the bottom */
	void linesAdded ();
/** Id of last command the input (i.e. the command itself) was shown for. Used to keep track of whether a command's input should be shown or not */
	int command_input_shown;
/** On a given command, the watch should not be raised more than once */
	int last_raised_command;

	QTextEdit *watch;
	QBoxLayout* pLayout;

	static RKCommandLog *rkcommand_log;
};

#endif
