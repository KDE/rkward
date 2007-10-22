/***************************************************************************
                          rcommandstack  -  description
                             -------------------
    begin                : Mon Sep 6 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
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

#include "rinterface.h"

#include "../debug.h"

//static
RCommandStack *RCommandStack::regular_stack;

RCommandStack::RCommandStack (RCommand* parent_command) : RCommandChain () {
	RK_TRACE (RBACKEND);
	closed = true;
	current_chain = this;

	RCommandStack::parent_command = parent_command;
	if (parent_command) {
		RCommandStack* parent_stack = stackForCommand (parent_command);
		RK_ASSERT (parent_stack);
		parent_stack->sub_stack = this;
	}
}

RCommandStack::~RCommandStack () {
	RK_TRACE (RBACKEND);

	if (parent_command) {
		RCommandStack* parent_stack = stackForCommand (parent_command);
		RK_ASSERT (parent_stack);
		parent_stack->sub_stack = 0;
	}
}

void RCommandStack::issueCommand (RCommand *command, RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	if (!chain) chain = regular_stack;

	RCommandStackModel::getModel ()->aboutToAdd (command);

	chain->commands.append (command);
	command->parent = chain;

	RCommandStackModel::getModel ()->addComplete (command);
}

RCommandChain *RCommandStack::startChain (RCommandChain *parent) {
	RK_TRACE (RBACKEND);
	if (!parent) parent = regular_stack;

	RCommandChain *chain = new RCommandChain ();
	RCommandStackModel::getModel ()->aboutToAdd (chain);

	chain->closed = false;
	chain->parent = parent;
	parent->commands.append (chain);

	RCommandStackModel::getModel ()->addComplete (chain);

	return chain;
}

void RCommandStack::closeChain (RCommandChain *chain) {
	RK_TRACE (RBACKEND);
	if (!chain) return;

	chain->closed = true;
}

RCommand* RCommandStack::currentCommand () {
	RK_TRACE (RBACKEND);

	clearFinishedChains ();
	RCommandBase *coc = current_chain;
	while (coc->chainPointer ()) {
		current_chain = coc->chainPointer ();
		if (current_chain->commands.isEmpty ()) return 0;
		coc = current_chain->commands.first ();
	}
	return coc->commandPointer ();
}

bool RCommandStack::isEmpty () {
//	RK_TRACE (RBACKEND);
	return (current_chain->commands.isEmpty () && current_chain->closed);
}

bool RCommandStack::isBlocked () {
//	RK_TRACE (RBACKEND);
	return ((!current_chain->closed) && (!current_chain->commands.isEmpty ()));
}

bool RCommandStack::isActive () {
//	RK_TRACE (RBACKEND);
	return (!current_chain->commands.isEmpty ());
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

	if (!isActive ()) return;
	RCommandBase* popped = current_chain->commands.first ();
	RK_ASSERT (popped->commandPointer ());
	RCommandStackModel::getModel ()->aboutToPop (popped->commandPointer ());
	current_chain->commands.removeFirst ();
	RCommandStackModel::getModel ()->popComplete (popped->commandPointer ());
}

void RCommandStack::clearFinishedChains () {
	RK_TRACE (RBACKEND);

	// reached end of chain and chain is closed? walk up
	while (current_chain->commands.isEmpty () && current_chain->closed && current_chain->parent) {
		RCommandChain *prev_chain = current_chain;
		RCommandStackModel::getModel ()->aboutToPop (prev_chain);
		current_chain->parent->commands.removeFirst ();
		current_chain = current_chain->parent;
		delete prev_chain;
		RCommandStackModel::getModel ()->popComplete (prev_chain);
	}
}

/////////////////////// RCommandStackModel ////////////////////

// static
RCommandStackModel* RCommandStackModel::static_model = 0;

RCommandStackModel::RCommandStackModel (QObject *parent) : QAbstractItemModel (parent) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (static_model == 0);	// only one instance should be created

	static_model = this;
	listeners = 0;
	have_mutex_lock = false;

	connect (this, SIGNAL (change()), this, SLOT (relayChange()), Qt::BlockingQueuedConnection);
	connect (this, SIGNAL (aboutToChange()), this, SLOT (relayAboutToChange()), Qt::BlockingQueuedConnection);
}

RCommandStackModel::~RCommandStackModel () {
	RK_TRACE (RBACKEND);

	static_model = 0;
	RK_ASSERT (!listeners);
}

void RCommandStackModel::removeListener () {
	RK_TRACE (RBACKEND);
qDebug ("remove");
	--listeners;
	if (!listeners) unlockMutex ();
}

QModelIndex RCommandStackModel::index (int row, int column, const QModelIndex& parent) const {
	RK_ASSERT (listeners > 0);
	RK_TRACE (RBACKEND);
	lockMutex ();

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
			RK_ASSERT (chain->commands.size () > row);
			index_data = chain->commands[row];
		}
	}

	return (createIndex (row, column, index_data));
}

QModelIndex RCommandStackModel::parent (const QModelIndex& child) const {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);
	lockMutex ();

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
	lockMutex ();

	if (!parent.isValid ()) return 1;

	RCommandBase* index_data = static_cast<RCommandBase*> (parent.internalPointer ());
	RK_ASSERT (index_data);
	if (index_data->commandPointer ()) {
		if (RCommandStack::stackForCommand (index_data->commandPointer ())->sub_stack) return 1;
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
	lockMutex ();

	return 2;
}

QVariant RCommandStackModel::data (const QModelIndex& index, int role) const {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);
	lockMutex ();

	if (!index.isValid ()) return QVariant ();
	RK_ASSERT (index.model () == this);

	RCommandBase* index_data = static_cast<RCommandBase*> (index.internalPointer ());

	if (index_data->commandPointer ()) {
		if ((index.column () == 0) && (role == Qt::DisplayRole)) return (index_data->commandPointer ()->command ());
		if ((index.column () == 1) && (role == Qt::DisplayRole)) return ("Some flags");
	}
	if (index_data->chainPointer ()) {
		RCommandChain* chain = index_data->chainPointer ();
		if (chain->isStack ()) {
			if ((index.column () == 0) && (role == Qt::DisplayRole)) return (i18n ("Command Stack"));
			if ((index.column () == 1) && (role == Qt::DisplayRole)) return ("Some flags");
		} else {
			if ((index.column () == 0) && (role == Qt::DisplayRole)) return (i18n ("Command Group"));
			if ((index.column () == 1) && (role == Qt::DisplayRole)) return ("Some flags");
		}
	}

	return (QVariant ());
}

QVariant RCommandStackModel::headerData (int section, Qt::Orientation orientation, int role) const {
	RK_ASSERT (listeners);
	RK_TRACE (RBACKEND);
	lockMutex ();

	if ((orientation == Qt::Horizontal) && (role = Qt::DisplayRole)) {
		if (section == 0) return (i18n ("Command"));
		if (section == 1) return (i18n ("Flags"));
	}

	return QVariant ();
}

void RCommandStackModel::aboutToPop (RCommandBase* popped) {
	if (!listeners) return;
	RK_TRACE (RBACKEND);

	RK_ASSERT (RInterface::inRThread ());
	MUTEX_UNLOCK;	// release the mutex in the R thread, as the main thread will need it.
	emit (aboutToChange ());
	MUTEX_LOCK;
}

void RCommandStackModel::popComplete (RCommandBase* popped) {
	if (!listeners) return;
	RK_TRACE (RBACKEND);

	RK_ASSERT (RInterface::inRThread ());
	MUTEX_UNLOCK;	// release the mutex in the R thread, as the main thread will need it.
	emit (change ());
	MUTEX_LOCK;
}

void RCommandStackModel::aboutToAdd (RCommandBase* added) {
	if (!listeners) return;
	RK_TRACE (RBACKEND);

	RK_ASSERT (!RInterface::inRThread ());
	relayAboutToChange ();
}

void RCommandStackModel::addComplete (RCommandBase* added) {
	if (!listeners) return;
	RK_TRACE (RBACKEND);

	RK_ASSERT (!RInterface::inRThread ());
	relayChange ();
}

void RCommandStackModel::lockMutex () const {
	if (have_mutex_lock) return;
	RK_TRACE (RBACKEND);

	RK_ASSERT (!RInterface::inRThread ());

	MUTEX_LOCK;
// We're playing silly const games, here, as the reimplemenations from QAbstractItemModel need to be const.
// Well, we're not really changing anything, though, just keeping track of the mutex lock.
	QTimer::singleShot (0, const_cast<RCommandStackModel*> (this), SLOT (unlockMutex()));

	bool *cheat = const_cast<bool*> (&have_mutex_lock);
	*cheat = true;
}

void RCommandStackModel::unlockMutex () {
	if (!have_mutex_lock) return;
	RK_TRACE (RBACKEND);

	RK_ASSERT (!RInterface::inRThread ());

	MUTEX_UNLOCK;
	have_mutex_lock = false;
}

void RCommandStackModel::relayChange () {
	RK_TRACE (RBACKEND);

	RK_ASSERT (!RInterface::inRThread ());
// TODO: for now we update everything on every change
// TODO: will need mutex locking for real solution

	emit (layoutChanged ());
}

void RCommandStackModel::relayAboutToChange () {
	RK_TRACE (RBACKEND);

	RK_ASSERT (!RInterface::inRThread ());
// TODO: for now we update everything on every change
// TODO: will need mutex locking for real solution

	emit (layoutAboutToBeChanged ());
}


#include "rcommandstack.moc"
