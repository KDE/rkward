/***************************************************************************
                          robjectbrowser  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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
#include <qdir.h>
#include <qstringlist.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qobject.h>
#include <qevent.h>
#include <qtimer.h>
#include <QMenu>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QVBoxLayout>

#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kshellcompletion.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/markinterface.h>
#include <kxmlguifactory.h>

#include "rkglobals.h"
#include "rkward.h"
#include "windows/rkhelpsearchwindow.h"
#include "rbackend/rinterface.h"
#include "rbackend/rcommand.h"
#include "settings/rksettings.h"
#include "settings/rksettingsmoduleconsole.h"
#include "misc/rkcommonfunctions.h"
#include "core/robjectlist.h"
#include "core/rfunctionobject.h"

#include "debug.h"

// static
RKConsole* RKConsole::main_console = 0;

RKConsole::RKConsole (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, RKMDIWindow::ConsoleWindow, tool_window, name) {
	RK_TRACE (APP);

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);

	// create a Kate-part as command-editor
	KTextEditor::Editor* editor = KTextEditor::editor("katepart");
	RK_ASSERT (editor);

	doc = editor->createDocument (this);
	view = doc->createView (this);
	layout->addWidget (view);

// KDE4: does this work?
	KTextEditor::ConfigInterface *confint = qobject_cast<KTextEditor::ConfigInterface*> (view);
	RK_ASSERT (view);
	confint->setConfigValue ("dynamic-word-wrap", false);

	setFocusProxy (view);
	setFocusPolicy (Qt::StrongFocus);
	
	/* We need to unplug kactions that were plugged to the KateViewInternal in kateview.cpp.
	These actions include Key_Up, Key_Down, etc.
	It's a bit boring to do, but there is no way to do that another way yet.
	Apparently, things will be different in KDE 4.*/
	kate_edit_actions = view->findChild<KActionCollection*> ("edit_actions");
	if (kate_edit_actions) {
		// so they never get activated by shortcuts
		kate_edit_actions->clearAssociatedWidgets ();
	} else {
		RK_DO (qDebug ("Could not retrieve the katepart's edit action collection"), APP, DL_ERROR);
	}

	view->focusProxy()->installEventFilter(this);
	view->installEventFilter(this);

	doc->setModified (false);

	hinter = new RKFunctionArgHinter (this, view);
	
	setCaption (i18n ("R Console"));
	console_part = new RKConsolePart (this);
	setPart (console_part);
	initializeActivationSignals ();
	initializeActions (getPart ()->actionCollection ());

	nprefix = "> ";
	iprefix = "+ ";
	prefix = nprefix;
	command_incomplete = false;
	output_continuation = false;
// KDE4: a way to do this?
//	doc->setUndoSteps (0);
	clear ();
	doc->setHighlightingMode ("RKWard output");

	commands_history = RKSettingsModuleConsole::loadCommandHistory ();
	commands_history_position = commands_history.constEnd ();

	current_command = 0;
	tab_key_pressed_before = false;
	command_was_piped = false;
}

RKConsole::~RKConsole () {
	RK_TRACE (APP);

	delete hinter;
	RKSettingsModuleConsole::saveCommandHistory (commands_history);
}

void RKConsole::triggerEditAction (QString name) {
	RK_TRACE (APP);

	if (!kate_edit_actions) return;
	QAction *action = kate_edit_actions->action (name);
	if (action) action->trigger ();
	else RK_ASSERT (false);
}

void RKConsole::setCursorClear (int line, int col) {
	RK_TRACE (APP);

	view->setCursorPosition (KTextEditor::Cursor (line, col));
	view->removeSelection ();
}

bool RKConsole::handleKeyPress (QKeyEvent *e) {

	KTextEditor::Cursor c = view->cursorPosition ();
	int para=c.line (); int pos=c.column ();
	command_was_piped = false;

	if (para < doc->lines () - 1 || pos < prefix.length ()) {	// not inside the last line?
		int t = (int) pos;					// adjust position before interpreting keystroke
		if (prefix.length()>pos) t=prefix.length ();
		view->setCursorPosition (KTextEditor::Cursor (doc->lines () -1, t));
	}

	if (view->selection ()) {
		KTextEditor::Range selrange = view->selectionRange ();
		if (selrange.start ().line () < (doc->lines () - 1) || selrange.start ().column () < prefix.length ()) {
			// There is a selection outside the command line
			// Eat the key and beep (unless it's just a modifier key). Otherwise it might overwrite or delete the selection
			if (e->key () - (e->key () & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier | Qt::GroupSwitchModifier))) {
				KApplication::kApplication ()->beep ();
				e->ignore ();
			}
			return true;
		}
	}

	if (current_command) {
		e->ignore ();
		return true;
	}

	if (e->key () == Qt::Key_Up) {
		commandsListUp (RKSettingsModuleConsole::shouldDoHistoryContextSensitive (e->modifiers ()));
		return true;
	}
	else if (e->key () == Qt::Key_Down) {
		commandsListDown (RKSettingsModuleConsole::shouldDoHistoryContextSensitive (e->modifiers ()));
		return true;
	}
	command_edited = true; // all other keys are considered as "editing" the current comand

	if (e->key () == Qt::Key_Home) {
		if (e->modifiers () == Qt::ShiftModifier) {
			int lastline = doc->lines () - 1;
			int firstcol = prefix.length ();
			KTextEditor::Range newrange (lastline, firstcol, lastline, pos);

			if (view->selection ()) {
				KTextEditor::Range oldrange = view->selectionRange ();
				if (oldrange.end ().column () == pos) {
					newrange.setBothColumns (firstcol);
				} else {
					newrange.expandToRange (oldrange);
				}
			}
			view->setSelection (newrange);
		} else {
			cursorAtTheBeginning ();
		}
		return true;
	} else if (e->key () == Qt::Key_End) {
		if (e->modifiers () == Qt::ShiftModifier) {
			int lastline = doc->lines () - 1;
			int lastcol = doc->lineLength (lastline);
			KTextEditor::Range newrange (lastline, pos, lastline, lastcol);

			if (view->selection ()) {
				KTextEditor::Range oldrange = view->selectionRange ();
				if (oldrange.end ().column () == pos) {
					newrange.expandToRange (oldrange);
				} else {
					newrange.setBothColumns (lastcol);
				}
			}
			view->setSelection (newrange);
		} else {
			cursorAtTheEnd ();
		}
		return true;
	} else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
		hinter->hideArgHint ();
		submitCommand ();
		return true;
	} else if (e->key () == Qt::Key_Left){
		if (pos <= prefix.length ()) return true;

		if (e->modifiers () == Qt::NoModifier) setCursorClear (para, pos - 1);
		else if (e->modifiers () == Qt::ShiftModifier) triggerEditAction ("select_char_left");
		else if (e->modifiers () == Qt::ControlModifier) triggerEditAction ("word_left");
		else if (e->modifiers () == (Qt::ControlModifier + Qt::ShiftModifier)) triggerEditAction ("select_word_left");
		else return false;

		return true;
	} else if (e->key () == Qt::Key_Right){
		if (pos >= doc->lineLength (para)) return true;

		if (e->modifiers () == Qt::NoModifier) setCursorClear (para, pos + 1);
		else if (e->modifiers () == Qt::ShiftModifier) triggerEditAction ("select_char_right");
		else if (e->modifiers () == Qt::ControlModifier) triggerEditAction ("word_right");
		else if (e->modifiers () == (Qt::ControlModifier + Qt::ShiftModifier)) triggerEditAction ("select_word_right");
		else return false;

		return true;
	} else if (e->key () == Qt::Key_Backspace){
		if(pos<=prefix.length ()) return true;

		if (view->selection ()) {
			doc->removeText (view->selectionRange ());
			view->removeSelection ();
		} else {
			doc->removeText (KTextEditor::Range (para, pos, para, pos-1));
		}
		return true;
	} else if (e->key () == Qt::Key_Delete) {
		// not currently handled by an action in kateview internal, but better not rely on that fact!
		if (view->selection ()) {
			doc->removeText (view->selectionRange ());
			view->removeSelection ();
		} else {
			doc->removeText (KTextEditor::Range (para, pos, para, pos + 1));
		}
		return true;
	} else if (e->key () == Qt::Key_Tab) {
		doTabCompletion ();
		return true;
	}

	return false;
}

bool RKConsole::provideContext (unsigned int line_rev, QString *context, int *cursor_position) {
	RK_TRACE (COMMANDEDITOR);

	if (line_rev > 1) return false;

	if (line_rev == 0) {
		*cursor_position = currentCursorPositionInCommand ();
		*context = currentCommand ();
	} else {
		*cursor_position = -1;
		*context = incomplete_command;
	}

	return true;
}

void RKConsole::insertCompletion (int line_num, int word_start, int word_end, const QString &completion) {
	RK_TRACE (APP);

	int offset = prefix.length ();
	doc->replaceText (KTextEditor::Range (line_num, offset + word_start, line_num, offset + word_end), completion);
}

bool RKConsole::doTabCompletionHelper (int line_num, const QString &line, int word_start, int word_end, const QStringList &entries) {
	RK_TRACE (APP);

	int count = entries.count ();
	QStringList::const_iterator it;
	if (!count) return false;

	if (count == 1) {
		it = entries.constBegin ();
		insertCompletion (line_num, word_start, word_end, *it);
	} else if (tab_key_pressed_before) {
		int i=0;
		for (it = entries.constBegin (); it != entries.constEnd (); ++it) {
			if (i % 3) {
				doc->insertText (KTextEditor::Cursor (doc->lines () - 1, 0), (*it).leftJustified (35));
			} else {
				doc->insertLine (doc->lines (), *it);
			}
			++i;
		}
		doc->insertLine (doc->lines (), prefix + line);
		cursorAtTheEnd ();
	} else {
		tab_key_pressed_before = true;

		// do all entries have a common start?
		QString common;
		bool done = false;
		int i = 0;
		while (!done) {
			bool ok = true;
			QChar current;
			for (it = entries.constBegin (); it != entries.constEnd (); ++it) {
				if ((*it).length () <= i) {
					ok = false;
					break;
				}
				if (it == entries.constBegin ()) {
					current = (*it).at(i);
				} else if ((*it).at(i) != current) {
					ok = false;
					break;
				}
			}
			if (ok) common.append (current);
			else break;
			++i;
		}
		if (i > 0) {
			if ((int) common.length() > (word_end - word_start)) {		// more than there already is
				insertCompletion (line_num, word_start, word_end, common);
				return false;	// will beep to signal completion is not complete
			}
		}

		return true;
	}
	tab_key_pressed_before = false;
	return true;
}


void RKConsole::doTabCompletion () {
	RK_TRACE (APP);

	QString current_line = currentCommand ();
	int current_line_num = doc->lines () - 1;
	int word_start;
	int word_end;
	int cursor_pos = currentCursorPositionInCommand ();
	RKCommonFunctions::getCurrentSymbolOffset (current_line, cursor_pos, false, &word_start, &word_end);
	QString current_symbol = current_line.mid (word_start, word_end - word_start);

	// should we try a file name completion? Let's do some heuristics
	bool do_file_completion = false;
	int quote_start = current_line.findRev ('"', cursor_pos - 1);
	if (quote_start < 0) quote_start = current_line.findRev ('\'', cursor_pos - 1);

	if (quote_start >= 0) {
		// we found a quoting char at some earlier position on the line, we might need a filename completion
		do_file_completion = true;

		// however, some characters around quotes signify it's probably not really filename string
		char char_before_quote = ' ';
		if (quote_start > 1) char_before_quote = current_line.at (quote_start - 1).toLatin1();
		char char_after_quote = ' ';
		if (quote_start <= (current_line.length() - 2)) char_after_quote = current_line.at (quote_start + 1).toLatin1();
		// these signifiy it might be an object in a list somewhere, e.g. my.data$"varname"
		if ((char_before_quote == '[') || (char_before_quote == '$') || (char_before_quote == ':')) do_file_completion = false;
		// these indicate, the quote has likely ended rather that started
		if ((char_after_quote == ',') || (char_after_quote == ')') || (char_after_quote == ' ') || (char_after_quote == ';')) do_file_completion = false;

		if (do_file_completion) {
			int quote_end = current_line.find ('"', cursor_pos);
			if (quote_end < 0) quote_end = current_line.find ('\'', cursor_pos);
			if (quote_end < 0) quote_end = current_line.length ();
	
			QString current_name = current_line.mid (quote_start + 1, quote_end - quote_start - 1);
			KUrlCompletion comp (KUrlCompletion::FileCompletion);
			comp.setDir (QDir::currentPath ());
			comp.makeCompletion (current_name);
	
			if (doTabCompletionHelper (current_line_num, current_line, quote_start+1, quote_end, comp.allMatches ())) return;
		}
	}

	if (!do_file_completion) {
		if (!current_symbol.isEmpty ()) {		// try object name completion first
			RObject::RObjectMap map;
			RObject::RObjectMap::const_iterator it;
			RObjectList::getObjectList ()->findObjectsMatching (current_symbol, &map);
	
			QStringList entries;
			for (it = map.constBegin (); it != map.constEnd (); ++it) {
				entries.append (it.key ());
			}
			if (doTabCompletionHelper (current_line_num, current_line, word_start, word_end, entries)) return;
		}
	}

	// no completion was possible
	KApplication::kApplication ()->beep ();
}

bool RKConsole::eventFilter (QObject *o, QEvent *e) {
	if (o == getPart ()) {
		return RKMDIWindow::eventFilter (o, e);
	}

	if (e->type () == QEvent::KeyPress) {
		QKeyEvent *k = (QKeyEvent *)e;
		return handleKeyPress (k);
	} else if (e->type () == QEvent::MouseButtonPress) {
		// we seem to need this, as the kateview will swallow the contextMenuEvent, otherwise
		QMouseEvent *m = (QMouseEvent *)e;
		if (m->button() == Qt::RightButton) {
			qApp->sendEvent (this, new QContextMenuEvent (QContextMenuEvent::Other, m->globalPos ()));
			return (true);
		}
	} else if (e->type () == QEvent::MouseButtonRelease){
		QMouseEvent *m = (QMouseEvent *)e;
		if (m->button() == Qt::MidButton) {
			QClipboard *cb = QApplication::clipboard ();
			submitBatch (cb->text (QClipboard::Selection));
			return (true);
		} /* else if (m->button () == Qt::LeftButton) {
			// prevent cursor from leaving last line
			uint para=0; uint p=0;
			view->cursorPosition (&para, &p);
			if (para != doc->lines () - 1) {
				int y = view->y ();
				view->setCursorPosition (doc->lines() -1, p);
				int y2 = view->y ();
				qDebug ("%d, %d", y, y2);
				view->scroll (0, y - y2);
			}
		} */ // not good, yet: always jumps to bottom of view
	}

	if (acceptsEventsFor (o)) return RKMDIWindow::eventFilter (o, e);
	return false;
}

QString RKConsole::currentCommand () {
	RK_TRACE (APP);
	return (doc->line (doc->lines () - 1).mid (prefix.length ()));
}

void RKConsole::setCurrentCommand (const QString &command) {
	RK_TRACE (APP);

	int lastline = doc->lines () - 1;
	doc->replaceText (KTextEditor::Range (lastline, prefix.length (), lastline, doc->lineLength (lastline)), command);
	cursorAtTheEnd ();
}

void RKConsole::cursorAtTheEnd () {
	RK_TRACE (APP);

	view->setCursorPosition (doc->endOfLine (doc->lines () - 1));
// KDE4 TODO: view->scrollDown ();
}

void RKConsole::cursorAtTheBeginning () {
	RK_TRACE (APP);
// KDE4 TODO: view->scrollDown ();
	view->setCursorPosition (KTextEditor::Cursor (doc->lines() - 1, prefix.length ()));
}

void RKConsole::submitCommand () {
	RK_TRACE (APP);

	QString current_line = currentCommand ();
	QString command = current_line;
	addCommandToHistory (current_line);
	
	if (command_incomplete) {
		command.prepend (incomplete_command + '\n');
	}

	doc->insertLine (doc->lines (), QString ());
	if (!current_line.isEmpty ()) {
		current_command = new RCommand (command, RCommand::User | RCommand::Console, QString::null, this);
		RKGlobals::rInterface ()->issueCommand (current_command);
		interrupt_command_action->setEnabled (true);
	} else {
		tryNextInBatch ();
	}
}

void RKConsole::commandsListUp (bool context_sensitive) {
	RK_TRACE (APP);

	// if we are at the last line, i.e. not yet navigating the command history, store the current command
	if (commands_history.constEnd () == commands_history_position) history_editing_line = currentCommand ();

	if (context_sensitive) {
		if (command_edited) {
			command_history_context = currentCommand ();
			commands_history_position = commands_history.constEnd ();
			command_edited = false;
		}
	} else {
		command_edited = true;
	}

	bool found = false;
	QStringList::const_iterator it = commands_history_position;
	while (it != commands_history.constBegin ()) {
		--it;
	  	if ((!context_sensitive) || (*it).startsWith (command_history_context)) { // we found a match or previous line
			found = true;
			break;
		}
	}

	if (found) {		// if we did not find a previous matching line, do not touch the commands_history_position
		commands_history_position = it;
		setCurrentCommand (*commands_history_position);
	} else {
		KApplication::kApplication ()->beep ();
	}
}

void RKConsole::commandsListDown (bool context_sensitive) {
	RK_TRACE (APP);

	if (context_sensitive) {
		if (command_edited) {
			command_history_context = currentCommand ();
	  		commands_history_position = commands_history.constEnd ();
	  		command_edited = false;
	  		return; // back at bottommost item
		}
	} else {
		command_edited = true;
	}

	if (commands_history.constEnd () == commands_history_position) {		// already at bottommost item
		KApplication::kApplication ()->beep ();
		return;
	}

	while (commands_history_position != commands_history.constEnd ()) {
		++commands_history_position;
		if ((!context_sensitive) || (*commands_history_position).startsWith (command_history_context)) { // we found a match or next line
			break;
		}
	}

	if (commands_history.constEnd () == commands_history_position) setCurrentCommand (history_editing_line);
	else setCurrentCommand (*commands_history_position);
}

void RKConsole::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	if (command->errorSyntax () && command->error ().isEmpty ()) {
		doc->insertLine (doc->lines () - 1, i18n ("Syntax error\n"));
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

	if (output_continuation) doc->insertLine (doc->lines (), "");
	output_continuation = false;
	commands_history_position = commands_history.constEnd ();
	tryNextInBatch ();
}

void RKConsole::newOutput (RCommand *, ROutput *output) {
	RK_TRACE (APP);

	int start_line = doc->lines () -1;
	if (output_continuation) {
		doc->insertText (doc->documentEnd (), output->output);
	} else {
		doc->insertText (KTextEditor::Cursor (doc->lines () -1, 0), output->output);
	}
	int end_line = doc->lines () -1;
	if (output->type != ROutput::Output) {
		KTextEditor::MarkInterface *markiface = qobject_cast<KTextEditor::MarkInterface*> (doc);
		RK_ASSERT (markiface);
		for (int line = start_line; line < end_line; ++line) {
			markiface->addMark (line, KTextEditor::MarkInterface::BreakpointActive);
		}
	}

	if (RKSettingsModuleConsole::maxConsoleLines ()) {
		uint c = (uint) doc->lines();
// TODO: WORKAROUND: Somehow, when removing paragraph 0, the QTextEdit scrolls to the top in between (yes, this also happens when using removeParagaph (0)). Since this may happen very often in newOutput, we're a bit sloppy, and only remove lines after a certain threshold (20) is exceeded. When the command is finished, this will be cleaned up automatically.
		if (c > (RKSettingsModuleConsole::maxConsoleLines () + 20)) {
			view->setUpdatesEnabled (false);		// major performance boost while removing lines!
			//TODO : deal with the case when there is already a selection
			doc->removeText (KTextEditor::Range (0, 0, c - RKSettingsModuleConsole::maxConsoleLines (), 0));
			view->setUpdatesEnabled (true);
		}
	}
	cursorAtTheEnd ();
	output_continuation = true;
}

void RKConsole::submitBatch (const QString &batch) {
	RK_TRACE (APP);
	// splitting batch, not allowing empty entries.
	// TODO: hack something so we can have empty entries.
	commands_batch = QStringList::split ("\n", batch, true);
	tryNextInBatch (false);
}

void RKConsole::tryNextInBatch (bool add_new_line) {
	RK_TRACE (APP);
	if (add_new_line) {
		if (RKSettingsModuleConsole::maxConsoleLines ()) {
			uint c = (uint) doc->lines();
			setUpdatesEnabled (false);
			for (uint ui = c; ui > RKSettingsModuleConsole::maxConsoleLines (); --ui) {
				doc->removeLine (0);
			}
			setUpdatesEnabled (true);
		}
		if (!doc->lines ()) doc->insertLine (0, QString ());
		doc->insertText (KTextEditor::Cursor (doc->lines () - 1, 0), prefix);
//		doc->insertLine (doc->lines (), prefix);		// somehow, it seems to be safer to do this after removing superfluous lines, than before
		cursorAtTheEnd ();
	}

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
	interrupt_command_action->setEnabled (isBusy ());
}

void RKConsole::paste () {
	RK_TRACE (APP);
	QClipboard *cb = QApplication::clipboard ();
	submitBatch (cb->text ());
}

void RKConsole::clear () {
	RK_TRACE (APP);
	doc->clear ();
	tryNextInBatch ();
}

void RKConsole::addCommandToHistory (const QString &command) {
	RK_TRACE (APP);
	if (command.isEmpty () || commands_history.last() == command) return; // don't add empty or duplicate lines

	commands_history.append (command);
	history_editing_line = QString::null;

	if (RKSettingsModuleConsole::maxHistoryLength ()) {
		uint c = commands_history.count ();
		for (uint ui = c; ui > RKSettingsModuleConsole::maxHistoryLength (); --ui) {
			commands_history.pop_front ();
		}
	}
}

QString RKConsole::cleanedSelection () {
	RK_TRACE (APP);

	QString ret = view->selectionText ();
	ret.replace ('\n' + QString (nprefix), "\n");
	ret.replace ('\n' + QString (iprefix), "\n");
	if (ret.startsWith (nprefix)) {
		ret = ret.mid (QString (nprefix).length ());
	}

	return ret;
}

void RKConsole::copy () {
	RK_TRACE (APP);

	QApplication::clipboard()->setText (cleanedSelection ());
}

void RKConsole::literalCopy () {
	RK_TRACE (APP);

	QApplication::clipboard()->setText (view->selectionText ());
}

int RKConsole::currentCursorPosition (){
	KTextEditor::Cursor c = view->cursorPosition ();
	return(c.column ());
}

//KDE4 still needed? (see ctor)
void RKConsole::unplugAction(const QString &action, KActionCollection* ac) {
	QAction* a = ac->action (action);
	if( a ){
		a->setEnabled (false);
	}
}

int RKConsole::currentCursorPositionInCommand(){
	RK_TRACE (APP);
	return(currentCursorPosition() - prefix.length());
}

void RKConsole::resetIncompleteCommand () {
	RK_TRACE (APP);

	RK_ASSERT (command_incomplete);
	prefix = nprefix;
	command_incomplete = false;
	incomplete_command = QString::null;
	doc->insertLine (doc->lines (), "");

	tryNextInBatch (true);
}

void RKConsole::configure () {
	RK_TRACE (APP);
	RKSettings::configureSettings (RKSettings::Console, this);
}

void RKConsole::slotInterruptCommand () {
	RK_TRACE (APP);
	RK_ASSERT (current_command || command_incomplete);
	RK_DO (qDebug("received interrupt signal in console"), APP, DL_DEBUG);

	commands_batch.clear ();
	if (command_incomplete) {
		resetIncompleteCommand ();
	} else {
		RKGlobals::rInterface ()->cancelCommand (current_command);
	}
}

void RKConsole::runSelection () {
	RK_TRACE (APP);

	QString command = cleanedSelection ();
	pipeUserCommand (new RCommand (command, RCommand::User));
}

void RKConsole::showContextHelp () {
	RK_TRACE (APP);
	RKHelpSearchWindow::mainHelpSearch ()->getContextHelp (currentCommand (), currentCursorPositionInCommand ());
}

void RKConsole::initializeActions (KActionCollection *ac) {
	RK_TRACE (APP);

	context_help_action = ac->addAction ("function_reference", this, SLOT(showContextHelp()));
	context_help_action->setText (i18n ("&Function reference"));
	context_help_action->setShortcut (Qt::Key_F2);

	run_selection_action = ac->addAction ("run_selection", this, SLOT (runSelection()));
	run_selection_action->setText (i18n ("Run selection"));
	run_selection_action->setIcon (QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/run_selection.png"));
	run_selection_action->setShortcut (Qt::Key_F8);

	interrupt_command_action = ac->addAction ("interrupt", this, SLOT (slotInterruptCommand()));
	interrupt_command_action->setText (i18n ("Interrupt running command"));
	interrupt_command_action->setShortcut (Qt::ControlModifier + Qt::Key_C);
	interrupt_command_action->setIcon (KIcon ("player_stop"));
	interrupt_command_action->setEnabled (false);

	copy_action = ac->addAction ("rkconsole_copy", this, SLOT (copy()));
	copy_action->setText (i18n ("Copy selection cleaned"));

	copy_literal_action = ac->addAction ("rkconsole_copy_literal", this, SLOT (literalCopy()));
	copy_literal_action->setText (i18n ("Copy selection literally"));

	ac->addAction (KStandardAction::Clear, "rkconsole_clear", this, SLOT (clear()));
	paste_action = ac->addAction (KStandardAction::Paste, "rkconsole_paste", this, SLOT (paste()));
	QAction *action = ac->addAction ("rkconsole_configure", this, SLOT (configure()));
	action->setText (i18n ("Configure"));
}

void RKConsole::pipeUserCommand (const QString &command) {
	RK_TRACE (APP);

	RCommand *cmd = new RCommand (command, RCommand::User);
	pipeUserCommand (cmd);
}

void RKConsole::pipeUserCommand (RCommand *command) {
	RK_TRACE (APP);

	if (RKSettingsModuleConsole::pipeUserCommandsThroughConsole ()) {
		RKConsole::mainConsole ()->pipeCommandThroughConsoleLocal (command);
	} else {
		RKGlobals::rInterface ()->issueCommand (command);
	}
}

void RKConsole::pipeCommandThroughConsoleLocal (RCommand *command) {
	RK_TRACE (APP);

	activate (false);
	if ((!command_was_piped) && (isBusy () || (!currentCommand ().isEmpty ()))) {
		int res = KMessageBox::questionYesNo (this, i18n ("You have configured RKWrad to run script commands through the console. However, the console is currently busy (either a command is running, or you have started to enter text in the console). Do you want to bypass the console this one time, or do you want to try again later?"), i18n ("Console is busy"), KGuiItem (i18n ("Bypass console")), KGuiItem (i18n ("Cancel")));
		if (res == KMessageBox::Yes) {
			RKGlobals::rInterface ()->issueCommand (command);
		} else {
			delete command;
		}
	} else {
		QString command_string = command->command ();
		QString text = command_string;
		if (RKSettingsModuleConsole::addPipedCommandsToHistory()) {
			QStringList lines = QStringList::split ('\n', text);
			for (QStringList::const_iterator it = lines.constBegin (); it != lines.constEnd (); ++it) {
				addCommandToHistory (*it);
			}
		}
		text.replace ('\n', QString ("\n") + iprefix);
		doc->insertText (KTextEditor::Cursor (doc->lines () - 1, QString (nprefix).length ()), text + '\n');
		command->addReceiver (this);
		command->addTypeFlag (RCommand::Console);
		current_command = command;
		if (command_incomplete) {
			RK_ASSERT (command_was_piped);
			command_string.prepend (incomplete_command + '\n');
			command->setCommand (command_string);
		}
		command_was_piped = true;
		RKGlobals::rInterface ()->issueCommand (command);
	}
}

void RKConsole::contextMenuEvent (QContextMenuEvent * event) {
	RK_TRACE (APP);

	copy_action->setEnabled (view->selection ());
	copy_literal_action->setEnabled (view->selection ());
	run_selection_action->setEnabled (view->selection ());

	console_part->showPopupMenu (event->pos ());

	run_selection_action->setEnabled (true);
	copy_literal_action->setEnabled (true);
	copy_action->setEnabled (true);
}

///################### END RKConsole ########################
///################### BEGIN RKConsolePart ####################


RKConsolePart::RKConsolePart (RKConsole *console) : KParts::Part (0) {
	RK_TRACE (APP);

	setComponentData (KGlobal::mainComponent ());

	setWidget (console);

	setXMLFile ("rkconsolepart.rc");
}

RKConsolePart::~RKConsolePart () {
	RK_TRACE (APP);
}

void RKConsolePart::showPopupMenu (const QPoint &pos) {
	RK_TRACE (APP);

	QMenu *menu = static_cast<QMenu *> (factory ()->container ("rkconsole_context_menu", this));

	if (!menu) {
		RK_ASSERT (false);
		return;
	}
	menu->exec (pos);
}

#include "rkconsole.moc"
