/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2004-2019 by Thomas Friedrichsmeier
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
class QActionGroup;
class QTemporaryDir;
class QFile;
class KActionMenu;
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

class RKCodeCompletionModel;
class RKFileCompletionModel;
class RKCallHintModel;
class RKArgumentHintModel;
class RKCompletionManager : public QObject {
	Q_OBJECT
public:
	RKCompletionManager (KTextEditor::View *view);
	~RKCompletionManager ();

	QString currentCompletionWord () const;
	KTextEditor::Range currentSymbolRange () const { return symbol_range; };
	KTextEditor::Range currentArgnameRange () const { return argname_range; };
	KTextEditor::Range currentCallRange () const;
	KTextEditor::View* view () const { return (_view); };
private slots:
	void lineWrapped (KTextEditor::Document *document, const KTextEditor::Cursor &position);
	void lineUnwrapped (KTextEditor::Document *document, int line);
	void textInserted (KTextEditor::Document *document, const KTextEditor::Cursor &position, const QString &text);
	void textRemoved (KTextEditor::Document *document, const KTextEditor::Range &range, const QString &text);
	void cursorPositionChanged (KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
/** show a code completion box if appropriate. Use tryCompletionProxy () instead, which will call this function after a timeout */
	void tryCompletion ();
private:
/** called whenever it might be appropriate to show a code completion box. The box is not shown immediately, but only after a timeout (if at all) */
	void tryCompletionProxy ();
	void updateVisibility ();
	void updateCallHint ();
	KTextEditor::CodeCompletionInterface *cc_iface;
	RKCodeCompletionModel *completion_model;
	RKFileCompletionModel *file_completion_model;
	RKCallHintModel *callhint_model;
	RKArgumentHintModel *arghint_model;
	KTextEditor::CodeCompletionModel* kate_keyword_completion_model;
	QTimer *completion_timer;

	KTextEditor::View *_view;
	KTextEditor::Cursor cached_position;

	KTextEditor::Range symbol_range;
	KTextEditor::Cursor call_opening;
	KTextEditor::Range argname_range;

	bool update_call;
	bool active;

	QList<KTextEditor::CodeCompletionModel*> active_models;
};

/** Base class for the completion models employed in script editor. Essentially it takes care of the bureaucratic overhead involved in providing a group header */
class RKCompletionModelBase : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface {
public:
	explicit RKCompletionModelBase (RKCompletionManager *manager);
	~RKCompletionModelBase ();

	QString filterString (KTextEditor::View *, const KTextEditor::Range &, const KTextEditor::Cursor &) override { return QString (); };
	bool shouldAbortCompletion (KTextEditor::View *, const KTextEditor::Range &, const QString &) override { return false; }
	KTextEditor::CodeCompletionModelControllerInterface::MatchReaction matchingItem (const QModelIndex &) override { return KTextEditor::CodeCompletionModelControllerInterface::None; };

	int rowCount (const QModelIndex &parent) const override;
	QModelIndex index (int row, int column, const QModelIndex &parent = QModelIndex ()) const override;
	QModelIndex parent (const QModelIndex &index) const override;

	bool isHeaderItem (const QModelIndex &parent) const { return (parent.internalId () == HeaderItem); };
	bool isEmpty () const { return (n_completions == 0); };
protected:
	int n_completions;
	RKCompletionManager *manager;
private:
	enum {
		// forcing non-0, so function will not return true on null-QModelIndex
		HeaderItem = 1,
		LeafItem = 2
	};
};

class RKCodeCompletionModel : public RKCompletionModelBase {
	Q_OBJECT
public:
	explicit RKCodeCompletionModel (RKCompletionManager *manager);
	~RKCodeCompletionModel ();

	KTextEditor::Range completionRange (KTextEditor::View *view, const KTextEditor::Cursor &position) override;

	void updateCompletionList (const QString& symbol);
	void executeCompletionItem (KTextEditor::View *view, const KTextEditor::Range &word, const QModelIndex &index) const override;
	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const override;
private:
	QList<QIcon> icons;
	QStringList names;
	QString current_symbol;
};

class RKCallHintModel : public RKCompletionModelBase {
	Q_OBJECT
public:
	explicit RKCallHintModel (RKCompletionManager *manager);
	void setFunction (RObject *function);

	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const override;
	KTextEditor::Range completionRange (KTextEditor::View *view, const KTextEditor::Cursor &position) override;
	RObject *currentFunction () const { return function; };
private:
	RObject *function;
	QString name;
	QString formals;
	QVariantList formatting;
};

class RKArgumentHintModel : public RKCompletionModelBase {
	Q_OBJECT
public:
	explicit RKArgumentHintModel (RKCompletionManager *manager);
	void updateCompletionList (RObject *function, const QString& argument);

	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const override;
	KTextEditor::Range completionRange (KTextEditor::View *view, const KTextEditor::Cursor &position) override;
private:
	RObject *function;
	QStringList args;
	QStringList defs;
	QString fragment;
	QList<int> matches;
};

#include <QThread>
class RKFileCompletionModelWorker : public QThread {
	Q_OBJECT
public:
	explicit RKFileCompletionModelWorker (const QString &string);
signals:
	void completionsReady (const QString &string, const QStringList &exes, const QStringList &files);
private:
	void run () override;
	QString string;
};

class RKFileCompletionModel : public RKCompletionModelBase {
	Q_OBJECT
public:
	explicit RKFileCompletionModel (RKCompletionManager *manager);
	~RKFileCompletionModel ();

	void updateCompletionList (const QString& fragment);
	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const override;
private slots:
	void completionsReady (const QString &string, const QStringList &exes, const QStringList &files);
private:
	void launchThread ();
	QStringList names;
	QString current_fragment;
	RKFileCompletionModelWorker *worker;
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

	void highlightLine (int linenum);
public slots:
/** update Tab caption according to the current url. Display the filename-component of the URL, or - if not available - a more elaborate description of the url. Also appends a "[modified]" if appropriate */
	void updateCaption ();
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
/** document was saved. Update preview, if appropriate */
	void documentSaved ();
private:
	KTextEditor::Cursor saved_scroll_position;
	KTextEditor::Document *m_doc;
	KTextEditor::View *m_view;
	KTextEditor::MovingInterface *smart_iface;
	RKFunctionArgHinter *hinter;

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
	QActionGroup* preview_modes;
	QAction* action_no_preview;
	QAction* action_preview_as_you_type;

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
	QFile *preview_input_file;
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
	static QString commandToHTML (const QString r_command, HighlightingMode mode=RScript);
private:
	static KTextEditor::Document* getDoc ();
	static KTextEditor::Document* _doc;
	static KTextEditor::View* getView ();
	static KTextEditor::View* _view;
};

#endif
