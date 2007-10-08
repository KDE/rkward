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
#include <qstring.h>
//Added by qt3to4:
#include <QEvent>
#include <QLabel>
#include <QCloseEvent>

#include <kate/view.h>
#include <kate/document.h>
#include <kurl.h>

#include "../windows/rkmdiwindow.h"

class Q3VBox;
class QLabel;

/** classes wishing to use RKFunctionArgHinter should derive from this, and implement provideContext () */
class RKScriptContextProvider {
public:
	RKScriptContextProvider () {};
	~RKScriptContextProvider () {};

	/** to be implemented in subclasses. Provide some context (probably a line, but you may provide chunks in arbitrary size). If line_rev is 0, provide the line, the cursor is in. If line_rev is greater than 0, provide context before that.
	@param context Place the context here
	@param cursor_position if line_rev is 0, set this to the current column of the cursor, else set to -1
	@returns whether context was available or not */
	virtual bool provideContext (unsigned int line_rev, QString *context, int *cursor_position) = 0;
};

/** function argument hinting for RKCommandEditorWindow and RKConsole */
class RKFunctionArgHinter : public QObject {
	Q_OBJECT
public:
	RKFunctionArgHinter (RKScriptContextProvider *provider, Kate::View* view);
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
	Kate::View *view;

	Q3VBox *arghints_popup;
	QLabel *arghints_popup_text;
};

class QTimer;

/**
	\brief Provides an editor window for R-commands, as well as a text-editor window in general.

While being called RKCommandEditorWindow, this class handles all sorts of text-files, both read/write and read-only. It is an MDI window that is added to the main window, based on KatePart.

@author Pierre Ecochard
*/
class RKCommandEditorWindow : public RKMDIWindow, public RKScriptContextProvider {
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
public slots:
/** update Tab caption according to the current url. Display the filename-component of the URL, or - if not available - a more elaborate description of the url. Also appends a "[modified]" if appropriate */
	void updateCaption ();
/** called whenever it might be appropriate to show a code completion box. The box is not shown immediately, but only after a timeout (if at all) */
	void tryCompletionProxy ();
/** show a code completion box if appropriate. Use tryCompletionProxy () instead, which will call this function after a timeout */
	void tryCompletion ();
/** called by the Kate part, if an entry was selected from the code completion box. Will remove the current symbol, as the kate part is about to re-add it (in completed form) */
	void fixCompletion (KTextEditor::CompletionEntry *, QString *);
	void setPopupMenu (Kate::View *);
	void setPopupMenu ();
/** Show help about the current word. */
	void showHelp ();
/** run the currently selected command(s) */
	void runSelection ();
/** run the current line */
	void runLine ();
/** run the entire script */
	void runAll ();
protected:
/** reimplemented from KMdiChildView: give the editor window a chance to object to being closed (if unsaved) */
	void closeEvent (QCloseEvent *e);
private:
	Kate::Document *m_doc;
	Kate::View *m_view;
	RKFunctionArgHinter *hinter;

	QTimer *completion_timer;
/** set syntax highlighting-mode to R syntax */
	void setRHighlighting ();
};

#endif
