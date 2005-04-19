/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
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
#ifndef RKCONSOLE_H
#define RKCONSOLE_H

#include <qtextedit.h>
#include <qptrlist.h>

#include "rbackend/rcommandreceiver.h"


class QStringList;

/**
** 	\brief Provides an R-like console.
**
** This class provides a console, which is very similar to the classic R console. It is mainly used by RKwatch to allow
** the user to enter commands manualy. It is basically just a modified QTextEdit.
** 
** \sa RKwatch, QTextEdit
** 
** @author Pierre Ecochard
**/

class RKConsole : public QTextEdit, public RCommandReceiver {
Q_OBJECT
public:
	/** Constructor */
	RKConsole(QWidget *parent = 0, const char *name = 0);
	/** Destructor */
	~RKConsole();
	
	/** Empties the console */
	void flush ();
	/** Sets the current command
	\param command the new command */
	void setCurrentCommand (QString command);
	/** Submits a batch of commands, line by line.
	\param batch a QString containing the batch of commands to be executed */
	void submitBatch(QString batch);
	

signals:
	void userCommandRunning (RCommand *command);
/** Emited when the command has been executed */
	void userCommandFinished ();
protected:
	void keyPressEvent ( QKeyEvent * e );
	void rCommandDone (RCommand *command);
private:
/** This string stores the prefix printed at the beginning of each line. */
	QString prefix;
	QString incomplete_command;
	bool command_incomplete;
/** A list to store previous commands */
	QPtrList<QString> commandsList;
/** A list to store a commands batch that will be executed one line at a time */
	QStringList commandsBatch;
/** Sets the cursor position to the end of the last line. */
	void cursorAtTheEnd();
/** Add a new line, with the prefix. */
	void newLine();
/** Returns the command currently being edited (not executed yet) */
	QString currentCommand();
/** Submits the current command */
	void submitCommand();
/** Set the current command to the previous command in the command list */
	void commandsListUp();
/** Set the current command to the next command in the command list */
	void commandsListDown();
/** Sets the cursor position to the beginning of the last line. */
	void cursorAtTheBeginning();
/** We overload the paste function, in order to intercept paste commands and get them executed thru submitBatch.
@sa submitBatch */
	void paste();
    
private slots:
/** Called when a command has been executed. */
	void slotCommandFinished();
};

#endif
