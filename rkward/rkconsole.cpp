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
#include <qstringlist.h>
#include <qclipboard.h>
#include <qapplication.h>
 
#include <klocale.h>
#include <kaction.h>
 
#include "rkconsole.h"

#include "rkglobals.h"
#include "rkward.h"
#include "khelpdlg.h"
#include "debug.h"
#include "rbackend/rinterface.h"
#include "rbackend/rcommand.h"

RKConsole::RKConsole (QWidget *parent, const char *name) : KTextEdit (parent, name) {
	RK_TRACE (APP);

	QFont font ("Courier");
	setFont (font);
	
	setCaption (i18n ("R Console"));
	
	nprefix = "> ";
	iprefix = "+ ";
	prefix = nprefix;
	command_incomplete = false;
	clear();

	commands_history.append (new QString (""));
	commands_history.setAutoDelete (true);

	RKGlobals::rkApp()->m_manager->addPart (new RKConsolePart (this), false);
}


RKConsole::~RKConsole () {
	RK_TRACE (APP);
}

void RKConsole::keyPressEvent (QKeyEvent *e) {
	int para=0; int p=0;
	getCursorPosition (&para, &p);
	// Explicitely converting so we get less warnings.
	uint pos=(uint) p;
	
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
		cursorAtTheBeginning ();
		return;
	}

	if (para<paragraphs () - 1 || pos <= prefix.length () - 1){
		cursorAtTheEnd ();
	}
	
	KTextEdit::keyPressEvent (e);
}

QString RKConsole::currentCommand () {
	QString s = text (paragraphs () - 1).right(paragraphLength (paragraphs () - 1) - prefix.length () + 1);
	s = s.stripWhiteSpace ();
	
	return (s);
}

void RKConsole::setCurrentCommand (QString command) {
	removeParagraph (paragraphs () - 1);
	append (prefix + command);
	cursorAtTheEnd ();
}

void RKConsole::cursorAtTheEnd () {
	setCursorPosition (paragraphs () - 1, paragraphLength (paragraphs () - 1));
}

void RKConsole::submitCommand () {
	// If we added an item to the list, we delete it here.
	if (!(commands_history.getLast () == commands_history.current ())){
		commands_history.removeLast ();
	}
	QString c = currentCommand ();
	commands_history.append (new QString (c.latin1 ()));
	
	if (command_incomplete) {
		c.prepend (incomplete_command + "\n");
	}

	if (!currentCommand ().isEmpty ()) {
		RKGlobals::rInterface ()->issueCommand (c, RCommand::User, QString::null, this);
	} else {
		tryNextInBatch ();
	}
}

void RKConsole::commandsListUp () {
	if (commands_history.getFirst () == commands_history.current ()) {		// already at topmost item
		return;
	}
	// We add the current line to the list.
	if (commands_history.getLast () == commands_history.current ()) {
		commands_history.append (new QString (currentCommand ().latin1 ()));
	}
	commands_history.prev ();
	QString new_string = commands_history.current ()->utf8 ();
	setCurrentCommand (new_string);
}

void RKConsole::commandsListDown () {
	if (commands_history.getLast () == commands_history.current ()) {		// already at bottommost item
		return;
	}
	commands_history.next ();
	QString newString = commands_history.current ()->utf8 ();
	setCurrentCommand (newString);
	
	// If we are back at the begining of the list, we remove the item we've added.
	if (commands_history.getLast () == commands_history.current ()){
		commands_history.remove ();
	}
}

void RKConsole::cursorAtTheBeginning () {
	setCursorPosition (paragraphs () - 1, prefix.length ());
}

void RKConsole::rCommandDone (RCommand *command) {
	if (command->hasOutput ()) {
		append (command->output ());
	}
	if (command->hasError ()) {
		append (command->error ());
	} else if (command->errorSyntax ()) {
		append (i18n ("Syntax error"));
	}

	if (command->errorIncomplete ()) {
		prefix = iprefix;
		command_incomplete = true;
		incomplete_command = command->command ();
	} else {
		prefix = nprefix;
		command_incomplete = false;
		incomplete_command = QString::null;
	}

	tryNextInBatch ();
}

void RKConsole::submitBatch (QString batch) {
	// splitting batch, not allowing empty entries.
	// TODO: hack something so we can have empty entries.
	commands_batch = QStringList::split ("\n", batch, true);
	tryNextInBatch ();
}

void RKConsole::tryNextInBatch () {
	append (prefix);
	cursorAtTheEnd ();

	if (!commands_batch.isEmpty()) {
		// If we were not finished executing a batch of commands, we execute the next one.
		setCurrentCommand (currentCommand () + commands_batch.first ());
		commands_batch.pop_front ();
		if (!commands_batch.isEmpty ()){
			submitCommand ();
		}
		// We would put this here if we would want the last line to be executed. We generally don't want this, as there is an empty last item, if there is a newline at the end.
		//TODO: deal with this kind of situation better.
		//commands_batch.erase(commands_batch.begin());
	}
}

void RKConsole::paste () {
	QClipboard *cb = QApplication::clipboard ();
	submitBatch (cb->text ());
}

void RKConsole::clear () {
	KTextEdit::clear ();
	tryNextInBatch ();
}

///################### END RKConsole ########################
///################### BEGIN RKConsolePart ####################

RKConsolePart::RKConsolePart (RKConsole *console) : KParts::Part (0) {
	RK_TRACE (APP);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);

	setWidget (console);
	RKConsolePart::console = console;

	setXMLFile ("rkconsolepart.rc");

	context_help = new KAction (i18n ("&Function reference"), KShortcut ("F2"), this, SLOT (showContextHelp ()), actionCollection (), "function_reference");
}

RKConsolePart::~RKConsolePart () {
	RK_TRACE (APP);
}

void RKConsolePart::showContextHelp () {
	RK_TRACE (APP);

	int para, p;
	console->getCursorPosition (&para, &p);

	RKGlobals::helpDialog ()->getContextHelp (console->text (para), p);
}

#include "rkconsole.moc"
