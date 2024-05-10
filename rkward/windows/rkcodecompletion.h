/*
rkcodecompletion - This file is part of the RKWard project. Created: Thu Feb 21 2019
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKCODECOMPLETION_H
#define RKCODECOMPLETION_H

#include <QWidget>
#include <QTimer>
#include <QString>

#include <ktexteditor/view.h>

#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>

class QEvent;
class RKCompletionModelBase;
class RKCodeCompletionSettings;
class RKCodeCompletionModel;
class RKFileCompletionModel;
class RKCallHintModel;
class RKArgumentHintModel;
/** Provides code completions / hints for a KTextEditor::View . Several model are managed, here, with most of the common logic for parsing context, and starting completions is
 *  handled in this class. To use, simply construct a manager as child of the view to provide completions for. */
class RKCompletionManager : public QObject {
	Q_OBJECT
public:
	RKCompletionManager (KTextEditor::View *view, const RKCodeCompletionSettings *settings);
	~RKCompletionManager ();

	QString currentCompletionWord () const;
	KTextEditor::Range currentSymbolRange () const { return symbol_range; };
	KTextEditor::Range currentArgnameRange () const { return argname_range; };
	KTextEditor::Range currentCallRange () const;
	KTextEditor::View* view () const { return (_view); };
	void setLinePrefixes(const QString &_prefix, const QString &_continuation_prefix) { prefix = _prefix; continuation_prefix = _continuation_prefix; };
	void modelGainedLateData(RKCompletionModelBase *model);
public Q_SLOTS:
	void userTriggeredCompletion ();
private Q_SLOTS:
	void lineWrapped (KTextEditor::Document *document, const KTextEditor::Cursor &position);
	void lineUnwrapped (KTextEditor::Document *document, int line);
	void textInserted (KTextEditor::Document *document, const KTextEditor::Cursor &position, const QString &text);
	void textRemoved (KTextEditor::Document *document, const KTextEditor::Range &range, const QString &text);
	void cursorPositionChanged (KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
/** show a code completion box if appropriate. Use tryCompletionProxy () instead, which will call this function after a timeout */
	void tryCompletion ();
private:
	void startModel(KTextEditor::CodeCompletionModel* model, bool start, const KTextEditor::Range &range);
	void updateVisibility();
	bool onlyCallHintShown() const;
	bool eventFilter (QObject *watched, QEvent *event) override;
/** called whenever it might be appropriate to show a code completion box. The box is not shown immediately, but only after a timeout (if at all) */
	void tryCompletionProxy ();
	void updateCallHint ();
	RKCodeCompletionModel *completion_model;
	RKFileCompletionModel *file_completion_model;
	RKCallHintModel *callhint_model;
	RKArgumentHintModel *arghint_model;
	KTextEditor::CodeCompletionModel* kate_keyword_completion_model;
	QTimer *completion_timer;

	const RKCodeCompletionSettings *settings;
	KTextEditor::View *_view;
	KTextEditor::Cursor cached_position;

	KTextEditor::Range symbol_range;
	KTextEditor::Cursor call_opening;
	KTextEditor::Range argname_range;

	bool update_call;
	bool user_triggered;
	bool ignore_next_trigger;
	QString prefix;
	QString continuation_prefix;

	QList<KTextEditor::CodeCompletionModel*> started_models;
	QHash<KTextEditor::CodeCompletionModel*, QString> model_filters;
};

/** Base class for the completion models employed in script editor. Essentially it takes care of the bureaucratic overhead involved in providing a group header */
class RKCompletionModelBase : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface {
	Q_OBJECT
	Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
public:
	explicit RKCompletionModelBase (RKCompletionManager *manager);
	~RKCompletionModelBase ();

	QString filterString (KTextEditor::View *, const KTextEditor::Range &, const KTextEditor::Cursor &) override { return QString (); };
	bool shouldAbortCompletion (KTextEditor::View *, const KTextEditor::Range &, const QString &) override { return false; }
	KTextEditor::CodeCompletionModelControllerInterface::MatchReaction matchingItem (const QModelIndex &) override { return KTextEditor::CodeCompletionModelControllerInterface::None; };
	void executeCompletionItem (KTextEditor::View *view, const KTextEditor::Range & word, const QModelIndex &index) const override;
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

class RKDynamicCompletionsAddition;
class RKCodeCompletionModel : public RKCompletionModelBase {
	Q_OBJECT
public:
	explicit RKCodeCompletionModel (RKCompletionManager *manager);
	~RKCodeCompletionModel ();

	KTextEditor::Range completionRange (KTextEditor::View *view, const KTextEditor::Cursor &position) override;

	void updateCompletionList(const QString& symbol, bool is_help);
	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const override;
	QStringList rawPartialCompletions() const;
private:
	QList<QIcon> icons;
	QStringList names;
	QStringList shortnames;
	QString current_symbol;
	void fetchRCompletions();
	QString r_base_symbol;
	bool is_help;
	RKDynamicCompletionsAddition *rcompletions;
	void addRCompletions();
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
	QStringList rawPartialCompletions() const;
private:
	RObject *function;
	QStringList args;
	int n_formals_args;
	QStringList defs;
	QString fragment;
	QList<int> matches;
	RKDynamicCompletionsAddition *rcompletions;
	void addRCompletions();
};

#include <QThread>
class RKFileCompletionModelWorker : public QThread {
	Q_OBJECT
public:
	explicit RKFileCompletionModelWorker (const QString &string);
Q_SIGNALS:
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

	KTextEditor::Range completionRange (KTextEditor::View *view, const KTextEditor::Cursor &position) override;
	void updateCompletionList (const QString& fragment);
	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const override;
	QStringList rawPartialCompletions() const;
private Q_SLOTS:
	void completionsReady (const QString &string, const QStringList &exes, const QStringList &files);
private:
	void launchThread ();
	QStringList names;
	QString current_fragment;
	RKFileCompletionModelWorker *worker;
};

class RKDynamicCompletionsAddition : public QObject {
	Q_OBJECT
public:
	explicit RKDynamicCompletionsAddition(RKCompletionModelBase *parent);
	~RKDynamicCompletionsAddition();
	void update(const QString &mode, const QString &fragment, const QString &filterprefix, const QStringList &filterlist);
	const QStringList results() const { return filtered_results; };
	const QString fragment() const { return current_fragment; };
	const QString mode() const { return current_mode; };
Q_SIGNALS:
	void resultsComplete();
private:
	void doUpdateFromR();
	void filterResults();
	QString current_mode;
	QString current_fragment;
	QString current_filterprefix;
	QStringList current_filterlist;
	QStringList current_raw_resultlist;
	QStringList filtered_results;
	enum {
		Ready,
		Updating,
		PendingUpdate
	} status;
};

#endif
