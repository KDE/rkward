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
#include "rkcodecompletion.h"

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <KLocalizedString>
#include <KActionCollection>

#include <QApplication>
#include <QKeyEvent>
#include <QIcon>
#include <QDir>

#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardicons.h"
#include "../core/rfunctionobject.h"
#include "../core/robjectlist.h"
#include "../settings/rksettingsmodulecommandeditor.h"

#include "../debug.h"


//////////////////////// RKCompletionManager //////////////////////

RKCompletionManager::RKCompletionManager (KTextEditor::View* view) : QObject (view) {
	RK_TRACE (COMMANDEDITOR);

	_view = view;
	active = false;
	user_triggered = false;
	ignore_next_trigger = false;
	update_call = true;
	cached_position = KTextEditor::Cursor (-1, -1);

	cc_iface = qobject_cast<KTextEditor::CodeCompletionInterface*> (view);
	if (cc_iface) {
		cc_iface->setAutomaticInvocationEnabled (false);
		completion_model = new RKCodeCompletionModel (this);
		file_completion_model = new RKFileCompletionModel (this);
		callhint_model = new RKCallHintModel (this);
		arghint_model = new RKArgumentHintModel (this);
		cc_iface->registerCompletionModel (completion_model);  // (at least) one model needs to be registerd, so we will know, when completion was triggered by the user (shortcut)
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

	} else {
		RK_ASSERT (false);  // Not a katepart?
	}
}

RKCompletionManager::~RKCompletionManager () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCompletionManager::tryCompletionProxy () {
	if (active) {
		// Handle this in the next event cycle, as more than one event may trigger
		completion_timer->start (0);
	} else if (RKSettingsModuleCommandEditor::autoCompletionEnabled ()) {
		completion_timer->start (RKSettingsModuleCommandEditor::autoCompletionTimeout ());
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
	// NOTE: Sometimes (non-determinate) the completion window will disappear directly after this. It does not seem to be our fault, here, according to the tests I did.
}

void RKCompletionManager::tryCompletion () {
	// TODO: merge this with RKConsole::doTabCompletion () somehow
	RK_TRACE (COMMANDEDITOR);
	if (!cc_iface) {
		// NOTE: This should not be possible, because the connections  have not been set up in the constructor, in this case.
		RK_ASSERT (cc_iface);
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
	if (!user_triggered) {
		if (end > cursor_pos) {
			symbol_range = KTextEditor::Range ();   // Only hint when at the end of a word/symbol: https://mail.kde.org/pipermail/rkward-devel/2015-April/004122.html
		} else {
			if (doc->defaultStyleAt (c) == KTextEditor::dsComment) symbol_range = KTextEditor::Range ();	// do not hint while in comments
		}
	}

	QString word = currentCompletionWord ();
	if (user_triggered || (word.length () >= RKSettingsModuleCommandEditor::autoCompletionMinChars ())) {
		QString filename;
		// as a very simple heuristic: If the current symbol starts with a quote, we should probably attempt file name completion, instead of symbol name completion
		if (word.startsWith ('\"') || word.startsWith ('\'') || word.startsWith ('`')) {
			symbol_range.setStart (KTextEditor::Cursor (symbol_range.start ().line (), symbol_range.start ().column () + 1));   // confine range to inside the quotes.
			filename = word.mid (1);
			word.clear ();
		}

		completion_model->updateCompletionList (word);
		file_completion_model->updateCompletionList (filename);
	} else {
		completion_model->updateCompletionList (QString ());
		file_completion_model->updateCompletionList (QString ());
	}

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

void RKCompletionManager::updateCallHint () {
	RK_TRACE (COMMANDEDITOR);

	if (!update_call) return;
	update_call = false;

	int line = cached_position.line () + 1;
	QString full_context;
	int potential_symbol_end = -2;
	KTextEditor::Document *doc = _view->document ();
	while (potential_symbol_end < -1 && line >= 0) {
		--line;
		QString context_line = doc->line (line);
		if (context_line.startsWith ('>')) continue; // For console: TODO limit to console
		full_context.prepend (context_line);

		int pos = context_line.length () - 1;
		if (line == cached_position.line ()) pos = cached_position.column () - 1;
		for (int i = pos; i >= 0; --i) {
			QChar c = context_line.at (i);
			if (c == '(') {
				KTextEditor::DefaultStyle style = doc->defaultStyleAt (KTextEditor::Cursor (line, i));
				if (style != KTextEditor::dsComment && style != KTextEditor::dsString && style != KTextEditor::dsChar) {
					potential_symbol_end = i - 1;
					break;
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
	RObject *object = 0;
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

void startModel (KTextEditor::CodeCompletionInterface* iface, KTextEditor::CodeCompletionModel *model, bool start, const KTextEditor::Range &range, QList<KTextEditor::CodeCompletionModel*> *active_models) {
	if (start) {
		if (model->rowCount () == 0) start = false;
		if (!range.isValid ()) start = false;
	}
	if (start) {
		if (!active_models->contains (model)) {
			iface->startCompletion (range, model);
			active_models->append (model);
		}
	} else {
		active_models->removeAll (model);
	}
}

void RKCompletionManager::updateVisibility () {
	RK_TRACE (COMMANDEDITOR);

	if (user_triggered || !cc_iface->isCompletionActive ()) {
		active_models.clear ();
	}

	bool min_len = (currentCompletionWord ().length () >= RKSettingsModuleCommandEditor::autoCompletionMinChars ()) || user_triggered;
	startModel (cc_iface, completion_model, min_len && RKSettingsModuleCommandEditor::isCompletionEnabled (RKSettingsModuleCommandEditor::Object), symbol_range, &active_models);
	startModel (cc_iface, file_completion_model, min_len && RKSettingsModuleCommandEditor::isCompletionEnabled (RKSettingsModuleCommandEditor::Filename), symbol_range, &active_models);
	if (kate_keyword_completion_model && RKSettingsModuleCommandEditor::isCompletionEnabled (RKSettingsModuleCommandEditor::AutoWord)) {
		// Model needs to update, first, as we have not handled it in tryCompletion:
		if (min_len) kate_keyword_completion_model->completionInvoked (view (), symbol_range, KTextEditor::CodeCompletionModel::ManualInvocation);
		startModel (cc_iface, kate_keyword_completion_model, min_len, symbol_range, &active_models);
	}
// NOTE: Freaky bug in KF 5.44.0: Call hint will not show for the first time, if logically above the primary screen. TODO: provide patch for kateargumenthinttree.cpp:166pp
	startModel (cc_iface, callhint_model, true && RKSettingsModuleCommandEditor::isCompletionEnabled (RKSettingsModuleCommandEditor::Calltip), currentCallRange (), &active_models);
	startModel (cc_iface, arghint_model, min_len && RKSettingsModuleCommandEditor::isCompletionEnabled (RKSettingsModuleCommandEditor::Arghint), argname_range, &active_models);

	if (!active_models.isEmpty ()) {
		active = true;
	} else {
		cc_iface->abortCompletion ();
		active = false;
	}

	if (!active) update_call = true;
}

void RKCompletionManager::textInserted (KTextEditor::Document*, const KTextEditor::Cursor& position, const QString& text) {
	if (active) {
		if (position < call_opening) update_call = true;
		else if (text.contains (QChar ('(')) || text.contains (QChar (')'))) update_call = true;
	}
	tryCompletionProxy();
}

void RKCompletionManager::textRemoved (KTextEditor::Document*, const KTextEditor::Range& range, const QString& text) {
	if (active) {
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
	if (active) {
		if (newPosition < call_opening) update_call = true;
		else {
			QString text = view->document ()->text (KTextEditor::Range (newPosition, cached_position));
			if (text.contains (QChar ('(')) || text.contains (QChar (')'))) update_call = true;
		}
	}
	tryCompletionProxy ();
}

KTextEditor::Range RKCompletionManager::currentCallRange () const {
	return KTextEditor::Range (call_opening, _view->cursorPosition ());
}

bool RKCompletionManager::eventFilter (QObject* watched, QEvent* event) {
	if (event->type () == QEvent::KeyPress || event->type () == QEvent::ShortcutOverride) {
		RK_TRACE (COMMANDEDITOR);	// avoid loads of empty traces, putting this here
		QKeyEvent *k = static_cast<QKeyEvent *> (event);

		if (k->key () == Qt::Key_Tab && (!k->modifiers ())) {
			RK_DEBUG(COMMANDEDITOR, DL_ERROR, "%d", k->type ());
			// If only the calltip is active, make sure the tab-key behaves as a regular key. There is no completion in this case.
			if (active_models.count () == 1 && active_models[0] == callhint_model) {
				cc_iface->abortCompletion (); // That's a bit lame, but the least hacky way to get the key into the document. Note that we keep active==true, so
				                              // the completion window should come back up, without delay
				return false;
			}

			// Otherwise, try to do partial completion.
			// TODO: It is not quite clear, what behavior is desirable, in case more than one completion model is active at a time.
			//       For now, we use the simplest solution (implemenation-wise), and complete from the topmost-model, only
			// TODO: Handle the ktexteditor builtin models, too.
			bool exact = false;
			QString comp;
			bool handled = false;
			if (active_models.contains (arghint_model)) {
				comp = arghint_model->partialCompletion (&exact);
				handled = true;
			} else if (active_models.contains (completion_model)) {
				comp = completion_model->partialCompletion (&exact);
				handled = true;
			} else if (active_models.contains (file_completion_model)) {
				comp = file_completion_model->partialCompletion (&exact);
				handled = true;
			}

			if (handled) {
				RK_DEBUG(COMMANDEDITOR, DL_DEBUG, "Tab completion: %s", qPrintable (comp));
				if (k->type () == QEvent::ShortcutOverride) {
					// Too bad for all the duplicate work, but the event will re-trigger as a keypress event, and we need to intercept that one, too.
					return true;
				}
				view ()->document ()->insertText (view ()->cursorPosition (), comp);
				if (exact) {
					// Ouch, how messy. We want to make sure completion stops, and is not re-triggered by the insertion, itself
					active_models.clear ();
					active = false;
					cc_iface->abortCompletion ();
					if (RKSettingsModuleCommandEditor::autoCompletionEnabled ()) ignore_next_trigger = true;
				}
				else if (comp.isEmpty ()) {
					QApplication::beep (); // TODO: unfortunately, we catch *two* tab events, so this is not good, yet
				}
				return true;
			}
		} else if ((k->key () == Qt::Key_Up || k->key () == Qt::Key_Down) && cc_iface->isCompletionActive ()) {
			if (RKSettingsModuleCommandEditor::cursorNavigatesCompletions ()) return false;

			// Make up / down-keys (without alt) navigate in the document (aborting the completion)
			// Meke alt+up / alt+down naviate in the completion list
			if (k->modifiers () & Qt::AltModifier) {
				if (k->type() != QKeyEvent::KeyPress) return true;  // eat the release event

				// No, we cannot just send a fake key event, easily...
				KActionCollection *kate_edit_actions = view ()->findChild<KActionCollection*> ("edit_actions");
				QAction *action = kate_edit_actions ? (kate_edit_actions->action (k->key () == Qt::Key_Up ? "move_line_up" : "move_line_down")) : 0;
				if (!action) {
					kate_edit_actions = view ()->actionCollection ();
					action = kate_edit_actions ? (kate_edit_actions->action (k->key () == Qt::Key_Up ? "move_line_up" : "move_line_down")) : 0;
				}
				if (action) action->trigger ();
				else RK_ASSERT (action);
				return true;
			} else {
				cc_iface->abortCompletion ();
				active = false;
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

//////////////////////// RKCodeCompletionModel ////////////////////

RKCodeCompletionModel::RKCodeCompletionModel (RKCompletionManager *manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);

	setHasGroups (true);
}

RKCodeCompletionModel::~RKCodeCompletionModel () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCodeCompletionModel::updateCompletionList (const QString& symbol) {
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
	n_completions = matches.size ();
	icons.clear ();
	icons.reserve (n_completions);
	names = RObject::getFullNames (matches, RKSettingsModuleCommandEditor::completionOptions());
	for (int i = 0; i < n_completions; ++i) {
		icons.append (RKStandardIcons::iconForObject (matches[i]));
	}

	current_symbol = symbol;

	endResetModel ();
}

KTextEditor::Range RKCodeCompletionModel::completionRange (KTextEditor::View *, const KTextEditor::Cursor&c) {
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

QString findCommonCompletion (const QStringList &list, const QString &lead, bool *exact_match) {
	RK_TRACE (COMMANDEDITOR);
	RK_DEBUG(COMMANDEDITOR, DL_DEBUG, "Looking for commong completion among set of %d, starting with %s", list.size (), qPrintable (lead));

	*exact_match = true;
	QString ret;
	bool first = true;
	int lead_size = lead.count ();
	for (int i = list.size () - 1; i >= 0; --i) {
		if (!list[i].startsWith (lead)) continue;

		QString candidate = list[i].mid (lead_size);
		if (first) {
			ret = candidate;
			first = false;
		} else {
			if (ret.length () > candidate.length ()) {
				ret = ret.left (candidate.length ());
				*exact_match = false;
			}

			for (int c = 0; c < ret.length(); ++c) {
				if (ret[c] != candidate[c]) {
					*exact_match = false;
					if (!c) return QString ();

					ret = ret.left (c);
					break;
				}
			}
		}
	}

	return ret;
}

QString RKCodeCompletionModel::partialCompletion (bool* exact_match) {
	RK_TRACE (COMMANDEDITOR);

	// Here, we need to do completion on the *last* portion of the object-path, only, so we will be able to complete "obj" to "object", even if "object" is present as
	// both packageA::object and packageB::object.
	// Thus as a first step, we look up the short names. We do this, lazily, as this function is only called on demand.
	QStringList objectpath = RObject::parseObjectPath (current_symbol);
	if (objectpath.isEmpty () || objectpath[0].isEmpty ()) return (QString ());
	RObject::ObjectList matches = RObjectList::getObjectList ()->findObjectsMatching (current_symbol);

	QStringList shortnames;
	for (int i = 0; i < matches.count (); ++i) {
		shortnames.append (matches[i]->getShortName ());
	}
	QString lead = objectpath.last ();
	if (!shortnames.value (0).startsWith (lead)) lead.clear ();  // This could happen if the current path ends with '$', for instance

	return (findCommonCompletion (shortnames, lead, exact_match));
}

void RKCodeCompletionModel::completionInvoked (KTextEditor::View* view, const KTextEditor::Range& range, KTextEditor::CodeCompletionModel::InvocationType invocationType) {
	RK_TRACE (COMMANDEDITOR);

	if (invocationType == KTextEditor::CodeCompletionModel::UserInvocation) {
		manager->userTriggeredCompletion ();
	}
}


//////////////////////// RKCallHintModel //////////////////////////
RKCallHintModel::RKCallHintModel (RKCompletionManager* manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);
	function = 0;
}

#include <QDesktopWidget>
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

		name = function->getFullName ();

		formals = '(';
		formatting.clear ();
		KTextEditor::Attribute format;
//		format.setFontBold (); // NOTE: Not good. makes size mis-calculation worse.
		format.setForeground (QBrush (Qt::green));  // But turns out purple?!

		// NOTE: Unfortunately, adding new-lines within (long) formals does not work. If this issue turns out to be relevant, we'll have to resort to breaking the formals into
		// several (dummy) items.
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
		if (col == KTextEditor::CodeCompletionModel::Postfix) return ("        "); // Size is of a bit for KF5 5.44.0. Provide some padding to work around cut-off parts.
	} else if (role == KTextEditor::CodeCompletionModel::ArgumentHintDepth) {
		return 1;
	} else if (role == KTextEditor::CodeCompletionModel::CompletionRole) {
		return KTextEditor::CodeCompletionModel::Function;
	} else if (role == KTextEditor::CodeCompletionModel::HighlightingMethod) {
		if (col == KTextEditor::CodeCompletionModel::Arguments) return KTextEditor::CodeCompletionModel::CustomHighlighting;
	} else if (role == KTextEditor::CodeCompletionModel::CustomHighlight) {
		if (col == KTextEditor::CodeCompletionModel::Arguments)  return formatting;
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

	function = 0;
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
			defs = fo->argumentDefaults ();
		} else {
			args.clear ();
			defs.clear ();
		}
	}

	if (changed || (argument != fragment)) {
		if (!changed) {
			changed = true;
			beginResetModel ();
		}
		fragment = argument;
		matches.clear ();
		for (int i = 0; i < args.size (); ++i) {
			if (args[i].startsWith (fragment)) matches.append (i);
		}
	}

	if (changed) {
		n_completions = matches.size ();
		endResetModel ();
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
		return (20);
	}

	return QVariant ();
}

KTextEditor::Range RKArgumentHintModel::completionRange (KTextEditor::View*, const KTextEditor::Cursor&) {
	return manager->currentArgnameRange ();
}

QString RKArgumentHintModel::partialCompletion (bool* exact) {
	RK_TRACE (COMMANDEDITOR);

	return (findCommonCompletion (args, fragment, exact));
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
	QStringList files = comp.allMatches ();

	QStringList exes;
	if (!QDir (string).isAbsolute ()) {  // NOTE: KUrlCompletion does not handle this well, returns, e.g. "/home/me/firefox" when given "/home/me/f"; KF5 5.44.0
		comp.setMode (KUrlCompletion::ExeCompletion);
		comp.makeCompletion (string);
		exes = comp.allMatches ();
	}

	emit (completionsReady (string, exes, files));
}

RKFileCompletionModel::RKFileCompletionModel (RKCompletionManager* manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);
	worker = 0;
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
	worker = 0;
	if (current_fragment == string) {
		beginResetModel ();
		names = files + exes; // TODO: This could be prettier
		n_completions = names.size ();
		endResetModel ();
	} else {
		launchThread ();
	}
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

QString RKFileCompletionModel::partialCompletion (bool* exact) {
	RK_TRACE (COMMANDEDITOR);

	return (findCommonCompletion (names, current_fragment, exact));
}
