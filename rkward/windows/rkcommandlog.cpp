/***************************************************************************
                          rkcommandlog  -  description
                             -------------------
    begin                : Sun Nov 3 2002
    copyright            : (C) 2002, 2004, 2005 2006, 2007, 2009, 2010 by Thomas Friedrichsmeier
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

#include "rkcommandlog.h"

#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "../rkconsole.h"
#include "../settings/rksettingsmodulewatch.h"
#include "../misc/rkstandardactions.h"
#include "rkcommandeditorwindow.h"

#include <qpushbutton.h>
#include <qfont.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <QMenu>
#include <QContextMenuEvent>

#include <klocale.h>
#include <kactioncollection.h>
#include <kglobalsettings.h>

#include "../debug.h"

//static
RKCommandLog *RKCommandLog::rkcommand_log = 0;

RKCommandLog::RKCommandLog (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, CommandLogWindow, tool_window, name) {
	RK_TRACE (APP);

	log_view = new RKCommandLogView (this);
	log_view->setLineWrapMode (QTextEdit::NoWrap);
	log_view->setUndoRedoEnabled (false);
	log_view->setReadOnly (true);

	QHBoxLayout *layout = new QHBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	layout->addWidget (log_view);

	setCaption (i18n ("Command log"));
	
	clearLog ();

	last_raised_command = 0;

	RKCommandLogPart *part = new RKCommandLogPart (this);
	setPart (part);
	part->initActions ();
	initializeActivationSignals ();
	setFocusPolicy (Qt::ClickFocus);

	connect (RKSettings::tracker (), SIGNAL (settingsChanged(RKSettings::SettingsPage)), this, SLOT (settingsChanged(RKSettings::SettingsPage)));
	settingsChanged (RKSettings::PageWatch);
}

RKCommandLog::~RKCommandLog(){
	RK_TRACE (APP);

	RK_ASSERT (command_input_shown.isEmpty ());
}

void RKCommandLog::addInput (RCommand *command) {
	RK_TRACE (APP);
	if (!RKSettingsModuleWatch::shouldShowInput (command)) return;

// commands submitted via the console are often incomplete. We delay showing the input, until the command is finished.
	if (command->type () & RCommand::Console) return;

	addInputNoCheck (command);
}

void RKCommandLog::addInputNoCheck (RCommand *command) {
	RK_TRACE (APP);
	if (command_input_shown.contains (command)) return;		// already shown

// TODO: make colors/styles configurable
	if (command->type () & RCommand::User) {
		log_view->setTextColor (Qt::red);
	} else if (command->type () & RCommand::Sync) {
		log_view->setTextColor (Qt::gray);
	} else if (command->type () & RCommand::Plugin) {
		log_view->setTextColor (Qt::blue);
	}

	log_view->setFontItalic (true);

	log_view->insertPlainText (command->command () + '\n');

	checkRaiseWindow (command);
	linesAdded ();

	log_view->setFontItalic (false);

	command_input_shown.append (command);
}

void RKCommandLog::addOutputNoCheck (RCommand *command, ROutput *output) {
	RK_TRACE (APP);

	if (command->type () & RCommand::User) {
		log_view->setTextColor (Qt::red);
	} else if (command->type () & RCommand::Sync) {
		log_view->setTextColor (Qt::gray);
	} else if (command->type () & RCommand::Plugin) {
		log_view->setTextColor (Qt::blue);
	}
	log_view->setFontWeight (QFont::Bold);
	if (output->type != ROutput::Output) {
		QTextBlockFormat f;
		f.setBackground (QBrush (QColor (255, 200, 200)));
		log_view->textCursor ().mergeBlockFormat (f);
	}

	log_view->insertPlainText (output->output);

	if (output->type != ROutput::Output) {
		QTextBlockFormat f;
		f.setBackground (QBrush (QColor (255, 255, 255)));
		log_view->textCursor ().mergeBlockFormat (f);
	}
	log_view->setFontWeight (QFont::Normal);
	log_view->setTextColor (Qt::black);

	checkRaiseWindow (command);
	linesAdded ();
}

void RKCommandLog::checkRaiseWindow (RCommand *command) {
	// called during output. do not trace
	if (command->id () == last_raised_command) return;
	if (!RKSettingsModuleWatch::shouldRaiseWindow (command)) return;
	if (command->type () & RCommand::Console) return;

	last_raised_command = command->id ();
	activate (false);
}

void RKCommandLog::newOutput (RCommand *command, ROutput *output_fragment) {
	RK_TRACE (APP);

	if (!RKSettingsModuleWatch::shouldShowOutput (command)) return;

	if (RKSettingsModuleWatch::shouldShowInput (command)) addInputNoCheck (command);

	addOutputNoCheck (command, output_fragment);
}

void RKCommandLog::rCommandDone (RCommand *command) {
	RK_TRACE (APP);

	if (command->type () & RCommand::Console) {
		if (command->errorIncomplete ()) return;
		addInputNoCheck (command);
	}

// the case we have to deal with here, is that the command/output has not been shown, yet, but should, due to errors
	if (command->hasWarnings() || command->failed()) {
		if (RKSettingsModuleWatch::shouldShowError (command)) {
			if (!RKSettingsModuleWatch::shouldShowInput (command)) addInputNoCheck (command);
			if (!RKSettingsModuleWatch::shouldShowOutput (command)) {
				ROutputList out_list = command->getOutput ();
				for (ROutputList::const_iterator it = out_list.constBegin (); it != out_list.constEnd (); ++it) {
					addOutputNoCheck (command, *it);
				}
			}
			if (command->failed () && command->error ().isEmpty ()) {
				ROutput dummy_output;
				dummy_output.type = ROutput::Error;
				if (command->errorIncomplete ()) {
					dummy_output.output = i18n ("Incomplete statement.\n");
				} else if (command->errorSyntax ()) {
					dummy_output.output = i18n ("Syntax error.\n");
				} else {
					dummy_output.output = i18n ("An unspecified error occurred while running the command.\n");
				}
				addOutputNoCheck (command, &dummy_output);
			}
		}
	}

	if (RKSettingsModuleWatch::shouldShowOutput (command)) log_view->insertPlainText ("\n");
	command_input_shown.removeAll (command);
}

void RKCommandLog::settingsChanged (RKSettings::SettingsPage page) {
	if (page != RKSettings::PageWatch) return;

	RK_TRACE (APP);

	log_view->document ()->setMaximumBlockCount (RKSettingsModuleWatch::maxLogLines ());
}

void RKCommandLog::linesAdded () {
	RK_TRACE (APP);

// scroll to bottom
	log_view->moveCursor (QTextCursor::End, QTextCursor::MoveAnchor);
	log_view->ensureCursorVisible ();
}

void RKCommandLog::configureLog () {
	RK_TRACE (APP);
	RKSettings::configureSettings (RKSettings::PageWatch, this);
}

void RKCommandLog::clearLog () {
	RK_TRACE (APP);

	log_view->setPlainText (QString ());

	// set a fixed width font
	QFont font = KGlobalSettings::fixedFont ();
	log_view->setCurrentFont (font);
}

void RKCommandLog::runSelection () {
	RK_TRACE (APP);

	RKConsole::pipeUserCommand (getView ()->textCursor ().selectedText ());
}

////////////////////////// END RKCommandLog ///////////////////////////
/////////////////////// BEGIN RKCommandLogView ////////////////////////


RKCommandLogView::RKCommandLogView (RKCommandLog *parent) : QTextEdit (parent) {
	RK_TRACE (APP);
}

RKCommandLogView::~RKCommandLogView () {
	RK_TRACE (APP);
}

void RKCommandLogView::contextMenuEvent (QContextMenuEvent *event) {
	RK_TRACE (APP);

	emit (popupMenuRequest (event->globalPos ()));
	event->accept ();
}

void RKCommandLogView::selectAll () {
	RK_TRACE (APP);

	moveCursor (QTextCursor::Start, QTextCursor::MoveAnchor);
	moveCursor (QTextCursor::Start, QTextCursor::KeepAnchor);
}

//////////////////////// END RKCommandLogView /////////////////////////
/////////////////////// BEGIN RKCommandLogPart ////////////////////////

#include <kxmlguifactory.h>
#include <kaction.h>

RKCommandLogPart::RKCommandLogPart (RKCommandLog *for_log) : KParts::Part (0) {
	RK_TRACE (APP);

	setComponentData (KGlobal::mainComponent ());
	setWidget (log = for_log);
	setXMLFile ("rkcommandlogpart.rc");
}

RKCommandLogPart::~RKCommandLogPart () {
	RK_TRACE (APP);
}

void RKCommandLogPart::initActions () {
	RK_TRACE (APP);

	copy = actionCollection ()->addAction (KStandardAction::Copy, "log_copy", log->getView (), SLOT (copy()));
	actionCollection ()->addAction (KStandardAction::Clear, "log_clear", log, SLOT (clearLog()));
	actionCollection ()->addAction (KStandardAction::SelectAll, "log_select_all", log->getView (), SLOT (selectAll()));
	QAction *configure = actionCollection ()->addAction ("log_configure", log, SLOT(configureLog()));
	configure->setText (i18n ("Configure"));

	run_selection = RKStandardActions::runSelection (log, log, SLOT(runSelection()));

	connect (log->getView (), SIGNAL (popupMenuRequest (const QPoint &)), this, SLOT (doPopupMenu (const QPoint &)));
}

void RKCommandLogPart::doPopupMenu (const QPoint &pos) {
	QMenu *menu = static_cast<QMenu *> (factory ()->container ("rkcommandlog_context_menu", this));
	copy->setEnabled (log->getView ()->textCursor ().hasSelection ());
	run_selection->setEnabled (log->getView ()->textCursor ().hasSelection ());

	if (!menu) {
		RK_ASSERT (false);
		return;
	}
	menu->exec (pos);

	copy->setEnabled (true);
	run_selection->setEnabled (true);
}

#include "rkcommandlog.moc"
