/*
rkcodecompletion - This file is part of RKWard (https://rkward.kde.org). Created: Thu Feb 21 2019
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkcodecompletion.h"

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <KLocalizedString>
#include <KActionCollection>

#include <QApplication>
#include <QKeyEvent>
#include <QIcon>
#include <QDir>
#include <QAction>
#include <QTimer>

#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardicons.h"
#include "../core/rfunctionobject.h"
#include "../core/robjectlist.h"
#include "../settings/rksettingsmodulecommandeditor.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rkrinterface.h"

#include "../debug.h"


class RKCompletionNotifierModel : public RKCompletionModelBase {
public:
	explicit RKCompletionNotifierModel(RKCompletionManager *manager) : RKCompletionModelBase(manager) {};
	KTextEditor::Range completionRange (KTextEditor::View *, const KTextEditor::Cursor &position) override {
		return KTextEditor::Range (position, position);
	}
	/** reimplemented in order to receive notification, when completion is invoked by user shortcut. */
	void completionInvoked (KTextEditor::View*, const KTextEditor::Range&, KTextEditor::CodeCompletionModel::InvocationType invocationType) override {
		RK_TRACE (COMMANDEDITOR);

		if (invocationType == KTextEditor::CodeCompletionModel::UserInvocation) {
			// NOTE: Without this short timeout, the completion window will sometimes disappear, again, right away.
			QTimer::singleShot(50, manager, [this](){ manager->userTriggeredCompletion();});
		}
	}
	QVariant data (const QModelIndex&, int) const override {
		return QVariant ();
	}
};

//////////////////////// RKCompletionManager //////////////////////
#include <KAboutData>
#include <KMessageBox>
RKCompletionManager::RKCompletionManager(KTextEditor::View* view, const RKCodeCompletionSettings *settings) : QObject(view), settings(settings) {
	RK_TRACE (COMMANDEDITOR);

	_view = view;
	user_triggered = false;
	ignore_next_trigger = false;
	update_call = true;
	cached_position = KTextEditor::Cursor (-1, -1);

	view->setAutomaticInvocationEnabled (false);
	completion_model = new RKCodeCompletionModel (this);
	file_completion_model = new RKFileCompletionModel (this);
	callhint_model = new RKCallHintModel (this);
	arghint_model = new RKArgumentHintModel (this);
	view->registerCompletionModel (new RKCompletionNotifierModel (this));  // (at least) one model needs to be registered, so we will know, when completion was triggered by the user (shortcut)
	completion_timer = new QTimer (this);
	completion_timer->setSingleShot (true);
	connect (completion_timer, &QTimer::timeout, this, &RKCompletionManager::tryCompletion);
	connect (view->document (), &KTextEditor::Document::textInserted, this, &RKCompletionManager::textInserted);
	connect (view->document (), &KTextEditor::Document::textRemoved, this, &RKCompletionManager::textRemoved);
	connect (view->document (), &KTextEditor::Document::lineWrapped, this, &RKCompletionManager::lineWrapped);
	connect (view->document (), &KTextEditor::Document::lineUnwrapped, this, &RKCompletionManager::lineUnwrapped);
	connect (view, &KTextEditor::View::cursorPositionChanged, this, &RKCompletionManager::cursorPositionChanged);
	const QObjectList children = _view->children ();
	for (QObjectList::const_iterator it = children.constBegin(); it != children.constEnd (); ++it) {
		(*it)->installEventFilter (this);  // to handle Tab-key; installing on the view, alone, is not enough.
	}

	// HACK: I just can't see to make the object name completion model play nice with automatic invocation.
	//       However, there is no official way to invoke all registered models, manually. So we try to hack our way
	//       to a pointer to the default kate keyword completion model
	kate_keyword_completion_model = KTextEditor::Editor::instance ()->findChild<KTextEditor::CodeCompletionModel *> ();
	if (!kate_keyword_completion_model) kate_keyword_completion_model = view->findChild<KTextEditor::CodeCompletionModel *> (QString());
}

RKCompletionManager::~RKCompletionManager () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCompletionManager::tryCompletionProxy () {
	if (_view->isCompletionActive ()) {
		// Handle this in the next event cycle, as more than one event may trigger
		completion_timer->start (0);
	} else if (settings->autoEnabled ()) {
		completion_timer->start (settings->autoTimeout ());
	}
}

QString RKCompletionManager::currentCompletionWord () const {
	RK_TRACE (COMMANDEDITOR);

	if (symbol_range.isValid ()) return _view->document ()->text (symbol_range);
	return QString ();
}

void RKCompletionManager::userTriggeredCompletion () {
	RK_TRACE (COMMANDEDITOR);

	user_triggered = true;
	completion_timer->stop ();
	tryCompletion ();
	user_triggered = false;
}

void RKCompletionManager::tryCompletion () {
	RK_TRACE (COMMANDEDITOR);
	if (!_view) {
		// NOTE: This should not be possible, because the connections  have not been set up in the constructor, in this case.
		RK_ASSERT (_view);
		return;
	}
	if (ignore_next_trigger) {
		ignore_next_trigger = false;
		return;
	}

	KTextEditor::Document *doc = _view->document ();
	KTextEditor::Cursor c = _view->cursorPosition();
	cached_position = c;
	uint para=c.line(); int cursor_pos=c.column();

	QString current_line = doc->line (para);
	int start;
	int end;
	RKCommonFunctions::getCurrentSymbolOffset (current_line, cursor_pos-1, false, &start, &end);
	symbol_range = KTextEditor::Range (para, start, para, end);
	bool is_help = (start >= 1) && (current_line.at(start-1) == '?');
	if (!user_triggered) {
		if (end > cursor_pos) {
			symbol_range = KTextEditor::Range ();   // Only hint when at the end of a word/symbol: https://mail.kde.org/pipermail/rkward-devel/2015-April/004122.html
		} else {
			if (doc->defaultStyleAt (c) == KSyntaxHighlighting::Theme::TextStyle::Comment) symbol_range = KTextEditor::Range ();	// do not hint while in comments
		}
	}

	QString word = currentCompletionWord ();
	if (user_triggered || (word.length () >= settings->autoMinChars ())) {
		QString filename;
		// as a very simple heuristic: If the current symbol starts with a quote, we should probably attempt file name completion, instead of symbol name completion
		if (word.startsWith ('\"') || word.startsWith ('\'') || word.startsWith ('`')) {
			symbol_range.setStart (KTextEditor::Cursor (symbol_range.start ().line (), symbol_range.start ().column () + 1));   // confine range to inside the quotes.
			filename = word.mid (1);
			word.clear ();
		}

		completion_model->updateCompletionList(word, is_help);
		file_completion_model->updateCompletionList(filename);
	} else {
		completion_model->updateCompletionList(QString(), false);
		file_completion_model->updateCompletionList(QString());
	}
	RK_DEBUG(EDITOR, DL_DEBUG, "completion symbol range %d, %d -> %d, %d", symbol_range.start().line(), symbol_range.start().column(), symbol_range.end().line(), symbol_range.end().column());

	updateCallHint ();

	// update ArgHint.
	argname_range = KTextEditor::Range (-1, -1, -1, -1);
	// Named arguments are just like regular symbols, *but* we must require that they are preceeded by either a ',', or the opening '(', immediately.
	// Otherwise, they are an argument value expression, for sure.
	// We also assume (foolishly), that if we are on a new line, we're probably starting a new arg. TODO: Actually check this!
	if (callhint_model->currentFunction ()) {
		argname_range = symbol_range;
		for (int i = symbol_range.start ().column () - 1; i >= 0; --i) {
			QChar c = current_line.at (i);
			if (c == ',' || c == '(') {
				break;
			} else if (!c.isSpace ()) {
				argname_range = KTextEditor::Range (-1, -1, -1, -1);
			}
		}
	}
	arghint_model->updateCompletionList (callhint_model->currentFunction (), argname_range.isValid () ? doc->text (argname_range) : QString ());

	updateVisibility ();
}

bool isCode (KTextEditor::Document* doc, int line, int column) {
	KSyntaxHighlighting::Theme::TextStyle style = doc->defaultStyleAt (KTextEditor::Cursor (line, column));
	if (style == KSyntaxHighlighting::Theme::TextStyle::Comment) return false;
	if (style == KSyntaxHighlighting::Theme::TextStyle::String) return false;
	if (style == KSyntaxHighlighting::Theme::TextStyle::Char) return false;
	return true;
}

void RKCompletionManager::updateCallHint () {
	RK_TRACE (COMMANDEDITOR);

	if (_view->isCompletionActive () && !update_call) return;
	update_call = false;

	int line = cached_position.line () + 1;
	QString full_context;
	int potential_symbol_end = -2;
	int parenthesis_level = 0;
	int prefix_offset = 0;
	KTextEditor::Document *doc = _view->document ();
	while (potential_symbol_end < -1 && line >= 0) {
		--line;
		QString context_line = doc->line (line);
		if (!prefix.isEmpty()) {   // For skipping interactive output sections in console. Empty for RKCommandEditorWindow
			if (context_line.startsWith(prefix)) {
				prefix_offset = prefix.length();
			} else if (context_line.startsWith(continuation_prefix)) {
				prefix_offset = continuation_prefix.length();
			} else {
				continue;
			}
			context_line = context_line.mid(prefix_offset);
		}
		full_context.prepend (context_line);

		int pos = context_line.length () - 1;
		if (line == cached_position.line ()) pos = cached_position.column () - 1 - prefix_offset;   // when on current line, look backward from cursor position, not line end
		for (int i = pos; i >= 0; --i) {
			QChar c = context_line.at (i);
			if (c == '(') {
				if (isCode (doc, line, i)) {
					if (--parenthesis_level < 0) {
						potential_symbol_end = i - 1;
						break;
					}
				}
			} else if (c == ')') {
				if (isCode (doc, line, i)) {
					++parenthesis_level;
				}
			}
		}
	}

	// now find out where the symbol to the left of the opening brace ends
	// there cannot be a line-break between the opening brace, and the symbol name (or can there?), so no need to fetch further context
	while ((potential_symbol_end >= 0) && full_context.at (potential_symbol_end).isSpace ()) {
		--potential_symbol_end;
	}

	// now identify the symbol and object (if any)
	RObject *object = nullptr;
	call_opening = KTextEditor::Cursor (-1, -1);
	if (potential_symbol_end > 0) {
		QString effective_symbol = RKCommonFunctions::getCurrentSymbol (full_context, potential_symbol_end);
		if (!effective_symbol.isEmpty ()) {
			object = RObjectList::getObjectList ()->findObject (effective_symbol);
			call_opening = KTextEditor::Cursor (line, potential_symbol_end+1);
		}
	}

	callhint_model->setFunction (object);
}

void RKCompletionManager::startModel( KTextEditor::CodeCompletionModel *model, bool start, const KTextEditor::Range &range) {
	if (start) {
		if (!range.isValid()) start = false;
	}
	if (start) {
		if (!started_models.contains (model)) {
			_view->startCompletion (range, model);
			started_models.append (model);
		}
		auto ci = dynamic_cast<KTextEditor::CodeCompletionModelControllerInterface*>(model);
		model_filters.insert(model, ci ? ci->filterString(view(), range, view()->cursorPosition()) : QString());
	} else {
		started_models.removeAll (model);
	}
}

void RKCompletionManager::updateVisibility () {
	RK_TRACE (COMMANDEDITOR);

	if (user_triggered || !_view->isCompletionActive()) {
		started_models.clear ();
	}

	bool min_len = (currentCompletionWord().length() >= settings->autoMinChars()) || user_triggered;
	startModel(completion_model, min_len && settings->isEnabled(RKCodeCompletionSettings::Object), symbol_range);
	startModel(file_completion_model, min_len && settings->isEnabled(RKCodeCompletionSettings::Filename), symbol_range);
	if (kate_keyword_completion_model && settings->isEnabled(RKCodeCompletionSettings::AutoWord)) {
		// Model needs to update, first, as we have not handled it in tryCompletion:
		if (min_len) kate_keyword_completion_model->completionInvoked(view(), symbol_range, KTextEditor::CodeCompletionModel::ManualInvocation);
		startModel(kate_keyword_completion_model, min_len, symbol_range);
	}
// NOTE: Freaky bug in KF 5.44.0: Call hint will not show for the first time, if logically above the primary screen. TODO: provide patch for kateargumenthinttree.cpp:166pp
	startModel(callhint_model, settings->isEnabled(RKCodeCompletionSettings::Calltip), currentCallRange());
	startModel(arghint_model, settings->isEnabled(RKCodeCompletionSettings::Arghint), argname_range);
}

void RKCompletionManager::modelGainedLateData(RKCompletionModelBase* model) {
	RK_TRACE (COMMANDEDITOR);

	RK_ASSERT(started_models.removeAll(model)); // should have been started before, thus be in the list
	startModel(model, true, model->completionRange(view(), view()->cursorPosition()));
}

void RKCompletionManager::textInserted (KTextEditor::Document*, const KTextEditor::Cursor& position, const QString& text) {
	if (_view->isCompletionActive ()) {
		if (position < call_opening) update_call = true;
		else if (text.contains (QChar ('(')) || text.contains (QChar (')'))) update_call = true;
	}
	tryCompletionProxy();
}

void RKCompletionManager::textRemoved (KTextEditor::Document*, const KTextEditor::Range& range, const QString& text) {
	if (_view->isCompletionActive ()) {
		if (range.start () < call_opening) update_call = true;
		else if (text.contains (QChar ('(')) || text.contains (QChar (')'))) update_call = true;
	}
	tryCompletionProxy();
}

void RKCompletionManager::lineWrapped (KTextEditor::Document* , const KTextEditor::Cursor& ) {
// should already have been handled by textInserted()
	tryCompletionProxy ();
}

void RKCompletionManager::lineUnwrapped (KTextEditor::Document* , int ) {
// should already have been handled by textRemoved()
	tryCompletionProxy ();
}

void RKCompletionManager::cursorPositionChanged (KTextEditor::View* view, const KTextEditor::Cursor& newPosition) {
	if (_view->isCompletionActive ()) {
		if (newPosition < call_opening) update_call = true;
		else {
			QString text = view->document ()->text (KTextEditor::Range (newPosition, cached_position));
			if (text.contains (QChar ('(')) || text.contains (QChar (')'))) update_call = true;
		}
		tryCompletionProxy ();
	} else if (settings->autoCursorActivated ()) {
		tryCompletionProxy ();
	}
}

KTextEditor::Range RKCompletionManager::currentCallRange () const {
	return KTextEditor::Range (call_opening, _view->cursorPosition ());
}

bool modelHasGroups(KTextEditor::CodeCompletionModel *model) {
	return (model->hasGroups() || (model->rowCount() && model->rowCount(model->index(0,0)))); // kate_keyword_completion_model claims to have no groups, but is still grouped
}

QStringList genPartialCompletions(KTextEditor::CodeCompletionModel *model, const QString &lead, const QModelIndex &root) {
	QStringList candidates;
	for (int i = 0; i < model->rowCount(root); ++i) {
		QString candidate = model->index(i, KTextEditor::CodeCompletionModel::Name, root).data().toString();
		if (candidate.startsWith(lead)) candidates.append(candidate.mid(lead.size()));
	}
	return candidates;
}

QStringList genPartialCompletions(KTextEditor::CodeCompletionModel *model, const QString &lead) {
	QStringList candidates;
	if (modelHasGroups(model)) {
		for (int i = 0; i < model->rowCount(); ++i) {
			candidates += genPartialCompletions(model, lead, model->index(i, 0));
		}
	} else {
		candidates = genPartialCompletions(model, lead, QModelIndex());
	}
	return candidates;
}

QStringList genPartialCompletions(const QStringList &matches, const QString &lead) {
	QStringList ret;
	ret.reserve(matches.size());
	for (int i = 0; i < matches.size(); ++i) {
		const QString &m = matches[i];
		if (!m.startsWith(lead)) continue;
		ret.append(m.mid(lead.length()));
	}
	return ret;
}

QString findCommonCompletion (const QStringList &candidates) {
	RK_TRACE (COMMANDEDITOR);
	RK_DEBUG(COMMANDEDITOR, DL_DEBUG, "Looking for common completion among set of %d", candidates.size ());

	QString ret;
	bool first = true;
	for (int i = candidates.size() - 1; i >= 0; --i) {
		const QString &candidate = candidates.at(i);

		if (first) {
			ret = candidate;
			first = false;
		} else {
			if (ret.length() > candidate.length()) {
				ret = ret.left(candidate.length());
			}

			for (int c = 0; c < ret.length(); ++c) {
				if (ret[c] != candidate[c]) {
					if (!c) return QString();

					ret = ret.left (c);
					break;
				}
			}
		}
	}

	return ret;
}

bool isModelEmpty(KTextEditor::CodeCompletionModel* model, const QString &filter) {
	int rootcount = model->rowCount();
	bool groups = modelHasGroups(model);
	for (int i = 0; i < rootcount; ++i) {
		auto pindex = model->index(i, 0, QModelIndex());
		if (groups) {
			int groupcount = model->rowCount(pindex);
			for (int j = 0; j < groupcount; ++j) {
				if (model->index(j, KTextEditor::CodeCompletionModel::Name, pindex).data().toString().startsWith(filter)) return false;
			}
		} else {
			if (model->index(i, KTextEditor::CodeCompletionModel::Name, QModelIndex()).data().toString().startsWith(filter)) return false;
		}
	}
	return true;
}

bool RKCompletionManager::onlyCallHintShown() const {
	for (int i = 0; i < started_models.size(); ++i) {
		auto model = started_models[i];
		if (model == callhint_model) continue;
		if (!isModelEmpty(model, model_filters[model])) {
			return false;
		}
	}
	return true;
}

bool RKCompletionManager::eventFilter (QObject*, QEvent* event) {
	if (event->type () == QEvent::KeyPress || event->type () == QEvent::ShortcutOverride) {
		RK_TRACE (COMMANDEDITOR);	// avoid loads of empty traces, putting this here
		QKeyEvent *k = static_cast<QKeyEvent *> (event);

		if (!_view->isCompletionActive()) {
			if (k->type () == QEvent::ShortcutOverride) return true; // retriggered as key event
			if (settings->tabKeyInvokesCompletion() && k->key() == Qt::Key_Tab && k->modifiers() == Qt::NoModifier) {
				userTriggeredCompletion();
				return true;
			}
			return false;
		}

		// If only the calltip is active, make sure the tab-key and enter behave as a regular keys. There is no completion in this case.
		if ((k->key() == Qt::Key_Tab) || (k->key() == Qt::Key_Return) || (k->key() == Qt::Key_Enter) || (k->key() == Qt::Key_Backtab) || (k->key() == Qt::Key_Up) || (k->key() == Qt::Key_Down)) {
			if (onlyCallHintShown()) {
				completion_timer->start(0);
				_view->abortCompletion(); // That's a bit lame, but the least hacky way to get the key into the document. completion_timer was started, so
							// the completion window should come back up, without delay
				return false;
			}
		}

		if ((k->modifiers() == Qt::NoModifier) && ((k->key () == Qt::Key_Return) || (k->key () == Qt::Key_Enter))) {
			if (k->type () == QEvent::ShortcutOverride) {
				// Too bad for all the duplicate work, but the event will re-trigger as a keypress event, and we need to intercept that one, too.
				return true;
			}
			bool callhint_active = started_models.contains(callhint_model);
			_view->forceCompletion();
			started_models.clear();  // forceCompletion(), also aborts. Keep track of that.
// TODO: If nothing was actually modified, should return press be sent? Configurable?
			completion_timer->stop();  // do not bring up completion right again, but do restore the call-hint, if applicable
			if (callhint_active) startModel(callhint_model, true, currentCallRange());
			return true;
		}

		if (k->key () == Qt::Key_Tab && (!k->modifiers ())) {
			// Try to do partial completion. Unfortunately, the base implementation in ktexteditor is totally broken (inserts the whole partial completion, without removing the start).
			// This is terribly inefficient, but fortunately, it only needs to be called, when Tab is actually pressed.
			QStringList candidate_completions;
			for (int i = 0; i < started_models.size(); ++i) {
				auto model = started_models[i];
				if (model == callhint_model) continue;
				if (model == arghint_model) candidate_completions += arghint_model->rawPartialCompletions();
				else if (model == completion_model) candidate_completions += completion_model->rawPartialCompletions();
				else if (model == file_completion_model) candidate_completions += file_completion_model->rawPartialCompletions();
				else candidate_completions += genPartialCompletions(model, model_filters[model]);
			}
			QString comp = findCommonCompletion(candidate_completions);

			if (!comp.isEmpty()) {
				RK_DEBUG(COMMANDEDITOR, DL_WARNING, "Tab completion: %s", qPrintable(comp));
				if (k->type () == QEvent::ShortcutOverride) {
					// Too bad for all the duplicate work, but the event will re-trigger as a keypress event, and we need to intercept that one, too.
					return true;
				}
				view ()->document ()->insertText (view ()->cursorPosition (), comp);
				if (candidate_completions.size() == 1) {
					// Ouch, how messy. We want to make sure completion stops (except for any call hints), and is not re-triggered by the insertion, itself
					bool callhint_active = started_models.contains(callhint_model);
					_view->abortCompletion ();
					started_models.clear ();
					completion_timer->stop();
					if (callhint_active) startModel(callhint_model, true, currentCallRange());
				}
			} else {
				QApplication::beep ();
			}
			return true;
		} else if ((k->key () == Qt::Key_Up || k->key () == Qt::Key_Down) && _view->isCompletionActive ()) {
			bool navigate = (settings->cursorNavigatesCompletions() && k->modifiers() == Qt::NoModifier) || (!settings->cursorNavigatesCompletions() && k->modifiers() == Qt::AltModifier);

			// Make up / down-keys (without alt) navigate in the document (aborting the completion)
			// Make alt+up / alt+down navigate in the completion list
			if (navigate) {
				if (k->type() == QKeyEvent::ShortcutOverride) {
					k->accept();  // -> will be re-sent as a regular key event, then handled, below
					return true;
				} else {
					if (k->type() != QKeyEvent::KeyPress) return true;  // eat the release event
				}

				// No, we cannot just send a fake key event, easily...
				KActionCollection *kate_edit_actions = view ()->findChild<KActionCollection*> ("edit_actions");
				QAction *action = kate_edit_actions ? (kate_edit_actions->action (k->key () == Qt::Key_Up ? "move_line_up" : "move_line_down")) : nullptr;
				if (!action) {
					kate_edit_actions = view ()->actionCollection ();
					action = kate_edit_actions ? (kate_edit_actions->action (k->key () == Qt::Key_Up ? "move_line_up" : "move_line_down")) : nullptr;
				}
				if (action) action->trigger ();
				else RK_ASSERT (action);
				return true;
			} else {
				_view->abortCompletion ();
				return false;
			}
		}
	}

	return false;
}

//////////////////////// RKCompletionModelBase ////////////////////

RKCompletionModelBase::RKCompletionModelBase (RKCompletionManager *manager) : KTextEditor::CodeCompletionModel (manager) {
	RK_TRACE (COMMANDEDITOR);
	n_completions = 0;
	RKCompletionModelBase::manager = manager;
	setHasGroups(true);
}

RKCompletionModelBase::~RKCompletionModelBase () {
	RK_TRACE (COMMANDEDITOR);
}

QModelIndex RKCompletionModelBase::index (int row, int column, const QModelIndex& parent) const {
	if (!parent.isValid ()) { // root
		if (row == 0) return createIndex (row, column, quintptr (HeaderItem));
	} else if (isHeaderItem (parent)) {
		return createIndex (row, column, quintptr (LeafItem));
	}
	return QModelIndex ();
}

QModelIndex RKCompletionModelBase::parent (const QModelIndex& index) const {
	if (index.isValid () && !isHeaderItem (index)) {
		return createIndex (0, 0, quintptr (HeaderItem));
	}
	return QModelIndex ();
}

int RKCompletionModelBase::rowCount (const QModelIndex& parent) const {
	if (!parent.isValid ()) return (n_completions ? 1 : 0); // header item, if list not empty
	if (isHeaderItem (parent)) return n_completions;
	return 0;  // no children on completion entries
}

void RKCompletionModelBase::executeCompletionItem (KTextEditor::View *view, const KTextEditor::Range & word, const QModelIndex &index) const {
// I have no idea, why this is needed at all, but sometimes (and inconsitently!) the default implementation will replace the wrong range, without this.
// Importantly, the RKFileCompletionModel replaces with the wrong range, the *second* time it gets called.
	KTextEditor::CodeCompletionModel::executeCompletionItem(view, const_cast<RKCompletionModelBase*>(this)->completionRange(view, word.end()), index);
}

//////////////////////// RKCodeCompletionModel ////////////////////

RKCodeCompletionModel::RKCodeCompletionModel (RKCompletionManager *manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);
	rcompletions = new RKDynamicCompletionsAddition(this);
	connect(rcompletions, &RKDynamicCompletionsAddition::resultsComplete, this, &RKCodeCompletionModel::addRCompletions);
}

RKCodeCompletionModel::~RKCodeCompletionModel () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCodeCompletionModel::updateCompletionList(const QString& symbol, bool is_help) {
	RK_TRACE (COMMANDEDITOR);

	if (current_symbol == symbol) return;	// already up to date
	beginResetModel ();

	RObject::ObjectList matches;
	QStringList objectpath = RObject::parseObjectPath (symbol);
	if (!objectpath.isEmpty () && !objectpath[0].isEmpty ()) {  // Skip completion, if the current symbol is '""' (or another empty quote), for instance
		matches = RObjectList::getObjectList ()->findObjectsMatching (symbol);
	}

	// copy the map to two lists. For one thing, we need an int indexable storage, for another, caching this information is safer
	// in case objects are removed while the completion mode is active.
	n_completions = matches.size();
	icons.clear();
	icons.reserve(n_completions);
	shortnames.clear();
	shortnames.reserve(n_completions);
	names = RObject::getFullNames (matches, RKSettingsModuleCommandEditor::completionSettings()->options());  // NOTE: Intentionally using the script editor completion settings object, here. the completion options are shared with the console!
	for (int i = 0; i < n_completions; ++i) {
		icons.append (RKStandardIcons::iconForObject (matches[i]));
		shortnames.append(matches[i]->getShortName());
	}

	if ((objectpath.size() == 2 || objectpath.size() == 3) && objectpath.at(1).startsWith("::")) {
		rcompletions->update(objectpath.at(1), objectpath.at(0), objectpath.value(2), shortnames);
	} else if (is_help) {
		rcompletions->update("?", symbol, symbol, shortnames);
	} else if (objectpath.size() > 1) {
		QString op = objectpath.at(objectpath.size() - 1 - objectpath.size() % 2);
		QString start = objectpath.mid(0, objectpath.size() - 1 - objectpath.size() % 2).join("");
		if (op == "$" || op == "@") {
			rcompletions->update(op, start, objectpath.size() % 2 ? objectpath.last() : QString(), shortnames);
		}
	}

	current_symbol = symbol;

	endResetModel ();
}

void RKCodeCompletionModel::addRCompletions() {
	RK_TRACE (COMMANDEDITOR);

	QStringList addlist = rcompletions->results();
	if (addlist.isEmpty()) return;
	bool help_mode = (rcompletions->mode() == "?");
	beginInsertRows(index(0, 0), n_completions, n_completions + addlist.size());
	n_completions += addlist.size();
	for (int i = 0; i < addlist.size(); ++i) {
		if (help_mode) {
			names.append(addlist.at(i));
			shortnames.append(addlist.at(i));
			icons.append(RKStandardIcons::getIcon(RKStandardIcons::WindowHelp));
		} else {
			names.append(rcompletions->fragment() + rcompletions->mode() + addlist.at(i));
			shortnames.append(addlist.at(i));
			icons.append(RKStandardIcons::getIcon(RKStandardIcons::WindowConsole));
		}
	}
	endInsertRows();
	manager->modelGainedLateData(this);
}

KTextEditor::Range RKCodeCompletionModel::completionRange (KTextEditor::View *, const KTextEditor::Cursor&) {
	return manager->currentSymbolRange ();
}

QVariant RKCodeCompletionModel::data (const QModelIndex& index, int role) const {
	if (isHeaderItem (index)) {
		if (role == Qt::DisplayRole) return i18n ("Objects on search path");
		if (role == KTextEditor::CodeCompletionModel::GroupRole) return Qt::DisplayRole;
		if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) return 1;  // sort below arghint model
		return QVariant ();
	}

	int col = index.column ();
	int row = index.row ();

	if ((role == Qt::DisplayRole) || (role == KTextEditor::CodeCompletionModel::CompletionRole)) {
		if (col == KTextEditor::CodeCompletionModel::Name) {
			return (names.value (row));
		}
	} else if (role == Qt::DecorationRole) {
		if (col == KTextEditor::CodeCompletionModel::Icon) {
			return (icons.value (row));
		}
	} else if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) {
		return (row);  // disable sorting
	} else if (role == KTextEditor::CodeCompletionModel::MatchQuality) {
		return (10);
	}

	return QVariant ();
}

QStringList RKCodeCompletionModel::rawPartialCompletions() const {
	RK_TRACE (COMMANDEDITOR);

	// Here, we need to do completion on the *last* portion of the object-path, only, so we will be able to complete "obj" to "object", even if "object" is present as
	// both packageA::object and packageB::object.
	// Thus as a first step, we look up the short names. We do this, lazily, as this function is only called on demand.
	QStringList objectpath = RObject::parseObjectPath (current_symbol);
	if (objectpath.isEmpty() || objectpath[0].isEmpty()) return (QStringList());
	QString lead = objectpath.last();
	if (!shortnames.value(0).startsWith(lead)) lead.clear ();  // This could happen if the current path ends with '$', for instance

	return genPartialCompletions(shortnames, lead);
}


//////////////////////// RKCallHintModel //////////////////////////
RKCallHintModel::RKCallHintModel (RKCompletionManager* manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);
	function = nullptr;
}

// TODO: There could be more than one function by a certain name, and we could support this!
void RKCallHintModel::setFunction(RObject* _function) {
	RK_TRACE (COMMANDEDITOR);

	if (function == _function) return;
	function = _function;

	beginResetModel ();
	if (function && function->isType (RObject::Function)) {
		// initialize hint
		RFunctionObject *fo = static_cast<RFunctionObject*> (function);
		QStringList args = fo->argumentNames ();
		QStringList defs = fo->argumentDefaults ();

		formals = '(';
		formatting.clear ();
		QTextCharFormat format;
		format.setForeground(QBrush(Qt::green));

		int pos = 1;
		for (int i = 0; i < args.size (); ++i) {
			QString pair = args[i];
			if (!defs.value(i).isEmpty ()) pair.append ('=' + defs[i]);
			formatting.append ({ pos + args[i].length (), pair.length ()-args[i].length (), format });

			if (i < (args.size () - 1)) pair.append (", ");
			formals.append (pair);

			pos = pos + pair.length ();
		}
		formals.append (')');
		n_completions = 1;
	} else {
		n_completions = 0;
	}
	endResetModel ();
}

QVariant RKCallHintModel::data (const QModelIndex& index, int role) const {
	if (isHeaderItem (index)) {
		if (role == Qt::DisplayRole) return i18n ("Function calls");  // NOTE: Header is not currently used as of KF5 5.44.0
		if (role == KTextEditor::CodeCompletionModel::GroupRole) return Qt::DisplayRole;
		return QVariant ();
	}

	int col = index.column ();
	if (role == Qt::DisplayRole) {
		if (col == KTextEditor::CodeCompletionModel::Prefix) return (name);
		if (col == KTextEditor::CodeCompletionModel::Arguments) return (formals);
	} else if (role == KTextEditor::CodeCompletionModel::ArgumentHintDepth) {
		return 1;
	} else if (role == KTextEditor::CodeCompletionModel::CompletionRole) {
		return KTextEditor::CodeCompletionModel::Function;
	} else if (role == KTextEditor::CodeCompletionModel::HighlightingMethod) {
		if (col == KTextEditor::CodeCompletionModel::Arguments) return KTextEditor::CodeCompletionModel::CustomHighlighting;
	} else if (role == KTextEditor::CodeCompletionModel::CustomHighlight) {
		if (col == KTextEditor::CodeCompletionModel::Arguments) return formatting;
	} else if (role == KTextEditor::CodeCompletionModel::MatchQuality) {
		return (10);
	}

	return QVariant ();
}

KTextEditor::Range RKCallHintModel::completionRange (KTextEditor::View *, const KTextEditor::Cursor&) {
	return manager->currentCallRange ();
}

//////////////////////// RKArgumentHintModel //////////////////////
RKArgumentHintModel::RKArgumentHintModel (RKCompletionManager* manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);

	function = nullptr;
	rcompletions = new RKDynamicCompletionsAddition(this);
	connect(rcompletions, &RKDynamicCompletionsAddition::resultsComplete, this, &RKArgumentHintModel::addRCompletions);
}

void RKArgumentHintModel::updateCompletionList (RObject* _function, const QString &argument) {
	RK_TRACE (COMMANDEDITOR);

	bool changed = false;
	if (function != _function) {
		beginResetModel ();
		changed = true;
		function = _function;
		if (function && function->isType (RObject::Function)) {
			// initialize hint
			RFunctionObject *fo = static_cast<RFunctionObject*> (function);
			args = fo->argumentNames ();
			n_formals_args = args.size();
			defs = fo->argumentDefaults ();
			rcompletions->update("funargs", function->getShortName(), QString(), args);
		} else {
			args.clear ();
			defs.clear ();
			n_formals_args = 0;
		}
	}

	if (changed || (argument != fragment)) {
		if (!changed) {
			changed = true;
			beginResetModel ();
		}
		fragment = argument;
		matches.clear ();
		if (!fragment.isNull()) {  // meaning were not on an argument name for sure
			for (int i = 0; i < args.size (); ++i) {
				if (args[i].startsWith (fragment)) matches.append (i);
			}
		}
	}

	if (changed) {
		n_completions = matches.size ();
		endResetModel ();
	}
}


void RKArgumentHintModel::addRCompletions() {
	RK_TRACE (COMMANDEDITOR);

	QStringList addlist = rcompletions->results();
	args += addlist;
	for (int i = 0; i < addlist.size (); ++i) {
		if (addlist[i].startsWith(fragment)) matches.append(n_completions + i);
	}
	if (n_completions != matches.size()) {
		beginInsertRows(index(0, 0), n_completions, matches.size());
		n_completions = matches.size();
		endInsertRows();
		manager->modelGainedLateData(this);
	}
}

QVariant RKArgumentHintModel::data (const QModelIndex& index, int role) const {
	if (isHeaderItem (index)) {
		if (role == Qt::DisplayRole) return i18n ("Function arguments");
		if (role == KTextEditor::CodeCompletionModel::GroupRole) return Qt::DisplayRole;
		if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) return 0; // Sort above other models (except calltip)
		return QVariant ();
	}

	int col = index.column ();
	int row = index.row ();
	if (role == Qt::DisplayRole) {
		if (col == KTextEditor::CodeCompletionModel::Name) return (args.value (matches.value (row)));
		if (col == KTextEditor::CodeCompletionModel::Postfix) {
			QString def = defs.value (matches.value (row));
			if (!def.isEmpty ()) return (QString ('=' + def));
		}
	} else if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) {
		return row;  // disable sorting
	} else if (role == KTextEditor::CodeCompletionModel::CompletionRole) {
		return KTextEditor::CodeCompletionModel::Function;
	} else if (role == KTextEditor::CodeCompletionModel::MatchQuality) {
		if (matches.value(row) >= n_formals_args) return (0);  // Argument names returned from R completion. This could be _lot_ (S3), so lower priority
		return (20);
	}

	return QVariant ();
}

KTextEditor::Range RKArgumentHintModel::completionRange (KTextEditor::View*, const KTextEditor::Cursor&) {
	return manager->currentArgnameRange ();
}

QStringList RKArgumentHintModel::rawPartialCompletions() const {
	RK_TRACE (COMMANDEDITOR);

	return genPartialCompletions(args, fragment);
}

//////////////////////// RKFileCompletionModel ////////////////////
#include <KUrlCompletion>
RKFileCompletionModelWorker::RKFileCompletionModelWorker (const QString &_string) : QThread () {
	RK_TRACE (COMMANDEDITOR);
	string = _string;
	connect (this, &QThread::finished, this, &QObject::deleteLater);
}

void RKFileCompletionModelWorker::run () {
	RK_TRACE (COMMANDEDITOR);

	KUrlCompletion comp;
	comp.setDir (QUrl::fromLocalFile (QDir::currentPath ()));
	comp.makeCompletion (string);
	while (comp.isRunning()) {
		msleep(50);
	}
	QStringList files = comp.allMatches ();

	QStringList exes;
	if (!QDir (string).isAbsolute ()) {  // NOTE: KUrlCompletion does not handle this well, returns, e.g. "/home/me/firefox" when given "/home/me/f"; KF5 5.44.0
		comp.setMode (KUrlCompletion::ExeCompletion);
		comp.makeCompletion (string);
		exes = comp.allMatches ();
	}

	Q_EMIT completionsReady(string, exes, files);
}

RKFileCompletionModel::RKFileCompletionModel (RKCompletionManager* manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);
	worker = nullptr;
}

RKFileCompletionModel::~RKFileCompletionModel () {
	RK_TRACE (COMMANDEDITOR);
}

void RKFileCompletionModel::updateCompletionList(const QString& fragment) {
	RK_TRACE (COMMANDEDITOR);

	if (current_fragment == fragment) return;

	current_fragment = fragment;
	launchThread ();
}

void RKFileCompletionModel::launchThread () {
	RK_TRACE (COMMANDEDITOR);

	if (current_fragment.isEmpty ()) {
		completionsReady (QString (), QStringList (), QStringList ());
	} else if (!worker) {
		RK_DEBUG (COMMANDEDITOR, DL_DEBUG, "Launching filename completion thread for '%s'", qPrintable (current_fragment));
		worker = new RKFileCompletionModelWorker (current_fragment);
		connect (worker, &RKFileCompletionModelWorker::completionsReady, this, &RKFileCompletionModel::completionsReady);
		worker->start ();
	}
}

void RKFileCompletionModel::completionsReady (const QString& string, const QStringList& exes, const QStringList& files) {
	RK_TRACE (COMMANDEDITOR);

	RK_DEBUG (COMMANDEDITOR, DL_DEBUG, "Filename completion finished for '%s': %d matches", qPrintable (string), files.size () + exes.size ());
	worker = nullptr;
	if (current_fragment == string) {
		beginResetModel ();
		names = files + exes; // TODO: This could be prettier
		n_completions = names.size ();
		endResetModel ();
	} else {
		launchThread ();
	}
}

KTextEditor::Range RKFileCompletionModel::completionRange (KTextEditor::View *, const KTextEditor::Cursor &) {
	return manager->currentSymbolRange ();
}

QVariant RKFileCompletionModel::data (const QModelIndex& index, int role) const {
	if (isHeaderItem (index)) {
		if (role == Qt::DisplayRole) return i18n ("Local file names");
		if (role == KTextEditor::CodeCompletionModel::GroupRole) return Qt::DisplayRole;
		if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) return 1; // Sort below arghint model
		return QVariant ();
	}

	int col = index.column ();
	int row = index.row ();
	if ((role == Qt::DisplayRole) || (role == KTextEditor::CodeCompletionModel::CompletionRole)) {
		if (col == KTextEditor::CodeCompletionModel::Name) {
			return (names.value (row));
		}
	} else if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) {
		return (row);  // disable sorting
	} else if (role == KTextEditor::CodeCompletionModel::MatchQuality) {
		return (10);
	}
	return QVariant ();
}

QStringList RKFileCompletionModel::rawPartialCompletions() const {
	RK_TRACE (COMMANDEDITOR);

	return genPartialCompletions(names, current_fragment);
}



RKDynamicCompletionsAddition::RKDynamicCompletionsAddition(RKCompletionModelBase *parent) : QObject(parent) {
	RK_TRACE(COMMANDEDITOR);
	status = Ready;
}

RKDynamicCompletionsAddition::~RKDynamicCompletionsAddition() {
	RK_TRACE(COMMANDEDITOR);
}

void RKDynamicCompletionsAddition::update(const QString &mode, const QString &fragment, const QString &filterprefix, const QStringList &filterlist) {
	RK_TRACE(COMMANDEDITOR);

	if ((mode != current_mode) || (fragment != current_fragment)) {
		current_mode = mode;
		current_fragment = fragment;
		doUpdateFromR();
	}
	if (filterprefix != current_filterprefix || filterlist != current_filterlist) {
		current_filterprefix = filterprefix;
		current_filterlist = filterlist;
		if (status == Ready) filterResults(); // no update was triggered, above, need to update filter, manually
	}
	if (status == Ready) Q_EMIT resultsComplete();
}

void RKDynamicCompletionsAddition::doUpdateFromR() {
	RK_TRACE(COMMANDEDITOR);
	if (status != Ready) {
		status = PendingUpdate;
		return;
	}

	status = Updating;
	RCommand *command = new RCommand(QString("rkward:::.rk.completions(%1, \"%2\")").arg(RObject::rQuote(current_fragment), current_mode), RCommand::Sync | RCommand::PriorityCommand | RCommand::GetStringVector);
	command->whenFinished(this, [this](RCommand *command) {
		if (status == PendingUpdate) {
			QTimer::singleShot(0, this, &RKDynamicCompletionsAddition::doUpdateFromR);
			return;
		}
		if (command->getDataType() == RCommand::StringVector) {
			QStringList res = command->stringVector();
			current_raw_resultlist.clear();
			// we used to do the subsitution below in R, but that proved challenging, as fragments would often contain symbols with a special meaning in regexps (importantly "$").
			QString input;
			if (mode() == "?") input = mode();
			else if (mode() != "funargs") input = fragment() + mode();
			for (int i = 0; i < res.size(); ++i) {
				auto it = res[i];
				if (it.startsWith(input)) current_raw_resultlist.append(it.right(it.length() - input.length()));
				else RK_DEBUG(COMMANDEDITOR, DL_INFO, "got malformed completion %s", qPrintable(it));
			}
		} else {
			RK_ASSERT(false);
		}
		filterResults();
		status = Ready;
		Q_EMIT resultsComplete();
	});
	RInterface::issueCommand(command);
}

void RKDynamicCompletionsAddition::filterResults() {
	RK_TRACE(COMMANDEDITOR);

	QStringList res;
	for (int i = 0; i < current_raw_resultlist.size(); ++i) {
		const auto item = current_raw_resultlist.at(i);
		if (!item.startsWith(current_filterprefix)) continue;
		if (current_filterlist.contains(item)) continue;
		res.append(item);
	}
	filtered_results = res;
}
