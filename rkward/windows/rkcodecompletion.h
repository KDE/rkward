/***************************************************************************
                          rkcodecompletion  -  description
                             -------------------
    begin                : Thu Feb 21 2019
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
#ifndef RKCODECOMPLETION_H
#define RKCODECOMPLETION_H

#include <QWidget>
#include <QTimer>
#include <QString>

#include <ktexteditor/view.h>

#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>

class QEvent;
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
	bool eventFilter (QObject *watched, QEvent *event) override;
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
	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const override;
	QString partialCompletion (bool* exact_match);
private:
	QList<QIcon> icons;
	QStringList names;
	QString current_symbol;
};

class RObject;
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
	QString partialCompletion (bool *exact);
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
	QString partialCompletion (bool *exact);
private slots:
	void completionsReady (const QString &string, const QStringList &exes, const QStringList &files);
private:
	void launchThread ();
	QStringList names;
	QString current_fragment;
	RKFileCompletionModelWorker *worker;
};

#endif
