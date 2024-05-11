/*
rkcommandlog - This file is part of RKWard (https://rkward.kde.org). Created: Sun Nov 3 2002
SPDX-FileCopyrightText: 2002-2023 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcommandlog.h"

#include "../rbackend/rkrinterface.h"
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
#include <QFontDatabase>

#include <KLocalizedString>
#include <kactioncollection.h>

#include "../debug.h"

//static
RKCommandLog *RKCommandLog::rkcommand_log = nullptr;

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

	connect (RKSettings::tracker (), &RKSettingsTracker::settingsChanged, this, &RKCommandLog::settingsChanged);
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

void RKCommandLog::addOtherMessage(const QString &message, const QIcon &icon, ROutput::ROutputType severity) {
	RK_TRACE(APP);

	if (!icon.isNull()) {
		auto doc = log_view->document();
		QUrl uri = QUrl(QString("internalicon://%1.png").arg(icon.name()));
		doc->addResource(QTextDocument::ImageResource, uri, QVariant(icon.pixmap(16,16)));
		QTextImageFormat f;
		f.setWidth(16);
		f.setHeight(16);
		f.setName(uri.toString());;
		log_view->textCursor().insertImage(f);
	}

	if (severity != ROutput::Output) {
		QTextBlockFormat f;
		f.setBackground(QBrush(QColor(255, 255, 255)));
		log_view->textCursor().mergeBlockFormat(f);
	}
	log_view->insertPlainText(message + '\n');
	log_view->setFontWeight(QFont::Normal);
	log_view->setTextColor(Qt::black);
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
	QFont font = QFontDatabase::systemFont (QFontDatabase::FixedFont);
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

void RKCommandLogView::contextMenuEvent(QContextMenuEvent *event) {
	RK_TRACE(APP);

	Q_EMIT popupMenuRequest(event->globalPos());
	event->accept();
}

void RKCommandLogView::selectAll () {
	RK_TRACE (APP);

	moveCursor (QTextCursor::Start, QTextCursor::MoveAnchor);
	moveCursor (QTextCursor::Start, QTextCursor::KeepAnchor);
}

//////////////////////// END RKCommandLogView /////////////////////////
/////////////////////// BEGIN RKCommandLogPart ////////////////////////

#include <kxmlguifactory.h>
#include <QAction>
#include <QGuiApplication>

RKCommandLogPart::RKCommandLogPart(RKCommandLog *for_log) : KParts::Part(nullptr) {
	RK_TRACE (APP);

	setComponentName (QCoreApplication::applicationName (), QGuiApplication::applicationDisplayName ());
	setWidget (log = for_log);
	setXMLFile ("rkcommandlogpart.rc");
}

RKCommandLogPart::~RKCommandLogPart () {
	RK_TRACE (APP);
}

void RKCommandLogPart::initActions () {
	RK_TRACE (APP);

	copy = actionCollection()->addAction(KStandardAction::Copy, "log_copy", log->getView(), &RKCommandLogView::copy);
	actionCollection()->addAction(KStandardAction::Clear, "log_clear", log, &RKCommandLog::clearLog);
	actionCollection()->addAction(KStandardAction::SelectAll, "log_select_all", log->getView(), &RKCommandLogView::selectAll);
	QAction *configure = actionCollection()->addAction("log_configure", log, &RKCommandLog::configureLog);
	configure->setText (i18n ("Configure"));

	run_selection = RKStandardActions::runCurrent (log, log, SLOT(runSelection()));

	connect (log->getView (), &RKCommandLogView::popupMenuRequest, this, &RKCommandLogPart::doPopupMenu);
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

