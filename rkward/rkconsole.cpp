/***************************************************************************
                          rkconsole  -  description
                             -------------------
    begin                : Thu Aug 19 2004
    copyright            : (C) 2004, 2006, 2007, 2009, 2010, 2011 by Thomas Friedrichsmeier
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
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <ktemporaryfile.h>

#include "rkglobals.h"
#include "rkward.h"
#include "windows/rkhelpsearchwindow.h"
#include "rbackend/rinterface.h"
#include "rbackend/rcommand.h"
#include "settings/rksettings.h"
#include "settings/rksettingsmoduleconsole.h"
#include "misc/rkcommonfunctions.h"
#include "misc/rkstandardicons.h"
#include "misc/rkstandardactions.h"
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

	KTextEditor::ConfigInterface *confint = qobject_cast<KTextEditor::ConfigInterface*> (view);
	RK_ASSERT (view);
	confint->setConfigValue ("dynamic-word-wrap", false);

	setFocusProxy (view);
	setFocusPolicy (Qt::StrongFocus);
	
	/* We need to disable kactions that were plugged to the KateViewInternal in kateview.cpp.
	These actions include Key_Up, Key_Down, etc. */
	kate_edit_actions = view->findChild<KActionCollection*> ("edit_actions");
	if (!kate_edit_actions) {
		kate_edit_actions=view->actionCollection();
	}
	if (kate_edit_actions) {
		// make sure these actions never get triggered by a shortcut

		QList<QKeySequence> noshort;
		noshort.append (QKeySequence ());	// no primary
		noshort.append (QKeySequence ());	// no secondary
		noshort.append (QKeySequence ());	// for good measure

		QList<QAction*> keas = kate_edit_actions->actions ();
		for (int i = 0; i < keas.size (); ++i) {
			keas[i]->setShortcuts (noshort);
		}
	} else {
		RK_DO (qDebug ("Could not retrieve the katepart's edit action collection"), APP, DL_ERROR);
	}

	if (view->focusProxy ()) view->focusProxy()->installEventFilter(this);
	view->installEventFilter(this);

	doc->setModified (false);

	hinter = new RKFunctionArgHinter (this, view);
	
	setCaption (i18n ("R Console"));
	console_part = new RKConsolePart (this);
	setPart (console_part);
	setMetaInfo (shortCaption (), "rkward://page/rkward_console", RKSettings::PageConsole);
	initializeActivationSignals ();
	initializeActions (getPart ()->actionCollection ());

	nprefix = "> ";
	iprefix = "+ ";
	prefix = nprefix;
// KDE4: a way to do this?
//	doc->setUndoSteps (0);
	clear ();
	doc->setHighlightingMode ("R interactive session");

	commands_history = RKSettingsModuleConsole::loadCommandHistory ();
	commands_history_position = commands_history.constEnd ();

	current_command = 0;
	current_command_displayed_up_to = 0;
	tab_key_pressed_before = false;
	previous_chunk_was_piped = false;

// KDE4 TODO: workaround for KDE 4 pre-release versions. Hope this gets fixed before 4.0
// see http://lists.kde.org/?l=kwrite-devel&m=119721420603507&w=2
	setMinimumHeight (50);
}

RKConsole::~RKConsole () {
	RK_TRACE (APP);

	delete hinter;
	RKSettingsModuleConsole::saveCommandHistory (commands_history);
}

QAction* RKConsole::addProxyAction (const QString& actionName, const QString& label) {
	RK_TRACE (APP);
	RK_ASSERT (getPart ());
	RK_ASSERT (view);

	// katepart has more than one actionCollection
	QList<KActionCollection*> acs = view->findChildren<KActionCollection*>();
	acs.append (view->actionCollection ());

	QAction* found = 0;
	foreach (KActionCollection* ac, acs) {
		found = ac->action (actionName);
		if (found) break;
	}

	if (found) {
		QAction* ret = new KAction (getPart ());
		if (label.isEmpty ()) ret->setText (found->text ());
		else ret->setText (label);
		ret->setIcon (found->icon ());
		ret->setIconText (found->iconText ());
		ret->setToolTip (found->toolTip ());
		ret->setStatusTip (found->statusTip ());
		ret->setCheckable (found->isCheckable ());
		ret->setChecked (found->isChecked ());
		// TODO: ideally, we'd also relay enabledness, checked state, etc. That would probably require a separate class,
		// and is not currently needed for the actions that we copy
		connect (ret, SIGNAL (triggered(bool)), found, SLOT (trigger()));
		connect (ret, SIGNAL (toggled(bool)), found, SLOT (toggle()));

		getPart ()->actionCollection ()->addAction (actionName, ret);
		return ret;
	} else {
		return 0;
	}
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

	if (para < doc->lines () - 1 || pos < prefix.length ()) {	// not inside the last line?
		int key = e->key ();
		if ((key != Qt::Key_Shift) && (key != Qt::Key_Control) && (key != Qt::Key_Meta) && (key != Qt::Key_Alt)) {
			int t = (int) pos;					// adjust position before interpreting non-modifier keystroke
			if (prefix.length()>pos) t=prefix.length ();
			view->setCursorPosition (KTextEditor::Cursor (doc->lines () -1, t));
		}
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

	previous_chunk_was_piped = false;

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
		}
		cursorAtTheBeginning ();
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
		}
		cursorAtTheEnd ();
		return true;
	} else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
		hinter->hideArgHint ();
		addCommandToHistory (currentEditingLine ());
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
		*context = currentEditingLine ();
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

	QString current_line = currentEditingLine ();
	int current_line_num = doc->lines () - 1;
	int word_start;
	int word_end;
	int cursor_pos = currentCursorPositionInCommand ();
	if (cursor_pos < 0) cursor_pos = current_line.length ();
	RKCommonFunctions::getCurrentSymbolOffset (current_line, cursor_pos, false, &word_start, &word_end);
	QString current_symbol = current_line.mid (word_start, word_end - word_start);

	// as a very simple heuristic: If the current symbol starts with a quote, we should probably attempt file name completion, instead of symbol name completion
	if (current_symbol.startsWith ("\"") || current_symbol.startsWith ("\'") || current_symbol.startsWith ("`")) {
		KUrlCompletion comp (KUrlCompletion::FileCompletion);
		comp.setDir (QDir::currentPath ());
		comp.makeCompletion (current_symbol.mid (1));

		if (doTabCompletionHelper (current_line_num, current_line, word_start + 1, word_end, comp.allMatches ())) return;
	} else if (!current_symbol.isEmpty ()) {
		RObject::RObjectSearchMap map;
		RObjectList::getObjectList ()->findObjectsMatching (current_symbol, &map);

		if (doTabCompletionHelper (current_line_num, current_line, word_start, word_end, map.keys ())) return;
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

QString RKConsole::currentEditingLine () const {
	RK_TRACE (APP);

	if (current_command) return QString ();	// it doesn't count as "editing" line, if a command is currently being executed
	return (doc->line (doc->lines () - 1).mid (prefix.length ()));
}

void RKConsole::setCurrentEditingLine (const QString &command) {
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

	QString command = incomplete_command + currentEditingLine ();
	if (!input_buffer.isEmpty ()) {
		int last_line_end = input_buffer.lastIndexOf ('\n');
		if (last_line_end < 0) {
			last_line_end = 0;
			RK_ASSERT (false);
		}
		command.append (input_buffer.left (last_line_end));
		if (last_line_end < (input_buffer.size () - 1)) {
			input_buffer = input_buffer.mid (last_line_end + 1);
		} else {
			input_buffer.clear ();
		}
	} else {
		RK_ASSERT (!command.endsWith ('\n'));
		command.append ('\n');
	}

	current_command_displayed_up_to = incomplete_command.length ();
	setCurrentEditingLine (command.mid (current_command_displayed_up_to, command.indexOf ('\n', current_command_displayed_up_to) - current_command_displayed_up_to));
	current_command_displayed_up_to += currentEditingLine ().length ();
	skip_command_display_lines = incomplete_command.count ('\n') + 1;	// incomplete command, and first line have already been shown.

	doc->insertLine (doc->lines (), QString ());
	if (!command.isEmpty ()) {
		current_command = new RCommand (command, RCommand::User | RCommand::Console, QString (), this);
		RKGlobals::rInterface ()->issueCommand (current_command);
		interrupt_command_action->setEnabled (true);
	} else {
		showPrompt ();
		tryNextInBuffer ();
	}
}

void RKConsole::commandsListUp (bool context_sensitive) {
	RK_TRACE (APP);

	// if we are at the last line, i.e. not yet navigating the command history, store the current command
	if (commands_history.constEnd () == commands_history_position) history_editing_line = currentEditingLine ();

	if (context_sensitive) {
		if (command_edited) {
			command_history_context = currentEditingLine ();
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
		setCurrentEditingLine (*commands_history_position);
	} else {
		KApplication::kApplication ()->beep ();
	}
}

void RKConsole::commandsListDown (bool context_sensitive) {
	RK_TRACE (APP);

	if (context_sensitive) {
		if (command_edited) {
			command_history_context = currentEditingLine ();
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

	while (++commands_history_position != commands_history.constEnd ()) {
		if ((!context_sensitive) || (*commands_history_position).startsWith (command_history_context)) { // we found a match or next line
			break;
		}
	}

	if (commands_history.constEnd () == commands_history_position) setCurrentEditingLine (history_editing_line);
	else setCurrentEditingLine (*commands_history_position);
}

void RKConsole::rCommandDone (RCommand *command) {
	RK_TRACE (APP);

	current_command = 0;

	if (command->errorSyntax () && command->error ().isEmpty ()) {
		doc->insertLine (doc->lines () - 1, i18n ("Syntax error\n"));
	}

	if (command->errorIncomplete ()) {
		prefix = iprefix;
		incomplete_command = command->remainingCommand ();
	} else {
		prefix = nprefix;
		incomplete_command.clear ();
	}

	commands_history_position = commands_history.constEnd ();
	showPrompt ();
	tryNextInBuffer ();
}

void RKConsole::newOutput (RCommand *, ROutput *output) {
	RK_TRACE (APP);

	int start_line = doc->lines () -1;
	doc->insertText (doc->documentEnd (), output->output);

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
// We remove the superflous lines in chunks of 20 while handling output for better performance. Later, in showPrompt(), we trim down to the correct size.
		if (c > (RKSettingsModuleConsole::maxConsoleLines () + 20)) {
// KDE4: does the setUpdatesEnabled (false) still affect performance?
			view->setUpdatesEnabled (false);		// major performance boost while removing lines!
			doc->removeText (KTextEditor::Range (0, 0, c - RKSettingsModuleConsole::maxConsoleLines (), 0));
			view->setUpdatesEnabled (true);
		}
	}
	cursorAtTheEnd ();
}

void RKConsole::userCommandLineIn (RCommand* cmd) {
	RK_TRACE (APP);
	RK_ASSERT (cmd == current_command);

	if (--skip_command_display_lines >= 0) return;

	QString line = cmd->command ().mid (current_command_displayed_up_to + 1);
	line = line.section ('\n', 0, 0) + '\n';
	current_command_displayed_up_to += line.length ();
	if (line.length () < 2) return;		// omit empty lines (esp. the trailing newline of the command!)

	prefix = iprefix;
	showPrompt ();
	setCurrentEditingLine (line);
}

void RKConsole::submitBatch (const QString &batch) {
	RK_TRACE (APP);

	previous_chunk_was_piped = false;
	input_buffer.append (batch);
	if (!current_command) {		// pasting into the middle of the command line
		QString line = currentEditingLine ();
		int pos = currentCursorPositionInCommand ();
		if (pos >= 0) {
			setCurrentEditingLine (line.left (pos));
			input_buffer.append (line.mid (pos));
		}
	}
	if (!current_command) tryNextInBuffer ();
}

void RKConsole::showPrompt () {
	RK_TRACE (APP);

	if ((!doc->lines ()) || (!doc->line (doc->lines () - 1).isEmpty ())) {
		if (RKSettingsModuleConsole::maxConsoleLines ()) {
			uint c = doc->lines();
			if (c > RKSettingsModuleConsole::maxConsoleLines ()) {
				// KDE4 TODO: setUpdatesEnabled(false) still faster?
				view->setUpdatesEnabled (false);
				doc->removeText (KTextEditor::Range (0, 0, c - RKSettingsModuleConsole::maxConsoleLines (), 0));
				view->setUpdatesEnabled (true);
			}
		}
		doc->insertLine (doc->lines (), QString ());
	}
	doc->insertText (KTextEditor::Cursor (doc->lines () - 1, 0), prefix);
//	doc->insertLine (doc->lines (), prefix);		// somehow, it seems to be safer to do this after removing superfluous lines, than before
	cursorAtTheEnd ();
}

void RKConsole::tryNextInBuffer () {
	RK_TRACE (APP);

	if (!input_buffer.isEmpty ()) {
		if (input_buffer.contains ('\n')) {
			submitCommand ();	// will submit and clear the buffer
			return;
		} else {
			setCurrentEditingLine (currentEditingLine () + input_buffer);
			input_buffer.clear ();
		}
	}
	interrupt_command_action->setEnabled (false);
}

bool RKConsole::isBusy () const {
	if (current_command) return true;
	if (!incomplete_command.isEmpty ()) return true;
	if (!currentEditingLine ().isEmpty ()) return true;
	if (!input_buffer.isEmpty ()) return true;
	return false;
}

void RKConsole::paste () {
	RK_TRACE (APP);
	QClipboard *cb = QApplication::clipboard ();
	submitBatch (cb->text ());
}

void RKConsole::clear () {
	RK_TRACE (APP);
	doc->clear ();
	showPrompt ();
}

void RKConsole::addCommandToHistory (const QString &command) {
	RK_TRACE (APP);
	if ((!command.isEmpty ()) && (commands_history.isEmpty () || commands_history.last() != command)) { // don't add empty or duplicate lines
		commands_history.append (command);
	}

	if (RKSettingsModuleConsole::maxHistoryLength ()) {
		uint c = commands_history.count ();
		for (uint ui = c; ui > RKSettingsModuleConsole::maxHistoryLength (); --ui) {
			commands_history.pop_front ();
		}
	}

	history_editing_line = QString ();
	commands_history_position = commands_history.constEnd ();
	command_edited = false;
}

void RKConsole::setCommandHistory (const QStringList &new_history, bool append) {
	RK_TRACE (APP);

	if (append) commands_history.append (new_history);
	else commands_history = new_history;

	addCommandToHistory (QString ());	// side-effect of checking history length
}

void RKConsole::userLoadHistory (const KUrl &_url) {
	RK_TRACE (APP);

	KUrl url = _url;
	if (url.isEmpty ()) {
		url = KFileDialog::getOpenUrl (KUrl (), i18n ("*.Rhistory|R history files (*.Rhistory)\n*|All files (*)"), this, i18n ("Select command history file to load"));
		if (url.isEmpty ()) return;
	}

	QString tempfile;
	KIO::NetAccess::download (url, tempfile, this);

	QFile file (tempfile);
	if (!file.open (QIODevice::Text | QIODevice::ReadOnly)) return;
	setCommandHistory (QString (file.readAll ()).split ('\n', QString::SkipEmptyParts), false);
	file.close ();

	KIO::NetAccess::removeTempFile (tempfile);
}

void RKConsole::userSaveHistory (const KUrl &_url) {
	RK_TRACE (APP);

	KUrl url = _url;
	if (url.isEmpty ()) {
		url = KFileDialog::getSaveUrl (KUrl (), i18n ("*.Rhistory|R history files (*.Rhistory)\n*|All files (*)"), this, i18n ("Select filename to save command history")
#if KDE_IS_VERSION(4,4,0)
		                                    , KFileDialog::ConfirmOverwrite
#endif
		                                   );
		if (url.isEmpty ()) return;
	}

	KTemporaryFile tempfile;
	tempfile.open ();
	tempfile.write (QString (commandHistory ().join ("\n") + "\n").toLocal8Bit ().data ());
	tempfile.close ();

	KIO::NetAccess::upload (tempfile.fileName (), url, this);
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

int RKConsole::currentCursorPositionInCommand(){
	RK_TRACE (APP);

	KTextEditor::Cursor c = view->cursorPosition ();
	if (c.line () < (doc->lines () - 1)) return -1;
	return (c.column () - prefix.length());
}

void RKConsole::resetConsole () {
	RK_TRACE (APP);
	RK_DO (qDebug("received interrupt signal in console"), APP, DL_DEBUG);

	input_buffer.clear ();
	if (current_command) {
		RKGlobals::rInterface ()->cancelCommand (current_command);
	} else {
		prefix = nprefix;
		incomplete_command.clear ();

		showPrompt ();
	}
}

void RKConsole::runSelection () {
	RK_TRACE (APP);

	pipeUserCommand (cleanedSelection ());
}

void RKConsole::showContextHelp () {
	RK_TRACE (APP);
	RKHelpSearchWindow::mainHelpSearch ()->getContextHelp (currentEditingLine (), currentCursorPositionInCommand ());
}

void RKConsole::initializeActions (KActionCollection *ac) {
	RK_TRACE (APP);

	context_help_action = RKStandardActions::functionHelp (this, this, SLOT(showContextHelp()));

	run_selection_action = RKStandardActions::runSelection (this, this, SLOT (runSelection()));

	interrupt_command_action = ac->addAction ("interrupt", this, SLOT (resetConsole()));
	interrupt_command_action->setText (i18n ("Interrupt running command"));
	interrupt_command_action->setShortcut (Qt::ControlModifier + Qt::Key_C);
	interrupt_command_action->setIcon (KIcon ("media-playback-stop"));
	interrupt_command_action->setEnabled (false);

	copy_action = ac->addAction ("rkconsole_copy", this, SLOT (copy()));
	copy_action->setText (i18n ("Copy selection cleaned"));

	copy_literal_action = ac->addAction ("rkconsole_copy_literal", this, SLOT (literalCopy()));
	copy_literal_action->setText (i18n ("Copy selection literally"));

	RKStandardActions::pasteSpecial (this, this, SLOT (submitBatch(const QString&)));

	ac->addAction (KStandardAction::Clear, "rkconsole_clear", this, SLOT (clear()));
	paste_action = ac->addAction (KStandardAction::Paste, "rkconsole_paste", this, SLOT (paste()));

	addProxyAction ("file_print", i18n ("Print Console"));
	addProxyAction ("file_export_html");
	addProxyAction ("view_dynamic_word_wrap");
	addProxyAction ("view_inc_font_sizes");
	addProxyAction ("view_dec_font_sizes");

	KAction *action = ac->addAction ("loadhistory", this, SLOT (userLoadHistory ()));
	action->setText (i18n ("Import command history..."));
	action = ac->addAction ("savehistory", this, SLOT (userSaveHistory ()));
	action->setText (i18n ("Export command history..."));
}

void RKConsole::pipeUserCommand (const QString &command) {
	RK_TRACE (APP);

	if (RKSettingsModuleConsole::pipeUserCommandsThroughConsole ()) {
		RKConsole::mainConsole ()->pipeCommandThroughConsoleLocal (command);
	} else {
		RCommand *cmd = new RCommand (command, RCommand::User);
		RKGlobals::rInterface ()->issueCommand (cmd);
	}
}

void RKConsole::pipeCommandThroughConsoleLocal (const QString &command_string) {
	RK_TRACE (APP);

	activate (false);
	if (isBusy () && (!previous_chunk_was_piped)) {
		int res = KMessageBox::questionYesNoCancel (this, i18n ("You have configured RKWard to pipe script editor commands through the R Console. However, another command is currently active in the console. Do you want to append it to the command in the console, or do you want to reset the console, first? Press cancel if you do not wish to run the new command, now."), i18n ("R Console is busy"), KGuiItem (i18n ("Append")), KGuiItem (i18n ("Reset, then submit")));
		if (res == KMessageBox::No) {
			resetConsole ();
		} else if (res != KMessageBox::Yes) {
			return;
		}
	}
	if (RKSettingsModuleConsole::addPipedCommandsToHistory() != RKSettingsModuleConsole::DontAdd) {
		QStringList lines = command_string.split ('\n', QString::SkipEmptyParts);
		if ((RKSettingsModuleConsole::addPipedCommandsToHistory() == RKSettingsModuleConsole::AlwaysAdd) || (lines.count () == 1)) {
			for (int i = 0; i < lines.count (); ++i) {
				addCommandToHistory (lines[i]);
			}
		}
	}
	cursorAtTheEnd ();
	submitBatch (command_string + '\n');
	previous_chunk_was_piped = true;
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

void RKConsole::activate (bool with_focus) {
	RK_TRACE (APP);

	// see http://www.mail-archive.com/rkward-devel@lists.sourceforge.net/msg00933.html
	if (with_focus) view->removeSelection ();
	RKMDIWindow::activate (with_focus);
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
