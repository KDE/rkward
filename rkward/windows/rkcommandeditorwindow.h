/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2004-2018 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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
#include <QTimer>
#include <QString>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>
#include <ktexteditor/movingrange.h>
#include <ktexteditor/movinginterface.h>

#include <QUrl>

#include "../windows/rkmdiwindow.h"

class QEvent;
class QCloseEvent;
class QFrame;
class QLabel;
class QAction;
class QTemporaryDir;
class KActionMenu;
class KSelectAction;
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

	void updateArgHintWindow ();
protected:
	/** The (keypress) events of the view are filtered to determine, when to show / hide an argument hint */
	bool eventFilter (QObject *, QEvent *e) override;
private:
	RKScriptContextProvider *provider;
	KTextEditor::View *view;

	/** A timer to refresh the hint window periodically. This is a bit sorry, but it's really hard to find out, when the view has been moved, or gains/loses focus. While possible, this approach uses much less code. */
	QTimer updater;
	bool active;
	QLabel *arghints_popup;
};

class RKCodeCompletionModel : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface {
	Q_OBJECT
	Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
public:
	explicit RKCodeCompletionModel (RKCommandEditorWindow* parent);
	~RKCodeCompletionModel ();

	KTextEditor::Range completionRange (KTextEditor::View *view, const KTextEditor::Cursor &position) override;
	QString filterString (KTextEditor::View *, const KTextEditor::Range &, const KTextEditor::Cursor &) override { return QString (); };

	void updateCompletionList (const QString& symbol);
	void completionInvoked (KTextEditor::View *, const KTextEditor::Range &, InvocationType) override;
	void executeCompletionItem (KTextEditor::View *view, const KTextEditor::Range &word, const QModelIndex &index) const override;
	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const override;

	bool isEmpty () const { return names.isEmpty (); };
private:
	QList<QIcon> icons;
	QStringList names;
	QString current_symbol;
	RKCommandEditorWindow *command_editor;
};

class RKJobSequence;
class RKXMLGUIPreviewArea;
class RKPreviewManager;

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
@param read_only Open the file in read-only mode
@param delete_on_close File should be deleted when closing the window. Only respected with read_only=true.
@param use_r_highlighting Initialize the view to use R syntax highlighting. Use, if you're going to edit an R syntax file */
	explicit RKCommandEditorWindow (QWidget *parent, const QUrl url, const QString& encoding=QString (), bool use_r_highlighting=true, bool use_codehinting=true, bool read_only=false, bool delete_on_close=false);
/** destructor */
	~RKCommandEditorWindow ();
/** returns, whether the document was modified since the last save */
	bool isModified () override;
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
	QString currentCompletionWord () const;

	void highlightLine (int linenum);
public slots:
/** update Tab caption according to the current url. Display the filename-component of the URL, or - if not available - a more elaborate description of the url. Also appends a "[modified]" if appropriate */
	void updateCaption ();
/** called whenever it might be appropriate to show a code completion box. The box is not shown immediately, but only after a timeout (if at all) */
	void tryCompletionProxy (KTextEditor::Document*);
/** show a code completion box if appropriate. Use tryCompletionProxy () instead, which will call this function after a timeout */
	void tryCompletion ();
	void setPopupMenu ();
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

	QAction* fileSaveAction () { return file_save; };
	QAction* fileSaveAsAction () { return file_save_as; };
protected:
/** reimplemented from RKMDIWindow: give the editor window a chance to object to being closed (if unsaved) */
	void closeEvent (QCloseEvent *e) override;
	void setWindowStyleHint (const QString& hint) override;
private slots:
/** mark current selection as a block */
	void markBlock ();
/** unmark a block */
	void unmarkBlock ();
/** run a block */
	void runBlock ();
	void clearUnusedBlocks ();
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
private:
	KTextEditor::Cursor saved_scroll_position;
	KTextEditor::Document *m_doc;
	KTextEditor::View *m_view;
	KTextEditor::CodeCompletionInterface *cc_iface;
	KTextEditor::MovingInterface *smart_iface;
	RKFunctionArgHinter *hinter;
	RKCodeCompletionModel *completion_model;

	QTimer *completion_timer;

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

	QAction *file_save, *file_save_as;

	KActionMenu* actionmenu_mark_block;
	KActionMenu* actionmenu_unmark_block;
	KActionMenu* actionmenu_run_block;

	QAction* action_run_all;
	QAction* action_run_current;
	KSelectAction* actionmenu_preview;

	enum PreviewMode {
		NoPreview,
		RMarkdownPreview,
		RKOutputPreview,
		GraphPreview,
		ConsolePreview,
	};

	QAction* action_setwd_to_script;

	QUrl previous_autosave_url;
	QTimer autosave_timer;

	QUrl delete_on_close;

	QString _id;
	static QMap<QString, KTextEditor::Document*> unnamed_documents;

	RKXMLGUIPreviewArea *preview;
	QTimer preview_timer;
	RKPreviewManager *preview_manager;
	QTemporaryDir *preview_dir;
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
	static QString commandToHTML (const QString r_command, HighlightingMode mode=RScript);
private:
	static KTextEditor::Document* getDoc ();
	static KTextEditor::Document* _doc;
	static KTextEditor::View* getView ();
	static KTextEditor::View* _view;
};

#endif
