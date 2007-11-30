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
#ifndef RKCOMMANDEDITORWINDOW_H
#define RKCOMMANDEDITORWINDOW_H

#include <QWidget>
#include <QVector>
#include <qstring.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/rangefeedback.h>
#include <kurl.h>

#include "../windows/rkmdiwindow.h"

class QEvent;
class QCloseEvent;
class QFrame;
class QLabel;
class QAction;
class RKCommandEditorWindow;
class KActionCollection;

/** This class provides a KPart interface to RKCommandEditorWindow. The reason to use this, is so the required menus/menu-items can be merged in on the fly.

@author Thomas Friedrichsmeier
*/
class RKCommandEditorWindowPart : public KParts::Part {
protected:
friend class RKCommandEditorWindow;
	RKCommandEditorWindowPart (QWidget *parent);
	~RKCommandEditorWindowPart ();
};

/** classes wishing to use RKFunctionArgHinter should derive from this, and implement provideContext () */
class RKScriptContextProvider {
public:
	RKScriptContextProvider () {};
	virtual ~RKScriptContextProvider () {};

	/** to be implemented in subclasses. Provide some context (probably a line, but you may provide chunks in arbitrary size). If line_rev is 0, provide the line, the cursor is in. If line_rev is greater than 0, provide context before that.
	@param context Place the context here
	@param cursor_position if line_rev is 0, set this to the current column of the cursor, else set to -1
	@returns whether context was available or not */
	virtual bool provideContext (unsigned int line_rev, QString *context, int *cursor_position) = 0;
};

class RObject;
/** function argument hinting for RKCommandEditorWindow and RKConsole */
class RKFunctionArgHinter : public QObject {
	Q_OBJECT
public:
	RKFunctionArgHinter (RKScriptContextProvider *provider, KTextEditor::View* view);
	~RKFunctionArgHinter ();

	/** Try to show an arg hint now */
	void tryArgHint ();
	/** Hide the arg hint (if shown) */
	void hideArgHint ();
public slots:
	/** Internal worker function for tryArgHint () */
	void tryArgHintNow ();
protected:
	/** The (keypress) events of the view are filtered to determine, when to show / hide an argument hint */
	bool eventFilter (QObject *, QEvent *e);
private:
	RKScriptContextProvider *provider;
	KTextEditor::View *view;

	QFrame *arghints_popup;
	QLabel *arghints_popup_text;
};

/** code completion model for RKCommandEditorWindow */
class RKCodeCompletionModel : public KTextEditor::CodeCompletionModel {
public:
	RKCodeCompletionModel (RKCommandEditorWindow* parent);
	~RKCodeCompletionModel ();

	void updateCompletionList (const QString& symbol);
	void completionInvoked (KTextEditor::View *, const KTextEditor::Range &, InvocationType);
	void executeCompletionItem (KTextEditor::Document *document, const KTextEditor::Range &word, int row) const;
	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const;

	bool isEmpty () const { return list.isEmpty (); };
private:
	QVector<RObject*> list;
	QVector<QString> list_names;
	QString current_symbol;
	RKCommandEditorWindow *command_editor;
};

class QTimer;

/**
	\brief Provides an editor window for R-commands, as well as a text-editor window in general.

While being called RKCommandEditorWindow, this class handles all sorts of text-files, both read/write and read-only. It is an MDI window that is added to the main window, based on KatePart.

@author Pierre Ecochard
*/
class RKCommandEditorWindow : public RKMDIWindow, public RKScriptContextProvider, public KTextEditor::SmartRangeWatcher {
// we need the Q_OBJECT thing for some inherits ("RKCommandEditorWindow")-calls in rkward.cpp.
	Q_OBJECT
public:
/** constructor
@param use_r_highlighting Initialize the view to use R syntax highlighting. Use, if you're going to edit an R syntax file */
	RKCommandEditorWindow (QWidget *parent = 0, bool use_r_highlighting=true);
/** destructor */
	~RKCommandEditorWindow ();
/** open given URL. 
@param use_r_highlighting Initialize the view to use R syntax highlighting. Use, if you're going to edit an R syntax file
@param read_only Open the file in read-only mode */
	bool openURL (const KUrl &url, bool use_r_highlighting=true, bool read_only=false);
/** returns, whether the document was modified since the last save */
	bool isModified ();
/** insert the given text into the document at the current cursor position. Additionally, focuses the view */
	void insertText (const QString &text);
/** set the current text (clear all previous text, and sets new text) */
	void setText (const QString &text);
/** copy current selection. Wrapper for use by external classes */
	void copy ();

	QString getDescription ();

/** reimplemented from RKMDIWindow to return full path of file (if any) */
	QString fullCaption ();

	void setReadOnly (bool ro);

/** Return current url */
	KUrl url ();

	bool provideContext (unsigned int line_rev, QString *context, int *cursor_position);
	QString currentCompletionWord () const;
public slots:
/** update Tab caption according to the current url. Display the filename-component of the URL, or - if not available - a more elaborate description of the url. Also appends a "[modified]" if appropriate */
	void updateCaption (KTextEditor::Document* = 0);
/** called whenever it might be appropriate to show a code completion box. The box is not shown immediately, but only after a timeout (if at all) */
	void tryCompletionProxy (KTextEditor::Document*);
/** show a code completion box if appropriate. Use tryCompletionProxy () instead, which will call this function after a timeout */
	void tryCompletion ();
	void setPopupMenu ();
	void focusIn (KTextEditor::View *);
/** Show help about the current word. */
	void showHelp ();
/** run the currently selected command(s) */
	void runSelection ();
/** run the current line */
	void runLine ();
/** run the entire script */
	void runAll ();
/** run the current block */
	void runBlock ();
/** invoke the settings page for the command editor */
	void configure ();

/** mark current selection as a block */
	void markBlock ();
/** unmark current block */
	void unmarkBlock ();

/** selection has changed. Enable / disable actions accordingly */
	void selectionChanged (KTextEditor::View* view);
/** cursor position has changed. Enable / disable actions accordingly */
	void cursorPositionChanged (KTextEditor::View* view, const KTextEditor::Cursor &new_position);
protected:
/** reimplemented from RKMDIWindow: give the editor window a chance to object to being closed (if unsaved) */
	void closeEvent (QCloseEvent *e);
private:
	KTextEditor::SmartRange* currentBlock() const;
	KTextEditor::SmartRange* last_active_block;

	void highlightBlock (KTextEditor::SmartRange* block, bool active);

	KTextEditor::Document *m_doc;
	KTextEditor::View *m_view;
	KTextEditor::CodeCompletionInterface *cc_iface;
	KTextEditor::SmartInterface *smart_iface;
	KTextEditor::SmartRange *top_block_range;
	RKFunctionArgHinter *hinter;
	RKCodeCompletionModel *completion_model;

	QTimer *completion_timer;
/** set syntax highlighting-mode to R syntax */
	void setRHighlighting ();

	void initializeActions (KActionCollection* ac);

	QAction* action_mark_block;
	QAction* action_unmark_block;

	QAction* action_run_all;
	QAction* action_run_selection;
	QAction* action_run_block;
	QAction* action_run_line;

	QAction* action_help_function;
};

#endif
