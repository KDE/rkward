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
#include "rkconsole.h"
 
#include <qfont.h>
#include <qstringlist.h>
#include <qclipboard.h>
#include <qapplication.h>

#include <klocale.h>
#include <kaction.h>

#include "rkglobals.h"
#include "rkward.h"
#include "khelpdlg.h"
#include "debug.h"
#include "rbackend/rinterface.h"
#include "rbackend/rcommand.h"
#include "settings/rksettingsmoduleconsole.h"

RKConsole::RKConsole () : KTextEdit (0) {
	RK_TRACE (APP);

	QFont font ("Courier");
	setFont (font);
	
	setCaption (i18n ("R Console"));
	
	nprefix = "> ";
	iprefix = "+ ";
	prefix = nprefix;
	command_incomplete = false;
	clear();

	commands_history.setAutoDelete (true);
	commands_history.append (new QString (""));
	QStringList history = RKSettingsModuleConsole::loadCommandHistory ();
	for (QStringList::const_iterator it = history.begin (); it != history.end (); ++it) {
		commands_history.append (new QString (*it));
	}

	current_command = 0;
}


RKConsole::~RKConsole () {
	RK_TRACE (APP);

	QStringList savelist;
	QString *str;
	for (str = commands_history.first (); str; str = commands_history.next ()) {
		savelist.append (*str);
	}

	RKSettingsModuleConsole::saveCommandHistory (savelist);
}

void RKConsole::keyPressEvent (QKeyEvent *e) {
	if (current_command) {
		e->ignore ();
		return;
	}

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
	QString s = text (paragraphs () - 1).right (paragraphLength (paragraphs () - 1) - prefix.length () + 1);
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
	addCommandToHistory (c);
	
	if (command_incomplete) {
		c.prepend (incomplete_command + "\n");
	}

	if (!currentCommand ().isEmpty ()) {
		current_command = new RCommand (c, RCommand::User, QString::null, this);
		RKGlobals::rInterface ()->issueCommand (current_command);
		emit (doingCommand (true));
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
		addCommandToHistory (currentCommand ());
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
			return;
		}
		// We would put this here if we would want the last line to be executed. We generally don't want this, as there is an empty last item, if there is a newline at the end.
		//TODO: deal with this kind of situation better.
		//commands_batch.erase(commands_batch.begin());
	}

	current_command = 0;
	emit (doingCommand (false));
}

void RKConsole::paste () {
	QClipboard *cb = QApplication::clipboard ();
	submitBatch (cb->text ());
}

void RKConsole::clear () {
	KTextEdit::clear ();
	tryNextInBatch ();
}

void RKConsole::addCommandToHistory (const QString &command) {
	commands_history.append (new QString (command.latin1 ()));

	if (RKSettingsModuleConsole::maxHistoryLength ()) {
		uint c = commands_history.count ();
		for (uint ui = c; c > RKSettingsModuleConsole::maxHistoryLength (); --ui) {
			commands_history.removeFirst ();
		}
	}
}

QPopupMenu *RKConsole::createPopupMenu (const QPoint &pos) {
	QPopupMenu *mp;
	emit (fetchPopupMenu (&mp));
	if (mp) return mp;

	return KTextEdit::createPopupMenu (pos);
}

///################### END RKConsole ########################
///################### BEGIN RKConsolePart ####################

RKConsolePart::RKConsolePart () : KParts::Part (0) {
	RK_TRACE (APP);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);

	setWidget (RKConsolePart::console = new RKConsole ());
	connect (console, SIGNAL (doingCommand (bool)), this, SLOT (setDoingCommand (bool)));

	setXMLFile ("rkconsolepart.rc");

	context_help = new KAction (i18n ("&Function reference"), KShortcut ("F2"), this, SLOT (showContextHelp ()), actionCollection (), "function_reference");
	interrupt_command = new KAction (i18n ("Interrupt running command"), KShortcut ("Ctrl+C"), this, SLOT (slotInterruptCommand ()), actionCollection (), "interrupt");
	interrupt_command->setIcon ("player_stop");
	interrupt_command->setEnabled (false);
// ugly HACK: we need this to override the default Ctrl+C binding
	interrupt_command->setShortcut ("Ctrl+C");

	copy = new KAction (i18n ("Copy selection"), 0, console, SLOT (copy ()), actionCollection ());
	paste = new KAction (i18n ("Paste"), KShortcut ("Ctrl+V"), console, SLOT (paste ()), actionCollection ());

	connect (console, SIGNAL (fetchPopupMenu (QPopupMenu**)), this, SLOT (makePopupMenu (QPopupMenu**)));
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

void RKConsolePart::setDoingCommand (bool busy) {
	RK_TRACE (APP);

	interrupt_command->setEnabled (busy);
}

void RKConsolePart::slotInterruptCommand () {
	RK_TRACE (APP);
	RK_ASSERT (console->current_command);

	console->commands_batch.clear ();
	RKGlobals::rInterface ()->cancelCommand (console->current_command);
	setDoingCommand (false);
}

void RKConsolePart::makePopupMenu (QPopupMenu **menu) {
	RK_TRACE (APP);

/*	// won't work, as both the factory (), and the KTextEdit will think, they own the menu -> crash
	*menu = static_cast<QPopupMenu *>(factory ()->container ("rkconsole_context_menu", this));
	factory ()->resetContainer ("rkconsole_context_menu"); */
	*menu = new QPopupMenu (console);
	copy->plug (*menu, 9);
	copy->setEnabled (console->hasSelectedText ());
	paste->plug (*menu, 10);
	(*menu)->insertSeparator (11);
	context_help->plug (*menu, 12);
	(*menu)->insertSeparator (13);
	interrupt_command->plug (*menu, 14);
}

#include "rkconsole.moc"
