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

/**
This class provides a console, which is very similar to the classic R console. It is mainly used by RKwatch to allow
the user to enter commands manualy. It is basically just a modified QTextEdit.

\sa RKwatch, QTextEdit

@author Pierre Ecochard
*/
class RKConsole : public QTextEdit
{
Q_OBJECT
public:
/** Constructor */
    RKConsole(QWidget *parent = 0, const char *name = 0);
/** Destructor */
    ~RKConsole();
    
/** Adds input to the watch-window (i.e. commands issued) 
\param s the input to be added. */
	void addInput (QString s);
/** Adds output to the watch-window (i.e. replies received) 
\param output the output received
\param error the optional error */
	void addOutput (QString output, QString error);    
/** Empties the console */
    void flush();
/** Sets the current command
\param command the new command */
    void setCurrentCommand(QString command);


signals:
	void commandSubmitted (QString c);
protected:
	void keyPressEvent ( QKeyEvent * e );
private:
	QString prefix;
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
};

#endif
