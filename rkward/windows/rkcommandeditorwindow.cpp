/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
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
#include "rkcommandeditorwindow.h"

#include <kparts/partmanager.h>
#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <kparts/factory.h>

#include <ktexteditor/configinterface.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/printinterface.h>
#include <ktexteditor/encodinginterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/popupmenuinterface.h>

#include <qlayout.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qtabwidget.h>
#include <qfile.h>
#include <qtimer.h>
#include <qobjectlist.h>

#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kaccel.h>
#include <klibloader.h>
#include <kiconloader.h>

#include "../misc/rkcommonfunctions.h"
#include "../core/robjectlist.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../khelpdlg.h"
#include "rkcommandeditorwindowpart.h"

#include "../debug.h"

#define GET_HELP_URL 1

RKCommandEditorWindow::RKCommandEditorWindow (QWidget *parent, bool use_r_highlighting) : RKMDIWindow (parent, RKMDIWindow::CommandEditorWindow) {
	RK_TRACE (COMMANDEDITOR);

	KLibFactory *factory = KLibLoader::self()->factory( "libkatepart" );
	if (factory) {
		// Create the part
		m_doc = (Kate::Document *) factory->create( this, "katepart", "KParts::ReadWritePart" );
		RK_ASSERT (m_doc);
		m_view = (Kate::View *) m_doc->widget();
	}

	// strip down the khtmlpart's GUI. remove some stuff we definitely don't need.
	RKCommonFunctions::removeContainers (m_doc, QStringList::split (',', "bookmarks,tools_spelling,tools_spelling_from_cursor,tools_spelling_selection,switch_to_cmd_line"), true);
	RKCommonFunctions::moveContainer (m_doc, "Menu", "tools", "edit", true);

	m_doc->insertChildClient (new RKCommandEditorWindowPart (m_view, this));
	initializeActivationSignals ();
	setFocusProxy (m_view);

	QHBoxLayout *pLayout = new QHBoxLayout( this, 0, -1, "layout");
	pLayout->addWidget(m_view);

	setIcon (SmallIcon ("source"));

	connect (m_doc, SIGNAL (fileNameChanged ()), this, SLOT (updateCaption ()));
	connect (m_doc, SIGNAL (modifiedChanged ()), this, SLOT (updateCaption ()));		// of course most of the time this causes a redundant call to updateCaption. Not if a modification is undone, however.
	connect (m_doc, SIGNAL (textChanged ()), this, SLOT (tryCompletionProxy ()));
	connect (m_view, SIGNAL (filterInsertString (KTextEditor::CompletionEntry *, QString *)), this, SLOT (fixCompletion (KTextEditor::CompletionEntry *, QString *)));
	connect (m_view, SIGNAL (gotFocus (Kate::View *)), this, SLOT (setPopupMenu (Kate::View *)));
	completion_timer = new QTimer (this);
	connect (completion_timer, SIGNAL (timeout ()), this, SLOT (tryCompletion()));

	if (use_r_highlighting) {
		setRHighlighting ();
		hinter = new RKFunctionArgHinter (this, m_view);
	} else {
		hinter = 0;
	}

	updateCaption ();	// initialize
	QTimer::singleShot (0, this, SLOT (setPopupMenu ()));
}

RKCommandEditorWindow::~RKCommandEditorWindow () {
	RK_TRACE (COMMANDEDITOR);
	delete hinter;
	delete m_doc;
}

void RKCommandEditorWindow::setPopupMenu () {
	RK_TRACE (COMMANDEDITOR);

	if (!m_view->factory ()) return;
	m_view->installPopup (static_cast<QPopupMenu *> (m_view->factory ()->container ("ktexteditor_popup", m_view)));
}

void RKCommandEditorWindow::setPopupMenu (Kate::View*) {
	setPopupMenu ();
}

QString RKCommandEditorWindow::getDescription () {
	RK_TRACE (COMMANDEDITOR);

	return ("script:" + m_doc->url ().url ());
}

void RKCommandEditorWindow::closeEvent (QCloseEvent *e) {
	if (isModified ()) {
		int status = KMessageBox::warningYesNo (this, i18n ("The document \"%1\" has been modified. Close it anyway?").arg (caption ()), i18n ("File not saved"));
	
		if (status != KMessageBox::Yes) {
			e->ignore ();
			return;
		}
	}

	QWidget::closeEvent (e);
}

void RKCommandEditorWindow::setRHighlighting () {
	// set syntax-highlighting for R
	int modes_count = highlightingInterface (m_doc)->hlModeCount ();
	bool found_mode = false;
	int i;
	for (i = 0; i < modes_count; ++i) {
		if (highlightingInterface (m_doc)->hlModeName (i).lower() == "r script") {
			found_mode = true;
			break;
		}
	}
	if (found_mode) {
		highlightingInterface (m_doc)->setHlMode (i);
	} else {
		RK_DO (qDebug ("No syntax highlighting definition found for r script."), COMMANDEDITOR, DL_WARNING);
	}
}

QString RKCommandEditorWindow::getSelection () {
	RK_TRACE (COMMANDEDITOR);
	return m_doc->selection ();
}

QString RKCommandEditorWindow::getLine () {
	RK_TRACE (COMMANDEDITOR);
	return m_view->currentTextLine ();
}

QString RKCommandEditorWindow::getText () {
	RK_TRACE (COMMANDEDITOR);
	return m_doc->text ();
}

void RKCommandEditorWindow::copy () {
	RK_TRACE (COMMANDEDITOR);

	m_view->copy ();
}

bool RKCommandEditorWindow::openURL (const KURL &url, bool use_r_highlighting, bool read_only){
	RK_TRACE (COMMANDEDITOR);
	if (m_doc->openURL (url)){
		if (use_r_highlighting) setRHighlighting ();
		m_doc->setReadWrite (!read_only);

		updateCaption ();

		return true;
	}
	return false;
}

KURL RKCommandEditorWindow::url () {
	RK_TRACE (COMMANDEDITOR);

	return (m_doc->url ());
}


bool RKCommandEditorWindow::isModified() {
	RK_TRACE (COMMANDEDITOR);
	return m_doc->isModified();
}

void RKCommandEditorWindow::insertText (const QString &text) {
	RK_TRACE (COMMANDEDITOR);
	m_doc->insertText (m_view->cursorLine  (), m_view->cursorColumn (), text);
	setFocus();
}

void RKCommandEditorWindow::setText (const QString &text) {
	RK_TRACE (COMMANDEDITOR);
	m_doc->setText (text);
}

void RKCommandEditorWindow::updateCaption () {
	RK_TRACE (COMMANDEDITOR);
	QString name = m_doc->url ().fileName ();
	if (name.isEmpty ()) name = m_doc->url ().prettyURL ();
	if (name.isEmpty ()) name = i18n ("Unnamed");
	if (isModified ()) name.append (i18n (" [modified]"));

	setCaption (name);
}

void RKCommandEditorWindow::showHelp () {
	RK_TRACE (COMMANDEDITOR);
	uint para=0; uint p=0;
	m_view->cursorPosition (&para, &p);

	QString line = m_view->currentTextLine() + ' ';

	RKHelpSearchWindow::mainHelpSearch ()->getContextHelp (line, p);
}

void RKCommandEditorWindow::tryCompletionProxy () {
	completion_timer->start (100, true);
}

void RKCommandEditorWindow::tryCompletion () {
	// TODO: merge this with RKConsole::doTabCompletion () somehow
	RK_TRACE (COMMANDEDITOR);

	uint para=0; uint cursor_pos=0;
	m_view->cursorPosition (&para, &cursor_pos);
	QString current_line = getLine ();
	if (current_line.findRev ("#", cursor_pos) >= 0) return;	// do not hint while in comments

	QString current_symbol = RKCommonFunctions::getCurrentSymbol (current_line, cursor_pos, false);
	if (current_symbol.length () >= 2) {
		RObject::RObjectMap map;
		RObject::RObjectMap::const_iterator it;
		RObjectList::getObjectList ()->findObjectsMatching (current_symbol, &map);

		if (!map.isEmpty ()) {
			QValueList<KTextEditor::CompletionEntry> list;
	
			for (it = map.constBegin (); it != map.constEnd (); ++it) {
				KTextEditor::CompletionEntry entry;
				entry.text = it.key ();
				list.append (entry);
			}

			m_view->showCompletionBox (list);
		}
	}
}

void RKCommandEditorWindow::fixCompletion (KTextEditor::CompletionEntry *entry, QString *string) {
	RK_TRACE (COMMANDEDITOR);

	*string = entry->text;	// why, oh, why, isn't this always the case?

	uint current_line_num=0; uint cursor_pos=0;
	m_view->cursorPosition (&current_line_num, &cursor_pos);
	QString current_line = getLine ();

	int word_start;
	int word_end;
	RKCommonFunctions::getCurrentSymbolOffset (current_line, cursor_pos, false, &word_start, &word_end);

	// remove the start of the word, as the whole string will be inserted by katepart
	m_doc->removeText (current_line_num, word_start, current_line_num, word_end);
}

bool RKCommandEditorWindow::provideContext (unsigned int line_rev, QString *context, int *cursor_position) {
	RK_TRACE (COMMANDEDITOR);

	uint current_line_num=0; uint cursor_pos=0;
	m_view->cursorPosition (&current_line_num, &cursor_pos);

	if (line_rev > current_line_num) return false;

	if (line_rev == 0) {
		*cursor_position = cursor_pos;
	} else {
		*cursor_position = -1;
	}
	*context = m_doc->textLine (current_line_num - line_rev);

	return true;
}

//////////////////////// RKFunctionArgHinter //////////////////////////////

#include <qvbox.h>

#include "../core/rfunctionobject.h"

RKFunctionArgHinter::RKFunctionArgHinter (RKScriptContextProvider *provider, Kate::View* view) {
	RK_TRACE (COMMANDEDITOR);

	RKFunctionArgHinter::provider = provider;
	RKFunctionArgHinter::view = view;

	const QObjectList *children = view->children ();
	QObjectListIt it (*children);
	QObject *obj;
	while ((obj = it.current()) != 0) {
		++it;
		obj->installEventFilter (this);
	}

	arghints_popup = new QVBox (0, 0, WType_Popup);
	arghints_popup->setFrameStyle (QFrame::Box | QFrame::Plain);
	arghints_popup->setLineWidth (1);
	arghints_popup_text = new QLabel (arghints_popup);
	arghints_popup->hide ();
	arghints_popup->setFocusProxy (view);
}

RKFunctionArgHinter::~RKFunctionArgHinter () {
	RK_TRACE (COMMANDEDITOR);
}

void RKFunctionArgHinter::tryArgHint () {
	RK_TRACE (COMMANDEDITOR);

	// do this in the next event cycle to make sure any inserted characters have truly been inserted
	QTimer::singleShot (0, this, SLOT (tryArgHintNow ()));
}

void RKFunctionArgHinter::tryArgHintNow () {
	RK_TRACE (COMMANDEDITOR);

	int line_rev;
	int cursor_pos;
	QString current_context;
	QString current_line;

	// fetch the most immediate context line. More will be fetched later, if appropriate
	bool have_context = provider->provideContext (line_rev = 0, &current_line, &cursor_pos);
	RK_ASSERT (have_context);
	RK_ASSERT (cursor_pos >= 0);
	current_context = current_line;

	// find the corrresponding opening brace
	int matching_left_brace_pos;
	int brace_level = 1;
	int i = cursor_pos;

	while (true) {
		if (current_context.at (i) == QChar (')')) {
			brace_level++;
		} else if (current_context.at (i) == QChar ('(')) {
			brace_level--;
			if (!brace_level) break;
		}

		--i;
		if (i < 0) {
			bool have_context = provider->provideContext (++line_rev, &current_line, &cursor_pos);
			if (!have_context) break;

			RK_ASSERT (cursor_pos < 0);
			current_context.prepend (current_line);
			i = current_line.length () - 1;
		}
	}

	if (!brace_level) matching_left_brace_pos = i;
	else {
		hideArgHint ();
		return;
	}

	// now find where the symbol to the left ends
	// there cannot be a line-break between the opening brace, and the symbol name (or can there?), so no need to fetch further context
	int potential_symbol_end = matching_left_brace_pos - 1;
	while ((potential_symbol_end > 0) && current_context.at (potential_symbol_end).isSpace ()) {
		--potential_symbol_end;
	}
	if (current_context.at (potential_symbol_end).isSpace ()) {
		hideArgHint ();
		return;
	}

	// now identify the symbol and object (if any)
	QString effective_symbol = RKCommonFunctions::getCurrentSymbol (current_context, potential_symbol_end+1);
	if (effective_symbol.isEmpty ()) {
		hideArgHint ();
		return;
	}

	RObject *object = RObjectList::getObjectList ()->findObject (effective_symbol);
	if ((!object) || (!object->isType (RObject::Function))) {
		hideArgHint ();
		return;
	}

	// initialize and show popup
	arghints_popup_text->setText (effective_symbol + " (" + static_cast<RFunctionObject*> (object)->printArgs () + ')');
	arghints_popup->resize (arghints_popup_text->sizeHint () + QSize (2, 2));
	arghints_popup->move (view->mapToGlobal (view->cursorCoordinates () + QPoint (0, arghints_popup->height ())));
	arghints_popup->show ();
}

void RKFunctionArgHinter::hideArgHint () {
	RK_TRACE (COMMANDEDITOR);
	arghints_popup->hide ();
}

bool RKFunctionArgHinter::eventFilter (QObject *, QEvent *e) {
	if (e->type () == QEvent::KeyPress || e->type () == QEvent::AccelOverride) {
		RK_TRACE (COMMANDEDITOR);	// avoid loads of empty traces, putting this here
		QKeyEvent *k = static_cast<QKeyEvent *> (e);

		if (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return || k->key () == Qt::Key_Up || k->key () == Qt::Key_Down || k->key () == Qt::Key_Left || k->key () == Qt::Key_Right || k->key () == Qt::Key_Home || k->key () == Qt::Key_Tab) {
			hideArgHint ();
		} else if (k->key () == Qt::Key_Backspace || k->key () == Qt::Key_Delete){
			tryArgHint ();
		} else {
			QString text = k->text ();
			if (text == "(") {
				tryArgHint ();
			} else if (text == ")") {
				tryArgHint ();
			}
		}
	}

	return false;
}

#include "rkcommandeditorwindow.moc"
