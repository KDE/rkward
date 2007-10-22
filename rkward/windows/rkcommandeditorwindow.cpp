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
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/editorchooser.h>

#include <qlayout.h>
#include <q3popupmenu.h>
#include <qapplication.h>
#include <qtabwidget.h>
#include <qfile.h>
#include <qtimer.h>
#include <qobject.h>
#include <QHBoxLayout>
//Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <Q3ValueList>
#include <QLabel>
#include <QKeyEvent>
#include <QEvent>
#include <QClipboard>

#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <klibloader.h>
#include <kiconloader.h>

#include "../misc/rkcommonfunctions.h"
#include "../core/robjectlist.h"
#include "../rkconsole.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "rkhelpsearchwindow.h"
#include "rkcommandeditorwindowpart.h"

#include "../debug.h"

#define GET_HELP_URL 1

RKCommandEditorWindow::RKCommandEditorWindow (QWidget *parent, bool use_r_highlighting) : RKMDIWindow (parent, RKMDIWindow::CommandEditorWindow) {
	RK_TRACE (COMMANDEDITOR);

	KTextEditor::Editor* editor = KTextEditor::editor("katepart");
	RK_ASSERT (editor);

	m_doc = editor->createDocument (this);
	RK_ASSERT (m_doc);
	m_view = m_doc->createView (this);

	setFocusProxy (m_view);
	setFocusPolicy (Qt::StrongFocus);

	// strip down the khtmlpart's GUI. remove some stuff we definitely don't need.
	RKCommonFunctions::removeContainers (m_view, QString ("bookmarks,tools_spelling,tools_spelling_from_cursor,tools_spelling_selection,switch_to_cmd_line").split (','), true);
	RKCommonFunctions::moveContainer (m_view, "Menu", "tools", "edit", true);

	RKCommandEditorWindowPart* part = new RKCommandEditorWindowPart (m_view, this);
	part->insertChildClient (m_view);
	setPart (part);
	initializeActivationSignals ();

	QHBoxLayout *layout = new QHBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	layout->addWidget(m_view);

	setIcon (SmallIcon ("source"));

	connect (m_doc, SIGNAL (documentUrlChanged (KTextEditor::Document*)), this, SLOT (updateCaption (KTextEditor::Document*)));
	connect (m_doc, SIGNAL (modifiedChanged (KTextEditor::Document*)), this, SLOT (updateCaption (KTextEditor::Document*)));		// of course most of the time this causes a redundant call to updateCaption. Not if a modification is undone, however.
	connect (m_doc, SIGNAL (textChanged (KTextEditor::Document*)), this, SLOT (tryCompletionProxy (KTextEditor::Document*)));
//KDE4	connect (m_view, SIGNAL (gotFocus (Kate::View *)), this, SLOT (setPopupMenu (Kate::View *)));
	completion_timer = new QTimer (this);
	connect (completion_timer, SIGNAL (timeout ()), this, SLOT (tryCompletion()));

	if (use_r_highlighting) {
		setRHighlighting ();
		completion_model = new RKCodeCompletionModel (m_view);
		hinter = new RKFunctionArgHinter (this, m_view);
	} else {
		hinter = 0;
		completion_model = 0;
	}

	updateCaption ();	// initialize
//KDE4	QTimer::singleShot (0, this, SLOT (setPopupMenu ()));
}

RKCommandEditorWindow::~RKCommandEditorWindow () {
	RK_TRACE (COMMANDEDITOR);
	delete hinter;
	delete m_doc;
}

/*
KDE4 TODO: still needed? Alternatives?
void RKCommandEditorWindow::setPopupMenu () {
	RK_TRACE (COMMANDEDITOR);

	if (!m_view->factory ()) return;
	m_view->installPopup (static_cast<Q3PopupMenu *> (m_view->factory ()->container ("ktexteditor_popup", m_view)));
}

void RKCommandEditorWindow::setPopupMenu (Kate::View*) {
	setPopupMenu ();
}*/

QString RKCommandEditorWindow::fullCaption () {
	RK_TRACE (COMMANDEDITOR);

	if (m_doc->url ().isEmpty ()) {
		return (shortCaption ());
	} else {
		QString cap = m_doc->url ().url ();
		if (isModified ()) cap.append (i18n (" [modified]"));
		return (cap);
	}
}

QString RKCommandEditorWindow::getDescription () {
	RK_TRACE (COMMANDEDITOR);

	return ("script:" + m_doc->url ().url ());
}

void RKCommandEditorWindow::closeEvent (QCloseEvent *e) {
	if (isModified ()) {
		int status = KMessageBox::warningYesNo (this, i18n ("The document \"%1\" has been modified. Close it anyway?", caption ()), i18n ("File not saved"));
	
		if (status != KMessageBox::Yes) {
			e->ignore ();
			return;
		}
	}

	QWidget::closeEvent (e);
}

// KDE4 TODO: inline
void RKCommandEditorWindow::setRHighlighting () {
	RK_TRACE (COMMANDEDITOR);

	// set syntax-highlighting for R
	if (!m_doc->setHighlightingMode("R Script")) RK_DO (qDebug ("R syntax highlighting defintion not found!"), COMMANDEDITOR, DL_ERROR);
}

void RKCommandEditorWindow::copy () {
	RK_TRACE (COMMANDEDITOR);

	QApplication::clipboard()->setText (m_view->selectionText ());
}

void RKCommandEditorWindow::setReadOnly (bool ro) {
	RK_TRACE (COMMANDEDITOR);

	m_doc->setReadWrite (!ro);
}

bool RKCommandEditorWindow::openURL (const KUrl &url, bool use_r_highlighting, bool read_only){
	RK_TRACE (COMMANDEDITOR);
	if (m_doc->openUrl (url)){
		if (use_r_highlighting) setRHighlighting ();
		setReadOnly (read_only);

		updateCaption ();

		return true;
	}
	return false;
}

KUrl RKCommandEditorWindow::url () {
	RK_TRACE (COMMANDEDITOR);

	return (m_doc->url ());
}

bool RKCommandEditorWindow::isModified() {
	RK_TRACE (COMMANDEDITOR);
	return m_doc->isModified();
}

void RKCommandEditorWindow::insertText (const QString &text) {
// KDE4: inline?
	RK_TRACE (COMMANDEDITOR);
	m_view->insertText (text);
	setFocus();
}

void RKCommandEditorWindow::setText (const QString &text) {
	RK_TRACE (COMMANDEDITOR);
	m_doc->setText (text);
}

void RKCommandEditorWindow::updateCaption (KTextEditor::Document*) {
	RK_TRACE (COMMANDEDITOR);
	QString name = m_doc->url ().fileName ();
	if (name.isEmpty ()) name = m_doc->url ().prettyUrl ();
	if (name.isEmpty ()) name = i18n ("Unnamed");
	if (isModified ()) name.append (i18n (" [modified]"));

	setCaption (name);
}

void RKCommandEditorWindow::showHelp () {
	RK_TRACE (COMMANDEDITOR);

	KTextEditor::Cursor c = m_view->cursorPosition();
	QString line = m_doc->line(c.line ()) + ' ';

	RKHelpSearchWindow::mainHelpSearch ()->getContextHelp (line, c.column());
}

void RKCommandEditorWindow::tryCompletionProxy (KTextEditor::Document*) {
	completion_timer->start (100, true);
}

void RKCommandEditorWindow::tryCompletion () {
	// TODO: merge this with RKConsole::doTabCompletion () somehow
	RK_TRACE (COMMANDEDITOR);
	if (!completion_model) return;

	KTextEditor::Cursor c = m_view->cursorPosition();
	uint para=c.line(); uint cursor_pos=c.column();

	QString current_line = m_doc->line (para);
	if (current_line.findRev ("#", cursor_pos) >= 0) return;	// do not hint while in comments

	int start;
	int end;
	RKCommonFunctions::getCurrentSymbolOffset (current_line, cursor_pos, false, &start, &end);
	if ((end - start) >= 2) {
		KTextEditor::Range range (para, start, para, end);

		KTextEditor::CodeCompletionInterface *iface = qobject_cast<KTextEditor::CodeCompletionInterface*> (m_view);
		if (!iface) {
			RK_ASSERT (false);
			return;
		}
		if (iface->isCompletionActive ()) {
			completion_model->completionInvoked (m_view, range, KTextEditor::CodeCompletionModel::ManualInvocation);
		} else {
			iface->startCompletion (range, completion_model);
		}
	}
}

bool RKCommandEditorWindow::provideContext (unsigned int line_rev, QString *context, int *cursor_position) {
	RK_TRACE (COMMANDEDITOR);

	KTextEditor::Cursor c = m_view->cursorPosition();
	uint current_line_num=c.line(); uint cursor_pos=c.column();

	if (line_rev > current_line_num) return false;

	if (line_rev == 0) {
		*cursor_position = cursor_pos;
	} else {
		*cursor_position = -1;
	}
	*context = m_doc->line (current_line_num - line_rev);

	return true;
}

void RKCommandEditorWindow::runSelection() {
	RK_TRACE (COMMANDEDITOR);

	QString command = m_view->selectionText ();
	if (command.isEmpty ()) return;

	RKConsole::pipeUserCommand (new RCommand (command, RCommand::User, QString::null));
}

void RKCommandEditorWindow::runLine() {
	RK_TRACE (COMMANDEDITOR);

	KTextEditor::Cursor c = m_view->cursorPosition();
	QString command = m_doc->line (c.line());
	if (!command.isEmpty ()) RKConsole::pipeUserCommand (new RCommand (command, RCommand::User, QString::null));

	// advance to next line (NOTE: m_view->down () won't work on auto-wrapped lines)
	c.setLine(c.line() + 1);
	m_view->setCursorPosition (c);
}


void RKCommandEditorWindow::runAll() {
	RK_TRACE (COMMANDEDITOR);

	QString command = m_doc->text ();
	if (command.isEmpty ()) return;

	RKConsole::pipeUserCommand (command);
}


//////////////////////// RKFunctionArgHinter //////////////////////////////

#include <q3vbox.h>

#include "../core/rfunctionobject.h"

RKFunctionArgHinter::RKFunctionArgHinter (RKScriptContextProvider *provider, KTextEditor::View* view) {
	RK_TRACE (COMMANDEDITOR);

	RKFunctionArgHinter::provider = provider;
	RKFunctionArgHinter::view = view;

	const QObjectList children = view->children ();
	for (QObjectList::const_iterator it = children.constBegin(); it != children.constEnd (); ++it) {
		QObject *obj = *it;
		obj->installEventFilter (this);
	}

	arghints_popup = new QFrame (0, Qt::ToolTip);
	QVBoxLayout *layout = new QVBoxLayout (arghints_popup);
	layout->setContentsMargins (2, 2, 2, 2);
	arghints_popup->setFrameStyle (QFrame::Plain);
	arghints_popup->setFrameShape (QFrame::Box);
	arghints_popup->setLineWidth (1);
	arghints_popup_text = new QLabel (arghints_popup);
	layout->addWidget (arghints_popup_text);
	arghints_popup->hide ();
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

	// fix up seems to be needed
	if (current_context.isEmpty ()) {
		hideArgHint ();
		return;
	}
	if (i >= current_context.size ()) i = current_context.size () -1;
	if (i < 0) i = 0;

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
			if ((!have_context) || (current_line.isEmpty ())) break;

			RK_ASSERT (cursor_pos > 0);
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
	arghints_popup->move (view->mapToGlobal (view->cursorPositionCoordinates () + QPoint (0, arghints_popup->height ())));
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

//////////////////////// RKCodeCompletionModel ////////////////////

RKCodeCompletionModel::RKCodeCompletionModel (KTextEditor::View *parent) : KTextEditor::CodeCompletionModel (parent) {
	RK_TRACE (COMMANDEDITOR);

	setRowCount (0);

	KTextEditor::CodeCompletionInterface *iface = qobject_cast<KTextEditor::CodeCompletionInterface*> (parent);
	if (!iface) {
		RK_ASSERT (false);
		return;
	}
	iface->registerCompletionModel (this);
	iface->setAutomaticInvocationEnabled (false);
}

RKCodeCompletionModel::~RKCodeCompletionModel () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCodeCompletionModel::completionInvoked (KTextEditor::View *view, const KTextEditor::Range &range, InvocationType) {
	RK_TRACE (COMMANDEDITOR);
qDebug ("invoked");

	QString current_symbol = view->document ()->text (range);
	RObject::RObjectMap map;
	RObjectList::getObjectList ()->findObjectsMatching (current_symbol, &map);

	list.clear ();
	// this is silly, but we need an int indexable storage, so we copy the map to a list
	for (RObject::RObjectMap::const_iterator it = map.constBegin (); it != map.constEnd (); ++it) {
		list.append (it.data ());
	}
qDebug ("%d", list.count ());
	setRowCount (list.count ());

	reset ();
}

void RKCodeCompletionModel::executeCompletionItem (KTextEditor::Document *document, const KTextEditor::Range &word, int row) const {
	RK_TRACE (COMMANDEDITOR);

	RK_ASSERT (list.size () > row);
	RObject *object = list[row];
	RK_ASSERT (object);

	document->replaceText (word, object->getFullName ());
}

QVariant RKCodeCompletionModel::data (const QModelIndex& index, int role) const {

	int col = index.column ();
	int row = index.row ();

	if (index.parent ().isValid ()) return QVariant ();
	if (row > list.count ()) return QVariant ();
//	if (column > 1) return QVariant;

	RObject* object = list[row];
	RK_ASSERT (object);

	if ((role == Qt::DisplayRole) || (role==KTextEditor::CodeCompletionModel::CompletionRole)) {
		if (col == KTextEditor::CodeCompletionModel::Name) {
			return (object->getBaseName ());
		}
	}

	return QVariant ();
}

#include "rkcommandeditorwindow.moc"
