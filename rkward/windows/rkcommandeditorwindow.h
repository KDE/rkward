/*
rkcommandeditorwindow - This file is part of the RKWard project. Created: Mon Aug 30 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKCOMMANDEDITORWINDOW_H
#define RKCOMMANDEDITORWINDOW_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QString>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/movingrange.h>

#include <QUrl>

#include "rkmdiwindow.h"
#include "rkworkplace.h"

class QEvent;
class QCloseEvent;
class QFrame;
class QLabel;
class QAction;
class QActionGroup;
class RKScriptPreviewIO;
class KActionMenu;
class RKCommandEditorWindow;
class KActionCollection;

/** This class provides a KPart interface to RKCommandEditorWindow. The reason to use this, is so the required menus/menu-items can be merged in on the fly.

@author Thomas Friedrichsmeier
*/
class RKCommandEditorWindowPart : public KParts::Part {
protected:
friend class RKCommandEditorWindow;
	explicit RKCommandEditorWindowPart(QWidget *parent);
	~RKCommandEditorWindowPart ();
};

/** classes wishing to use context help should derive from this, and implement provideContext () */
class RKScriptContextProvider {
public:
	RKScriptContextProvider () {};
	virtual ~RKScriptContextProvider () {};

	/** to be implemented in subclasses. Provide some context, i.e. text *preceding* the cursor position (probably a line, but you may provide chunks in arbitrary size). If line_rev is 0, provide the line, the cursor is in. If line_rev is greater than 0, provide context before that.
	@param context Place the context here
	@returns a chunk of context. A null QString(), if no context was available. */
	virtual QString provideContext (int line_rev) {
		Q_UNUSED (line_rev);
		return QString ();
	};
	/** to be implemented in subclasses. Provide current context for help searches (based on current selection / current cursor position). If not package information is known, leave that empty. */
	virtual void currentHelpContext (QString *symbol, QString *package) = 0;
};

class RKJobSequence;
class RKXMLGUIPreviewArea;
class RKPreviewManager;
class RKCompletionManager;

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
@param encoding encoding to use. If QString (), the default encoding is used.
@param flags @See Combination of RKCommandEditorFlags */
	explicit RKCommandEditorWindow (QWidget *parent, const QUrl &url, const QString& encoding=QString (), int flags=RKCommandEditorFlags::DefaultFlags);
/** destructor */
	~RKCommandEditorWindow ();
/** returns, whether the document was modified since the last save */
	bool isModified() const override;
/** saves the document, returns true on success */
	bool save () override;
/** insert the given text into the document at the current cursor position. Additionally, focuses the view */
	void insertText (const QString &text);
/** set the current text (clear all previous text, and sets new text) */
	void setText (const QString &text);
/** @see restoreScrollPosition (). Note: Currently this saves/restored the cursor position, not necessarily the scroll position. */
	void saveScrollPosition ();
/** @see saveScrollPosition (). Note: Currently this saves/restored the cursor position, not necessarily the scroll position. */
	void restoreScrollPosition ();
/** copy current selection. Wrapper for use by external classes */
	void copy ();

/** reimplemented from RKMDIWindow to return full path of file (if any) */
	QString fullCaption () override;

	void setReadOnly (bool ro);

/** Return current url */
	QUrl url () const;
/** Returns an id string for this document. Meaningful, only when url is empty. For keeping track of split views on unnamed/unsaved windows */
	QString id () const { return _id; };

	QString provideContext (int line_rev) override;
	void currentHelpContext (QString* symbol, QString* package) override;

	void highlightLine (int linenum);
/** Returns the (main, non-preview) texteditor view in this editor. */
	KTextEditor::View* getView () const { return m_view; };
public Q_SLOTS:
/** update Tab caption according to the current url. Display the filename-component of the URL, or - if not available - a more elaborate description of the url. Also appends a "[modified]" if appropriate */
	void updateCaption ();
	void focusIn (KTextEditor::View *);
/** run the currently selected command(s) or line */
	void runCurrent ();
/** run the entire script */
	void runAll ();
/** insert line break and run the (previous) line */
	void enterAndSubmit ();
	void copyLinesToOutput ();

/** selection has changed. Enable / disable actions accordingly */
	void selectionChanged (KTextEditor::View* view);
/** change to the directory of the current script */
	void setWDToScript ();
/** paste the given text at the current cursor position */
	void paste (const QString &text);

/** apply our customizations to the katepart GUI */
	void fixupPartGUI ();
protected:
/** reimplemented from RKMDIWindow: give the editor window a chance to object to being closed (if unsaved) */
	void closeEvent (QCloseEvent *e) override;
	void setWindowStyleHint (const QString& hint) override;
private Q_SLOTS:
/** mark current selection as a block */
	void markBlock(int index);
/** unmark a block */
	void unmarkBlock(int index);
/** run a block */
	void runBlock(int index);
	void clearUnusedBlocks();
/** handler to control when autosaves should be created, preview should be updated */
	void textChanged ();
/** Render the (.Rmd) current script */
	void doRenderPreview ();
/** creates an autosave file */
	void doAutoSave ();
/** handler to control when autosaves should be created */
	void autoSaveHandlerModifiedChanged ();
/** handle any errors during auto-saving */
	void autoSaveHandlerJobFinished (RKJobSequence* seq);
/** document was saved. Update preview, if appropriate */
	void documentSaved ();
private:
friend class RKWardCoreTest;
	void urlChanged();
	KTextEditor::Cursor saved_scroll_position;
	KTextEditor::Document *m_doc;
	KTextEditor::View *m_view;

	void initializeActions (KActionCollection* ac);

	struct BlockRecord {
		KTextEditor::MovingRange* range;
		bool active;
		KTextEditor::Attribute::Ptr attribute;
		QAction* mark;
		QAction* unmark;
		QAction* run;
	};
	QVector<BlockRecord> block_records;
	void initBlocks ();
	void addBlock (int index, const KTextEditor::Range& range);
	void removeBlock (int index, bool was_deleted=false);

	KActionMenu* actionmenu_mark_block;
	KActionMenu* actionmenu_unmark_block;
	KActionMenu* actionmenu_run_block;

	QAction* action_run_all;
	QAction* action_run_current;
	QActionGroup* preview_modes;
	QAction* action_no_preview;
	QAction* action_preview_as_you_type;

	QAction* action_setwd_to_script;

	QUrl previous_autosave_url;
	QTimer autosave_timer;

	QUrl delete_on_close;
	bool visible_to_kateplugins;

	QString _id;
	static QMap<QString, KTextEditor::Document*> unnamed_documents;

	struct PreviewMode {
		QIcon icon;
		QString actionlabel;
		QString previewlabel;
		QString tooltip;
		QString input_ext;
		std::function<QString(const QString&, const QString&, const QString&)> command;
	};
	QList<PreviewMode> preview_mode_list;
	void initPreviewModes();

	RKXMLGUIPreviewArea *preview;
	QTimer preview_timer;
	RKPreviewManager *preview_manager;
	RKScriptPreviewIO *preview_io;
	void changePreviewMode (QAction* mode);
	void discardPreview ();
};

/** Simple class to provide HTML highlighting for arbitrary R code. */
class RKCommandHighlighter {
public:
	enum HighlightingMode {
		RInteractiveSession,
		RScript,
		Automatic
	};
	static void copyLinesToOutput (KTextEditor::View *view, HighlightingMode mode);
	static void setHighlighting (KTextEditor::Document *doc, HighlightingMode mode);
	static QString commandToHTML (const QString &r_command, HighlightingMode mode=RScript);
private:
	static KTextEditor::Document* getDoc ();
	static KTextEditor::Document* _doc;
	static KTextEditor::View* getView ();
	static KTextEditor::View* _view;
};

#endif
