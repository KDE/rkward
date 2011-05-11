/***************************************************************************
                          rcommandstack  -  description
                             -------------------
    begin                : Mon Sep 6 2004
    copyright            : (C) 2004, 2007, 2010, 2011 by Thomas Friedrichsmeier
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
#include "rcommandstack.h"

#include <klocale.h>

#include <QTimer>
#include <QTime>

#include "rinterface.h"

#include "../debug.h"

//static
RCommandStack *RCommandStack::regular_stack;

RCommandStack::RCommandStack (RCommand* parent_command) : RCommandChain () {
	RK_TRACE (RBACKEND);
	closed = true;
	current_chain = this;
	sub_stack = 0;

	RCommandStackModel::getModel ()->aboutToAdd (parent_command);
	RCommandStack::parent_command = parent_command;
	if (parent_command) {
		RCommandStack* parent_stack = stackForCommand (parent_command);
		RK_ASSERT (parent_stack);
		parent_stack->sub_stack = this;
	}
	RCommandStackModel::getModel ()->addComplete ();
}

RCommandStack::~RCommandStack () {
	RK_TRACE (RBACKEND);

	RCommandStackModel::getModel ()->aboutToPop (parent_command);
	if (parent_command) {
		RCommandStack* parent_stack = stackForCommand (parent_command);
		RK_ASSERT (parent_stack);
		parent_stack->sub_stack = 0;
	}
	RCommandStackModel::getModel ()->popComplete ();
}

void RCommandStack::issueCommand (RCommand *command, RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	if (!chain) chain = regular_stack;

	RCommandStackModel::getModel ()->aboutToAdd (chain);

	chain->commands.append (command);
	command->parent = chain;

	RCommandStackModel::getModel ()->addComplete ();
}

RCommandChain *RCommandStack::startChain (RCommandChain *parent) {
	RK_TRACE (RBACKEND);
	if (!parent) parent = regular_stack;

	RCommandChain *chain = new RCommandChain ();
	RCommandStackModel::getModel ()->aboutToAdd (parent);
	chain->closed = false;
	chain->parent = parent;
	parent->commands.append (chain);

	RCommandStackModel::getModel ()->addComplete ();

	return chain;
}

void RCommandStack::closeChain (RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	if (!chain) return;

	chain->closed = true;
	RCommandStackModel::getModel ()->itemChange (chain);
}

RCommand* RCommandStack::currentCommand () {
	RK_TRACE (RBACKEND);

	if (RK_Debug_CommandStep) {
		QTime t;
		t.start ();
		while (t.elapsed () < RK_Debug_CommandStep) {}
	}

	clearFinishedChains ();
	RCommandBase *coc = current_chain;
	while (coc->chainPointer ()) {
		current_chain = coc->chainPointer ();
		if (current_chain->commands.isEmpty ()) return 0;
		coc = current_chain->commands.first ();
	}
	return coc->commandPointer ();
}

void RCommandStack::addChainCommandsToList (QList<RCommand*> *list, const RCommandChain *chain) const {
	RK_TRACE (RBACKEND);

	foreach (RCommandBase* coc, chain->commands) {
		if (coc->chainPointer ()) addChainCommandsToList (list, coc->chainPointer ());
		else if (coc->commandPointer ()) list->append (coc->commandPointer ());
		else RK_ASSERT (false);
	}
}

QList<RCommand*> RCommandStack::allCommands () const {
	RK_TRACE (RBACKEND);

	QList<RCommand*> ret;
	addChainCommandsToList (&ret, current_chain);
	return ret;
}

bool RCommandStack::isEmpty () {
//	RK_TRACE (RBACKEND);
	return (commands.isEmpty ());
}

bool RCommandStack::isActive () {
//	RK_TRACE (RBACKEND);

	bool ret = !current_chain->commands.isEmpty ();
	if (!ret) {
		clearFinishedChains ();
	}
	return ret;
}

//static
RCommandStack *RCommandStack::chainStack (RCommandChain *child) {
	RK_TRACE (RBACKEND);
	while (child->parent) {
		child = child->parent;
	}
	return static_cast<RCommandStack *> (child);
}

//static
RCommandStack *RCommandStack::stackForCommand (RCommand *child) {
	RK_TRACE (RBACKEND);

	RCommandBase *chain = child;
	while (chain->parent) {
		chain = chain->parent;
	}
	return static_cast<RCommandStack *> (chain);
}

RCommandStack* RCommandStack::currentStack () {
	RK_TRACE (RBACKEND);

	RCommandStack* stack = regular_stack;
	while (stack->sub_stack) {
		stack = stack->sub_stack;
	}
	return stack;
}

void RCommandStack::pop () {
	RK_TRACE (RBACKEND);

	if (current_chain->commands.isEmpty ()) return;
	RCommandBase* popped = current_chain->commands.first ();
	RK_ASSERT (popped->commandPointer ());
	RCommandStackModel::getModel ()->aboutToPop (popped->parent);
	current_chain->commands.removeFirst ();
	RCommandStackModel::getModel ()->popComplete ();
}

void RCommandStack::clearFinishedChains () {
	// reached end of chain and chain is closed? walk up
	while (current_chain->commands.isEmpty () && current_chain->closed && current_chain->parent) {
		RK_TRACE (RBACKEND);	// trace in here. This is actually reach much *less* often

		RCommandChain *prev_chain = current_chain;
		RCommandStackModel::getModel ()->aboutToPop (current_chain->parent);
		current_chain->parent->commands.removeFirst ();
		current_chain = current_chain->parent;
		delete prev_chain;
		RCommandStackModel::getModel ()->popComplete ();
	}
}

/////////////////////// RCommandStackModel ////////////////////

#define MAIN_COL 0
#define STATUS_COL 1
#define FLAGS_COL 2
#define DESC_COL 3
#define NUM_COLS 4

// static
RCommandStackModel* RCommandStackModel::static_model = 0;

RCommandStackModel::RCommandStackModel (QObject *parent) : QAbstractItemModel (parent) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (static_model == 0);	// only one instance should be created

	static_model = this;
	listeners = 0;
}

RCommandStackModel::~RCommandStackModel () {
	RK_TRACE (RBACKEND);

	static_model = 0;
	RK_ASSERT (!listeners);
}

void RCommandStackModel::addListener () {
	RK_TRACE (RBACKEND);

	++listeners;
}

void RCommandStackModel::removeListener () {
	RK_TRACE (RBACKEND);

	--listeners;
	RK_ASSERT (listeners >= 0);
}

QModelIndex RCommandStackModel::index (int row, int column, const QModelIndex& parent) const {
	RK_ASSERT (listeners > 0);
	RK_TRACE (RBACKEND);

	RCommandBase* index_data = 0;

	if (!parent.isValid ()) {
		index_data = RCommandStack::regular_stack;
	} else {
		RCommandBase* parent_index = static_cast<RCommandBase*> (parent.internalPointer ());
		RK_ASSERT (parent_index);

		// parent is a command -> this must be a substack
		if (parent_index->commandPointer ()) {
			RK_ASSERT (parent.row () == 0);
			index_data = RCommandStack::stackForCommand (parent_index->commandPointer ())->sub_stack;
			RK_ASSERT (index_data);
		} else {
			// parent is a chain or stack
			RCommandChain *chain = parent_index->chainPointer ();
			if (chain->commands.size () <= row) {
				RK_ASSERT (false);
				return QModelIndex ();
			}
			index_data = chain->commands[row];
		}
	}

	return (createIndex (row, column, index_data));
}

QModelIndex RCommandStackModel::parent (const QModelIndex& child) const {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);

	RCommandBase* index_data;
	if (!child.isValid()) {
		return QModelIndex ();
	} else {
		RCommandBase* child_index = static_cast<RCommandBase*> (child.internalPointer ());
		RK_ASSERT (child_index);

		if (child_index->chainPointer () && child_index->chainPointer ()->isStack ()) {
			index_data = static_cast<RCommandStack*> (child_index->chainPointer ())->parent_command;
			if (!index_data) {
				return QModelIndex ();
			}
		} else {	// regular chains or commands
			index_data = child_index->parent;
		}
	}

	return (createIndex (0, 0, index_data));
}

int RCommandStackModel::rowCount (const QModelIndex& parent) const {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);

	if (!parent.isValid ()) return 1;

	RCommandBase* index_data = static_cast<RCommandBase*> (parent.internalPointer ());
	RK_ASSERT (index_data);
	if (index_data->commandPointer ()) {
		RCommandStack *substack = RCommandStack::stackForCommand (index_data->commandPointer ())->sub_stack;
		if (substack) {
			if (substack->parent_command == index_data) {
				RK_ASSERT (parent.row () == 0);
				return 1;
			}
		}
		return 0;
	}
	if (index_data->chainPointer ()) {
		return (index_data->chainPointer ()->commands.size ());
	}
	RK_ASSERT (false);
	return 0;
}

int RCommandStackModel::columnCount (const QModelIndex&) const {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);

	return NUM_COLS;
}

QVariant RCommandStackModel::data (const QModelIndex& index, int role) const {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);

	if (!index.isValid ()) return QVariant ();
	RK_ASSERT (index.model () == this);

	RCommandBase* index_data = static_cast<RCommandBase*> (index.internalPointer ());

	if (index_data->commandPointer ()) {
		RCommand *command = index_data->commandPointer ();
		if ((index.column () == MAIN_COL) && (role == Qt::DisplayRole)) return (command->command ());
		if ((index.column () == FLAGS_COL) && (role == Qt::DisplayRole)) {
			QString ret;
			if (command->type () & RCommand::User) ret += 'U';
			if (command->type () & RCommand::Plugin) ret += 'P';
			if (command->type () & RCommand::App) ret += 'A';
			if (command->type () & RCommand::Sync) ret += 'S';
			if (command->type () & RCommand::EmptyCommand) ret += 'E';
			if (command->type () & (RCommand::GetIntVector | RCommand::GetRealVector | RCommand::GetStringVector | RCommand::GetStructuredData)) ret += 'D';
			if (command->type () & RCommand::CCOutput) ret += 'O';
			return (ret);
		}
		if ((index.column () == STATUS_COL) && (role == Qt::DisplayRole)) {
			QString ret;
			if (command->status & RCommand::Running) ret += i18n ("Running");
			if (command->status & RCommand::Canceled) {
				if (!ret.isEmpty ()) ret += ", ";
				ret += i18n ("Cancelled");
			}
			return (ret);
		}
		if ((index.column () == DESC_COL) && (role == Qt::DisplayRole)) {
			return (command->rkEquivalent ());
		}
	}
	if (index_data->chainPointer ()) {
		RCommandChain* chain = index_data->chainPointer ();
		if (chain->isStack ()) {
			if ((index.column () == MAIN_COL) && (role == Qt::DisplayRole)) return (i18n ("Command Stack"));
		} else {
			if ((index.column () == MAIN_COL) && (role == Qt::DisplayRole)) return (i18n ("Command Chain"));
			if ((index.column () == STATUS_COL) && (role == Qt::DisplayRole)) {
				if (chain->closed) return (i18n ("Closed"));
				return (i18n ("Waiting"));
			}
		}
	}

	return (QVariant ());
}

Qt::ItemFlags RCommandStackModel::flags (const QModelIndex& index) const {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);

	if (!index.isValid ()) return 0;
	RK_ASSERT (index.model () == this);

	RCommandBase* index_data = static_cast<RCommandBase*> (index.internalPointer ());
	RK_ASSERT (index_data);

	if (index_data->commandPointer ()) return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	return Qt::ItemIsEnabled;
}

QVariant RCommandStackModel::headerData (int section, Qt::Orientation orientation, int role) const {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);

	if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
		if (section == MAIN_COL) return (i18n ("Command"));
		if (section == FLAGS_COL) return (i18n ("Type"));
		if (section == STATUS_COL) return (i18n ("Status"));
		if (section == DESC_COL) return (i18n ("Description"));
	}

	return QVariant ();
}

QModelIndex RCommandStackModel::indexFor (RCommandBase *item) {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);

	if (!item) return (QModelIndex ());

	if (item->chainPointer () && item->chainPointer ()->isStack ()) {
		// stacks are always the first (and only) child of their parent
		return (createIndex (0, 0, item));
	}

	RK_ASSERT (item->parent);
	int row = item->parent->commands.indexOf (item);
	if (row < 0) {
		RK_ASSERT (false);
		return (QModelIndex ());
	}
	return (createIndex (row, 0, item));
}

void RCommandStackModel::aboutToPop (RCommandBase* parent) {
	if (!listeners) return;
	RK_TRACE (RBACKEND);

	QModelIndex parent_index = indexFor (parent);
	// items are always removed at the front
	beginRemoveRows (parent_index, 0, 0);
}

void RCommandStackModel::popComplete () {
	if (!listeners) return;
	RK_TRACE (RBACKEND);

	endRemoveRows ();
}

void RCommandStackModel::aboutToAdd (RCommandBase* parent) {
	if (!listeners) return;
	RK_TRACE (RBACKEND);

	QModelIndex parent_index = indexFor (parent);
	if ((!parent) || parent->commandPointer ()) {
		beginInsertRows (parent_index, 0, 0);
	} else {
		// items are always added at the end
		int row = parent->chainPointer ()->commands.size ();
		beginInsertRows (parent_index, row, row);
	}
}

void RCommandStackModel::addComplete () {
	if (!listeners) return;
	RK_TRACE (RBACKEND);

	endInsertRows ();
}

void RCommandStackModel::itemChange (RCommandBase* item) {
	if (!listeners) return;
	RK_TRACE (RBACKEND);

	QModelIndex item_index = indexFor (item);
	emit (dataChanged (item_index, item_index));
}

#include "rcommandstack.moc"
