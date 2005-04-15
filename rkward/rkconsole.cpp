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

 
 
// TODO : use QStringList::split to enter several lines at a time.
 
 
#include <qfont.h>
 
#include <klocale.h>
 
#include "rkconsole.h"

#include "rkglobals.h"
#include "rbackend/rinterface.h"
#include "rbackend/rcommand.h"

RKConsole::RKConsole(QWidget *parent, const char *name)
 : QTextEdit(parent, name)
{
	QFont font ("Courier");
	setFont (font);
	
	prefix = "> ";
	command_incomplete = false;
	flush();
	
	commandsList.append (new QString(""));
	
	commandsList.setAutoDelete( TRUE );
	
}


RKConsole::~RKConsole()
{
}


void RKConsole::keyPressEvent ( QKeyEvent * e )
{
	int para=0; int pos=0;
	getCursorPosition (&para, &pos);
	
	
	if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
		submitCommand ();
		return;
	}
	else if (e->key () == Qt::Key_Up){
		commandsListUp ();
		return;
	}
	else if (e->key () == Qt::Key_Down){
		commandsListDown ();
		return;
	}
	else if (e->key () == Qt::Key_Left && pos<=prefix.length ()){
		return;
	}
	else if (e->key () == Qt::Key_Backspace && pos<=prefix.length ()){
		return;
	}
	else if (e->key () == Qt::Key_Home){
		cursorAtTheBegining ();
		return;
	}
	
	
	
	if (para<paragraphs () - 1 || pos <= prefix.length () - 1){
		cursorAtTheEnd ();
	}
	
	QTextEdit::keyPressEvent( e );
}


void RKConsole::newLine()
{
	append (prefix);
	cursorAtTheEnd ();
}

QString RKConsole::currentCommand() 
{
	QString s = text (paragraphs () - 1).right(paragraphLength (paragraphs () - 1) - prefix.length () + 1);
	s = s.stripWhiteSpace ();
	
	return(s);
}


void RKConsole::flush()
{
	setText("");
	append (i18n (" "));
	newLine ();
}


void RKConsole::setCurrentCommand(QString command)
{
	removeParagraph (paragraphs () - 1);
	append (prefix + command);
	cursorAtTheEnd ();
}


void RKConsole::cursorAtTheEnd()
{
	setCursorPosition (paragraphs () - 1, paragraphLength (paragraphs () - 1));
}



void RKConsole::submitCommand()
{
	if (!currentCommand ().isEmpty ()) {
		// If we added an item to the list, we delete it here.
		if (!(commandsList.getLast () == commandsList.current ())){
			commandsList.last ();
			commandsList.remove ();
		}
		QString c = currentCommand ();
		commandsList.append (new QString (c.latin1 ()));
		
		if (command_incomplete) {
			c.prepend (incomplete_command + "\n");
		}

		RKGlobals::rInterface ()->issueCommand (c, RCommand::User, "", this);
	} else {
		command_incomplete = false;
		newLine ();
	}
}



void RKConsole::commandsListUp()
{
	if (commandsList.getFirst () == commandsList.current ()){
		return;
	}
	// We add the current line to the list.
	if (commandsList.getLast () == commandsList.current ()) {
		commandsList.append (new QString(currentCommand ().latin1 ()));
	}
	commandsList.prev ();
	QString newString = commandsList.current ()->utf8 ();
	setCurrentCommand (newString);
}



void RKConsole::commandsListDown()
{
	if (commandsList.getLast () == commandsList.current ()){
		return;
	}
	commandsList.next ();
	QString newString = commandsList.current ()->utf8 ();
	setCurrentCommand (newString);
	
	// If we are back at the begining of the list, we remove the item we've added.
	if (commandsList.getLast () == commandsList.current ()){
		commandsList.remove ();
	}
}


/*!
    \fn RKConsole::cursorAtTheBegining()
    Puts the cursor at the begining of the last line.
 */
void RKConsole::cursorAtTheBegining()
{
	setCursorPosition (paragraphs () - 1, prefix.length ());
}


void RKConsole::rCommandDone (RCommand *command) {
	if (command->hasOutput ()) {
		append (command->output ());
	}
	if (command->hasError ()) {
		append (command->error ());
	}

	if (command->errorIncomplete ()) {
		prefix = "# ";
		command_incomplete = true;
		incomplete_command = command->command ();
	} else {
		prefix = "> ";
		command_incomplete = false;
		incomplete_command = "";
	}

	newLine ();
}

#include "rkconsole.moc"
