/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
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
#ifndef RKCOMMANDEDITORWINDOW_H
#define RKCOMMANDEDITORWINDOW_H

#include <qwidget.h>
#include <qstring.h>

#include <kate/view.h>
#include <kate/document.h>
#include <kurl.h>
#include <kmdichildview.h>

class RKCommandEditor;
class KAction;
class KToggleAction;
class RKwardApp; 
class RCommandChain;

/**
	\brief Provides an editor window for R-commands, as well as a text-editor window in general.

While being called RKCommandEditorWindow, this class handles all sort of text-files, both read/write and read-only. It is an MDI child that is added to the main window, based on KatePart.

@author Pierre Ecochard
*/
class RKCommandEditorWindow : public KMdiChildView {
// we need the Q_OBJECT thing for some inherits ("RKCommandEditorWindow")-calls in rkward.cpp.
	Q_OBJECT
public:
/** constructor
@param use_r_highlighting Initialize the view to use R syntax highlighting. Use, if you're going to edit an R syntax file */
	RKCommandEditorWindow (QWidget *parent = 0, bool use_r_highlighting=true);
/** destructor */
	~RKCommandEditorWindow ();
/** return text of current selection */
	QString getSelection ();
/** return text in current line */
	QString getLine ();
/** return entire text */
	QString getText ();
/** open given URL. 
@param use_r_highlighting Initialize the view to use R syntax highlighting. Use, if you're going to edit an R syntax file
@param read_only Open the file in read-only mode */
	bool openURL (const KURL &url, bool use_r_highlighting=true, bool read_only=false);
/** returns, whether the document was modified since the last save */
	bool isModified ();
/** insert the given text into the document at the current cursor position. Additionally, focuses the view */
	void insertText (const QString &text);
/** Show help about the current word. */
	void showHelp ();
/** set the current text (clear all previous text, and sets new text) */
	void setText (const QString &text);
public slots:
/** update Tab caption according to the current url. Display the filename-component of the URL, or - if not available - a more elaborate description of the url. Also appends a "[modified]" if approriate */
	void updateCaption ();
protected:
/** reimplemented from KMdiChildView: give the editor window a chance to object to being closed (if unsaved) */
	void closeEvent (QCloseEvent *e);
private:
	Kate::Document *m_doc;
	Kate::View *m_view;

/** set syntax highlighting-mode to R syntax */
	void setRHighlighting ();
};

#endif
