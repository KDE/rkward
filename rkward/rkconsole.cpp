/*
rkconsole - This file is part of RKWard (https://rkward.kde.org). Created: Thu Aug 19 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
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
#include <QStandardPaths>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QMimeData>
#include <QAction>
#include <QFileDialog>
#include <QApplication>

#include <KLocalizedString>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kshellcompletion.h>
#include <ktexteditor/editor.h>
#include <ktexteditor_version.h>
#include <kxmlguifactory.h>
#include <kio/filecopyjob.h>
#include <KJobWidgets>
#include <KJobUiDelegate>

#include "rkward.h"
#include "windows/rkhelpsearchwindow.h"
#include "windows/rkcodecompletion.h"
#include "windows/rktexthints.h"
#include "windows/katepluginintegration.h"
#include "rbackend/rkrinterface.h"
#include "settings/rksettings.h"
#include "settings/rksettingsmoduleconsole.h"
#include "settings/rkrecenturls.h"
#include "misc/rkcommonfunctions.h"
#include "misc/rkstandardicons.h"
#include "misc/rkstandardactions.h"
#include "core/robjectlist.h"
#include "core/rfunctionobject.h"

#include "debug.h"

// static
RKConsole* RKConsole::main_console = nullptr;

RKConsole::RKConsole (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, RKMDIWindow::ConsoleWindow, tool_window, name), commands_history (false, false) {
	RK_TRACE (APP);

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);

	// create a Kate-part as command-editor
	KTextEditor::Editor* editor = KTextEditor::Editor::instance ();
	if (!editor) {
		RK_ASSERT (false);
		KMessageBox::error (this, i18n ("The 'katepart' component could not be loaded. RKWard cannot run without katepart, and will exit, now. Please install katepart, and try again."), i18n ("'katepart' component could not be found"));
		exit (1);
	}

	doc = editor->createDocument (this);
	// even though the R Console is not really a document window, it is important for it to be constructed with a proper main window.
	// E.g. argument hints don't get the correct window parent, otherwise
	view = doc->createView (this, RKWardMainWindow::getMain()->katePluginIntegration()->mainWindow ()->mainWindow());
	layout->addWidget (view);
	view->setStatusBarEnabled (false);
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
		RK_DEBUG (APP, DL_ERROR, "Could not retrieve the katepart's edit action collection");
	}

	if (view->focusProxy ()) view->focusProxy()->installEventFilter(this);
	view->installEventFilter(this);
	auto manager = new RKCompletionManager (view, RKSettingsModuleConsole::completionSettings());  // Must be instantiated _after_ our event filter, so that it will apply its filter first
	manager->setLinePrefixes(nprefix, iprefix);
	new RKTextHints(view, RKSettingsModuleConsole::completionSettings());

	doc->setModified (false);

	setCaption (i18n ("R Console"));
	console_part = new RKConsolePart (this);
	setPart (console_part);
	setMetaInfo (shortCaption (), QUrl ("rkward://page/rkward_console"), RKSettings::PageConsole);
	initializeActivationSignals ();
	initializeActions (getPart ()->actionCollection ());
	QAction* action = RKSettingsModuleConsole::showMinimap()->makeAction(this, i18n("Scrollbar minimap"), [this](bool val) { view->setConfigValue("scrollbar-minimap", val); });
	getPart()->actionCollection()->addAction("view_show_minimap", action);
	action = RKSettingsModuleConsole::wordWrap()->makeAction(this, i18n("Dynamic word wrap"), [this](bool val) { view->setConfigValue("dynamic-word-wrap", val); });
	getPart()->actionCollection()->addAction("view_dynamic_word_wrap", action);

	nprefix = "> ";
	iprefix = "+ ";
	prefix = nprefix;
// KDE4: a way to do this?
//	doc->setUndoSteps (0);
	clear ();
	RKCommandHighlighter::setHighlighting (doc, RKCommandHighlighter::RInteractiveSession);

	commands_history.setHistory (RKSettingsModuleConsole::loadCommandHistory ());

	current_command = nullptr;
	current_command_displayed_up_to = 0;
	tab_key_pressed_before = false;
	previous_chunk_was_piped = false;

// KDE4 TODO: workaround for KDE 4 pre-release versions. Hope this gets fixed before 4.0
// see http://lists.kde.org/?l=kwrite-devel&m=119721420603507&w=2
	setMinimumHeight (50);
}

RKConsole::~RKConsole () {
	RK_TRACE (APP);

	if (this == main_console) main_console = nullptr;
	RKSettingsModuleConsole::saveCommandHistory (commands_history.getHistory ());
}

QAction* RKConsole::addProxyAction (const QString& actionName, const QString& label) {
	RK_TRACE (APP);
	RK_ASSERT (getPart ());
	RK_ASSERT (view);

	// katepart has more than one actionCollection
	QList<KActionCollection*> acs = view->findChildren<KActionCollection*>();
	acs.append (view->actionCollection ());

	QAction* found = nullptr;
	for (KActionCollection* ac : std::as_const(acs)) {
		found = ac->action (actionName);
		if (found) break;
	}

	if (found) {
		QAction* ret = new QAction (getPart ());
		if (label.isEmpty ()) ret->setText (found->text ());
		else ret->setText (label);
		ret->setIcon (found->icon ());
		ret->setIconText (found->iconText ());
		ret->setToolTip (found->toolTip ());
		ret->setWhatsThis(found->statusTip ());
		ret->setCheckable (found->isCheckable ());
		ret->setChecked (found->isChecked ());
		// TODO: ideally, we'd also relay enabledness, checked state, etc. That would probably require a separate class,
		// and is not currently needed for the actions that we copy
		connect (ret, &QAction::triggered, found, &QAction::trigger);
		connect (ret, &QAction::toggled, found, &QAction::toggle);

		getPart ()->actionCollection ()->addAction (actionName, ret);
		return ret;
	} else {
		return nullptr;
	}
}

void RKConsole::triggerEditAction (const QString &name) {
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
	int key = e->key ();
	bool is_modifier_key = ((key == Qt::Key_Shift) || (key == Qt::Key_Control) || (key == Qt::Key_Meta) || (key == Qt::Key_Alt));
	
	if (view->selection ()) {
		KTextEditor::Range selrange = view->selectionRange ();
		if (selrange.start ().line () < (doc->lines () - 1) || selrange.start ().column () < prefix.length ()) {
			// There is a selection outside the command line
			// If it's a modifier key, that's quite ok. Otherwise, we need to take off the selection, then proceed to handling the key press
			if (is_modifier_key) return true;
			view->setSelection (KTextEditor::Range ());
		}
	}

	if (para < doc->lines () - 1 || pos < prefix.length ()) {	// not inside the last line?
		if (!is_modifier_key) cursorAtTheEnd ();	// adjust position before interpreting non-modifier keystroke

		// KDE 4.13.3 (may have started, earlier): Apparently, cursor adjustment does not take effect, immediately.
		if (para < doc->lines () - 1 || pos < prefix.length ()) { // still not inside the last line?
			// HACK ish workaround:
			// qApp->postEvent (this, e); // not quite as easy to re-post, as event will be deleted
			return true;	// at least prevent kate part form interpreting it
		}
	}

	if (current_command) {
		e->ignore ();
		return true;
	}

	previous_chunk_was_piped = false;

	// Apparently, on MacOSX, additional modifiers may sometimes be present (KeypadModifier?), which we want to ignore.
	const int modifier_mask = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier;
	Qt::KeyboardModifiers modifier = e->modifiers () & modifier_mask;

	if ((key == Qt::Key_Up) || (key == Qt::Key_Down)) {
		if (key == Qt::Key_Up) commandsListUp (RKSettingsModuleConsole::shouldDoHistoryContextSensitive (modifier));
		else commandsListDown (RKSettingsModuleConsole::shouldDoHistoryContextSensitive (modifier));
		return true;
	}

	if (key == Qt::Key_Home) {
		if (modifier == Qt::ShiftModifier) {
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
	} else if (key == Qt::Key_End) {
		if (modifier == Qt::ShiftModifier) {
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
	} else if (key == Qt::Key_Enter || key == Qt::Key_Return) {
		commands_history.append (currentEditingLine ());
		submitCommand ();
		return true;
	} else if (key == Qt::Key_Left){
		if (pos <= prefix.length ()) return true;

		if (modifier == Qt::NoModifier) setCursorClear (para, pos - 1);
		else if (modifier == Qt::ShiftModifier) triggerEditAction ("select_char_left");
		else if (modifier == Qt::ControlModifier) triggerEditAction ("word_left");
		else if (modifier == (Qt::ControlModifier | Qt::ShiftModifier)) triggerEditAction ("select_word_left");
		else return false;

		return true;
	} else if (key == Qt::Key_Right){
		if (pos >= doc->lineLength (para)) return true;

		if (modifier == Qt::NoModifier) setCursorClear (para, pos + 1);
		else if (modifier == Qt::ShiftModifier) triggerEditAction ("select_char_right");
		else if (modifier == Qt::ControlModifier) triggerEditAction ("word_right");
		else if (modifier == (Qt::ControlModifier | Qt::ShiftModifier)) triggerEditAction ("select_word_right");
		else return false;

		return true;
	} else if (key == Qt::Key_Backspace){
		if(pos<=prefix.length ()) return true;

		if (view->selection ()) {
			doc->removeText (view->selectionRange ());
			view->removeSelection ();
		} else {
			doc->removeText (KTextEditor::Range (para, pos, para, pos-1));
		}
		return true;
	} else if (key == Qt::Key_Delete) {
		// not currently handled by an action in kateview internal, but better not rely on that fact!
		if (view->selection ()) {
			doc->removeText (view->selectionRange ());
			view->removeSelection ();
		} else {
			doc->removeText (KTextEditor::Range (para, pos, para, pos + 1));
		}
		return true;
	}

	return false;
}

QString RKConsole::provideContext (int line_rev) {
	RK_TRACE (COMMANDEDITOR);

	QString ret;
	if (line_rev == 0) ret = currentEditingLine ().left (currentCursorPositionInCommand ()); 
	else if (!incomplete_command.isEmpty ()) {
		QStringList lines = incomplete_command.split ('\n');
		if (lines.size () > line_rev) {
			ret = lines[lines.size () - line_rev - 1];
		}
	}
	return ret;
}

bool RKConsole::eventFilter (QObject *o, QEvent *e) {
	if (o == getPart ()) {
		return RKMDIWindow::eventFilter (o, e);
	}

	if ((e->type () == QEvent::KeyPress) || (e->type () == QEvent::Shortcut)) {
		QKeyEvent *k = static_cast<QKeyEvent *>(e);
		return (handleKeyPress (k));
	} else if (e->type () == QEvent::MouseButtonPress) {
		// we seem to need this, as the kateview will swallow the contextMenuEvent, otherwise
		QMouseEvent *m = (QMouseEvent *)e;
		if (m->button() == Qt::RightButton) {
			QPoint pos = m->globalPosition().toPoint();
			qApp->sendEvent(this, new QContextMenuEvent(QContextMenuEvent::Mouse, mapFromGlobal(pos), pos));
			return (true);
		}
	} else if (e->type () == QEvent::MouseButtonRelease){
		QMouseEvent *m = (QMouseEvent *)e;
		if (m->button() == Qt::MiddleButton) {
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
	} else if (e->type () == QEvent::DragMove || e->type () == QEvent::Drop) {
		QDropEvent* me = static_cast<QDropEvent*> (e);  // NOTE: QDragMoveEvent inherits from QDropEvent

		// Widget of the event != view. The position returned by coordinatesToCursor seems to be off by around two chars. Icon border?
		// We try to map it back to the view, correctly.
		QWidget *rec = dynamic_cast<QWidget*> (o);
		if (!o) rec = view;
		KTextEditor::Cursor pos = view->coordinatesToCursor(rec->mapTo(view, me->position().toPoint()));

		bool in_last_line = (pos.line () == doc->lines () - 1) && (pos.column () >= prefix.length ());
		if (!in_last_line) {
			e->ignore ();
			return true;
		} else {
			if (e->type () == QEvent::DragMove) {
				// Not sure why this is needed, here, but without this, the move will remain permanently unacceptable,
				// once it has been ignored, above, once. KF5 5.9.0
				e->accept ();
				// But also _not_ filtering it.
			} else {
				// We have to prevent the katepart from _moving_ the text in question. Thus, instead we fake a paste.
				// This does mean, we don't support movements within the last line, either, but so what.
				view->setCursorPosition (pos);
				submitBatch (me->mimeData ()->text ());
				me->ignore ();
				return true;
			}
		}
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
		command.append(QStringView{input_buffer}.left(last_line_end));
		if (last_line_end < (input_buffer.size() - 1)) {
			input_buffer = input_buffer.mid(last_line_end + 1);
		} else {
			input_buffer.clear();
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
	output_cursor = doc->documentEnd();
	if (!command.isEmpty ()) {
		current_command = new RCommand (command, RCommand::User | RCommand::Console);
		connect(current_command->notifier(), &RCommandNotifier::commandOutput, this, &RKConsole::newOutput);
		connect(current_command->notifier(), &RCommandNotifier::commandLineIn, this, &RKConsole::userCommandLineIn);
		current_command->whenFinished(this, [this](RCommand *command) {
			RK_TRACE(APP);
			current_command = nullptr;

			if (command->errorSyntax() && command->error().isEmpty()) {
				doc->insertLine(doc->lines() - 1, i18n("Syntax error\n"));
			}
			if (command->errorIncomplete()) {
				prefix = iprefix;
				incomplete_command = command->remainingCommand();
			} else {
				prefix = nprefix;
				incomplete_command.clear();
			}

			commands_history.goToEnd();
			showPrompt();
			tryNextInBuffer();
		});
		RInterface::issueCommand (current_command);
		interrupt_command_action->setEnabled (true);
	} else {
		showPrompt ();
		tryNextInBuffer ();
	}
}

void RKConsole::commandsListUp (bool context_sensitive) {
	RK_TRACE (APP);

	bool found = commands_history.up (context_sensitive, currentEditingLine ());
	if (found) setCurrentEditingLine (commands_history.current ());
	else qApp->beep ();
}

void RKConsole::commandsListDown (bool context_sensitive) {
	RK_TRACE (APP);

	bool found = commands_history.down (context_sensitive, currentEditingLine ());
	if (found) setCurrentEditingLine (commands_history.current ());
	else qApp->beep ();
}

void RKConsole::rawWriteLine(const QString& line, QChar line_end) {
	int existing_line_length = doc->lineLength(output_cursor.line());
	if (output_cursor.column() < existing_line_length) {
		int overwrite_end = qMin(existing_line_length, output_cursor.column() + line.length());
		doc->removeText(KTextEditor::Range(output_cursor, KTextEditor::Cursor(output_cursor.line(), overwrite_end)));
	}
	doc->insertText(output_cursor, line);
	output_cursor.setColumn(output_cursor.column() + line.length());
	if (line_end == '\n') {
		output_cursor.setColumn(doc->lineLength(output_cursor.line()));
		doc->insertText(output_cursor, "\n");
		output_cursor.setColumn(0);
		output_cursor.setLine(output_cursor.line() + 1);
	} else if (line_end == '\r') {
		output_cursor.setColumn(0);
	}
}

void RKConsole::newOutput (RCommand *command, const ROutput *output) {
	RK_TRACE (APP);

	int first_line = doc->lines () -1;
	QString popped_line;
	// TODO: rewrite utilizing output_cursor;
	if (!command) {
		// spontanteous R output, to be inserted _above_ the current command
		// as a shortcut, we pop the last line, and reinsert in, later
		popped_line = doc->line(doc->lines() - 1);
		doc->removeLine(doc->lines() - 1);
	}

	// split by and handle carriage returns (important for progress bars)
	const QString out = output->output;
	int string_pos = -1;
	int start_pos = 0;
	int end_pos = out.length();
	while (++string_pos < end_pos) {
		auto c = out.at(string_pos);
		if (c == '\n' || c == '\r') {
			rawWriteLine(out.mid(start_pos, string_pos - start_pos), c);
			start_pos = string_pos+1;
		}
	}
	if (start_pos < end_pos) rawWriteLine(out.mid(start_pos, string_pos - start_pos + 1), ' ');

	int end_line = doc->lines () -1;
	if (output->type != ROutput::Output || (!command)) {
		for (int line = first_line; line < end_line; ++line) {
			doc->addMark (line, command ? KTextEditor::Document::BreakpointActive : KTextEditor::Document::BreakpointDisabled);
		}
	}

	if (RKSettingsModuleConsole::maxConsoleLines ()) {
		uint c = (uint) doc->lines();
// We remove the superfluous lines in chunks of 20 while handling output for better performance. Later, in showPrompt(), we trim down to the correct size.
		if (c > (RKSettingsModuleConsole::maxConsoleLines () + 20)) {
// KDE4: does the setUpdatesEnabled (false) still affect performance?
			view->setUpdatesEnabled (false);		// major performance boost while removing lines!
			doc->removeText (KTextEditor::Range (0, 0, c - RKSettingsModuleConsole::maxConsoleLines (), 0));
			output_cursor.setLine(output_cursor.line() - c + RKSettingsModuleConsole::maxConsoleLines());
			view->setUpdatesEnabled (true);
		}
	}

	if (!command) {
		doc->insertLine(doc->lines(), popped_line);
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
	output_cursor = doc->documentEnd();
}

void RKConsole::submitBatch (const QString &batch) {
	RK_TRACE (APP);

	previous_chunk_was_piped = false;
	input_buffer.append (batch);
	if (!current_command) {		// pasting into the middle of the command line
		QString line = currentEditingLine ();
		int pos = currentCursorPositionInCommand ();
		if (pos >= 0) {
			setCurrentEditingLine(line.left(pos));
			input_buffer.append(QStringView{line}.mid(pos));
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
	interrupt_command_action->setEnabled (!incomplete_command.isEmpty ());
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

void RKConsole::setCommandHistory (const QStringList &new_history, bool append) {
	RK_TRACE (APP);

	commands_history.setHistory (new_history, append);
}

void RKConsole::userLoadHistory (const QUrl &_url) {
	RK_TRACE (APP);

	QUrl url = _url;
	if (url.isEmpty ()) {
		url = QFileDialog::getOpenFileUrl (this, i18n ("Select command history file to load"), RKRecentUrls::mostRecentUrl(RKRecentUrls::scriptsId()).adjusted(QUrl::RemoveFilename), i18n ("R history files [*.Rhistory](*.Rhistory);;All files [*](*)"));
		if (url.isEmpty ()) return;
	}

	QTemporaryFile *tmpfile = nullptr;
	QString filename;
	if (!url.isLocalFile ()) {
		tmpfile = new QTemporaryFile (this);
		KIO::Job* getjob = KIO::file_copy (url, QUrl::fromLocalFile (tmpfile->fileName()));
		KJobWidgets::setWindow (getjob, RKWardMainWindow::getMain ());
		if (!getjob->exec ()) {
			getjob->uiDelegate ()->showErrorMessage();
			delete (tmpfile);
			return;
		}
		filename = tmpfile->fileName ();
	} else {
		filename = url.toLocalFile ();
	}

	QFile file (filename);
	if (!file.open (QIODevice::Text | QIODevice::ReadOnly)) return;
	setCommandHistory (QString (file.readAll ()).split ('\n', Qt::SkipEmptyParts), false);
	file.close ();

	delete (tmpfile);
}

void RKConsole::userSaveHistory (const QUrl &_url) {
	RK_TRACE (APP);

	QUrl url = _url;
	if (url.isEmpty ()) {
		url = QFileDialog::getSaveFileUrl (this, i18n ("Select filename to save command history"), QUrl (), i18n ("R history files [*.Rhistory] (*.Rhistory);;All files [*] (*)"));
		if (url.isEmpty ()) return;
	}

	QTemporaryFile tempfile;
	tempfile.open ();
	tempfile.write (QString (commandHistory ().join ("\n") + '\n').toLocal8Bit ().data ());
	tempfile.close ();

	KIO::Job* getjob = KIO::file_copy (QUrl::fromLocalFile (tempfile.fileName()), url);
	KJobWidgets::setWindow (getjob, RKWardMainWindow::getMain ());
	if (!getjob->exec ()) {
		getjob->uiDelegate ()->showErrorMessage();
		return;
	}
}

QString RKConsole::cleanSelection (const QString &origin) {
	RK_TRACE (APP);

	QString ret;
	ret.reserve (origin.length ());
	const QStringList lines = origin.split ('\n');
	for (const QString& line : lines) {
		if (line.startsWith(nprefix)) {
			ret.append(QStringView{line}.mid(nprefix.length()));
		} else if (line.startsWith(iprefix)) {
			ret.append(QStringView{line}.mid(iprefix.length()));
		} else {
			ret.append(line);
		}
		ret.append ('\n');
	}

	return ret;
}

void RKConsole::copyCommands () {
	RK_TRACE (APP);

	KTextEditor::Range sel = view->selectionRange ();
	if (!sel.isValid ()) return;

	// we use this somewhat cumbersome (and inefficient) approach as it should also be correct in case of blockwise selections
	QStringList command_lines = view->selectionText ().split ('\n');
	int i = 0;
	for (int l = sel.start ().line (); l <= sel.end ().line (); ++l) {
		QString line = doc->line (l);
		if (!(line.startsWith (iprefix) || line.startsWith (nprefix))) {
			command_lines.removeAt (i);
			--i;
		}
		++i;
	}
	QApplication::clipboard()->setText (cleanSelection (command_lines.join ("\n")));
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
	RK_DEBUG (APP, DL_DEBUG, "received interrupt signal in console");

	input_buffer.clear ();
	if (current_command) {
		RInterface::instance()->cancelCommand(current_command);
	} else {
		prefix = nprefix;
		incomplete_command.clear ();

		showPrompt ();
		tryNextInBuffer ();		// well, the buffer is empty, but this also sets the interrupt_command_action's state, etc.
	}
}

void RKConsole::runSelection () {
	RK_TRACE (APP);

	pipeUserCommand (cleanSelection (view->selectionText ()));
}

void RKConsole::copyLinesToOutput () {
	RK_TRACE (APP);

	RKCommandHighlighter::copyLinesToOutput (view, RKCommandHighlighter::RInteractiveSession);
}

void RKConsole::currentHelpContext (QString* symbol, QString* package) {
	RK_TRACE (APP);
	Q_UNUSED (package);

	if (view->selection()) {
		*symbol = view->selectionText();
	} else {
		*symbol = RKCommonFunctions::getCurrentSymbol (currentEditingLine (), currentCursorPositionInCommand ());
	}
}

void RKConsole::initializeActions (KActionCollection *ac) {
	RK_TRACE (APP);
#ifdef Q_OS_MACOS
#	define REAL_CTRL_KEY Qt::MetaModifier
#	define REAL_CMD_KEY Qt::ControlModifier
#else
#	define REAL_CTRL_KEY Qt::ControlModifier
#	define REAL_CMD_KEY Qt::MetaModifier
#endif
	RKStandardActions::copyLinesToOutput (this, this, SLOT (copyLinesToOutput()));
	RKStandardActions::functionHelp (this, this);
	RKStandardActions::onlineHelp (this, this);
	run_selection_action = RKStandardActions::runCurrent(this, this, SLOT (runSelection()));

	interrupt_command_action = ac->addAction("interrupt", this, &RKConsole::resetConsole);
	interrupt_command_action->setText(i18n("Interrupt running command"));
	ac->setDefaultShortcut(interrupt_command_action, REAL_CTRL_KEY | Qt::Key_C);
	interrupt_command_action->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionInterrupt));
	interrupt_command_action->setEnabled(false);

	copy_literal_action = ac->addAction("rkconsole_copy_literal", this, &RKConsole::literalCopy);
	ac->setDefaultShortcut(copy_literal_action, REAL_CMD_KEY | Qt::Key_C);
	copy_literal_action->setText(i18n("Copy selection literally"));

	copy_commands_action = ac->addAction("rkconsole_copy_commands", this, &RKConsole::copyCommands);
	copy_commands_action->setText(i18n("Copy commands, only"));

	RKStandardActions::pasteSpecial (this, this, SLOT (submitBatch(QString)));

	ac->addAction(KStandardAction::Clear, "rkconsole_clear", this, &RKConsole::clear);
	paste_action = ac->addAction(KStandardAction::Paste, "rkconsole_paste", this, &RKConsole::paste);

	addProxyAction ("file_print", i18n ("Print Console"));
	addProxyAction ("file_export_html");
	addProxyAction ("view_inc_font_sizes");
	addProxyAction ("view_dec_font_sizes");

	QAction *action = ac->addAction("loadhistory", this, [this](){ userLoadHistory(); });
	action->setText(i18n("Import command history..."));
	action = ac->addAction("savehistory", this, [this](){ userSaveHistory(); });
	action->setText(i18n("Export command history..."));
}

void RKConsole::pipeUserCommand (const QString &command) {
	RK_TRACE (APP);

	if (RKSettingsModuleConsole::pipeUserCommandsThroughConsole ()) {
		RKConsole::mainConsole ()->pipeCommandThroughConsoleLocal (command);
	} else {
		RCommand *cmd = new RCommand (command, RCommand::User);
		RInterface::issueCommand (cmd);
	}
}

void RKConsole::pipeCommandThroughConsoleLocal (const QString &command_string) {
	RK_TRACE (APP);

	activate (false);
	if (isBusy () && (!previous_chunk_was_piped)) {
		int res = KMessageBox::questionTwoActionsCancel (this, i18n ("You have configured RKWard to pipe script editor commands through the R Console. However, another command is currently active in the console. Do you want to append it to the command in the console, or do you want to reset the console, first? Press cancel if you do not wish to run the new command, now."), i18n ("R Console is busy"), KGuiItem (i18n ("Append")), KGuiItem (i18n ("Reset, then submit")));
		if (res == KMessageBox::SecondaryAction) {
			resetConsole ();
		} else if (res != KMessageBox::PrimaryAction) {
			return;
		}
	}
	if (RKSettingsModuleConsole::addPipedCommandsToHistory() != RKSettingsModuleConsole::DontAdd) {
		QStringList lines = command_string.split ('\n', Qt::SkipEmptyParts);
		if ((RKSettingsModuleConsole::addPipedCommandsToHistory() == RKSettingsModuleConsole::AlwaysAdd) || (lines.count () == 1)) {
			for (int i = 0; i < lines.count (); ++i) {
				commands_history.append (lines[i]);
			}
		}
	}
	cursorAtTheEnd ();
	submitBatch (command_string + '\n');
	previous_chunk_was_piped = true;
}

void RKConsole::insertSpontaneousROutput(ROutput* output) {
	RK_TRACE (APP);
	RK_ASSERT(!current_command);
	newOutput(nullptr, output);
}

void RKConsole::contextMenuEvent (QContextMenuEvent * event) {
	RK_TRACE (APP);

	copy_commands_action->setEnabled (view->selection ());
	copy_literal_action->setEnabled (view->selection ());
	run_selection_action->setEnabled (view->selection ());

	console_part->showPopupMenu(event->globalPos());

	run_selection_action->setEnabled (true);
	copy_literal_action->setEnabled (true);
	copy_commands_action->setEnabled (true);
}

void RKConsole::activate (bool with_focus) {
	RK_TRACE (APP);

	// see https://mail.kde.org/pipermail/rkward-devel/2010-September/002422.html
	if (with_focus) view->removeSelection ();
	RKMDIWindow::activate (with_focus);
}

///################### END RKConsole ########################
///################### BEGIN RKConsolePart ####################


RKConsolePart::RKConsolePart(RKConsole *console) : KParts::Part(nullptr) {
	RK_TRACE (APP);

	setComponentName (QCoreApplication::applicationName (), QGuiApplication::applicationDisplayName ());

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

