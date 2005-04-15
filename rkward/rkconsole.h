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

/**
\brief Provides an R-like console.
This class provides a console, which is very similar to the classic R console. It is mainly used by RKwatch to allow
the user to enter commands manualy. It is basically just a modified QTextEdit.

\sa RKwatch, QTextEdit

@author Pierre Ecochard
*/
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

signals:
	void userCommandRunning (RCommand *command);
protected:
	void keyPressEvent ( QKeyEvent * e );
	void rCommandDone (RCommand *command);
private:
	QString prefix;
	QString incomplete_command;
	bool command_incomplete;
/** A list to store previous commands */
	QPtrList<QString> commandsList;
/** Sets the cursor position to the end of the last line. */
    void cursorAtTheEnd();
    void newLine();
    QString currentCommand();
/**
Submits the current command
*/
    void submitCommand();
    void commandsListUp();
    void commandsListDown();
    void cursorAtTheBegining();
};

#endif
