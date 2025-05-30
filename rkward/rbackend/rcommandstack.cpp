/*
rcommandstack - This file is part of RKWard (https://rkward.kde.org). Created: Mon Sep 6 2004
SPDX-FileCopyrightText: 2004-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rcommandstack.h"

#include <KLocalizedString>

#include <QTime>
#include <QTimer>

#include "rkrinterface.h"

#include "../debug.h"

// static
RCommandStack *RCommandStack::regular_stack;

RCommandStack::RCommandStack() : RCommandChain() {
	RK_TRACE(RBACKEND);
	closed = false;
	parent = nullptr;
}

RCommandStack::~RCommandStack() {
	RK_TRACE(RBACKEND);
}

void RCommandStack::issueCommand(RCommand *command, RCommandChain *chain) {
	RK_TRACE(RBACKEND);
	if (!chain) chain = regular_stack;
	issueCommandInternal(command, chain);
}

void RCommandStack::issueCommandInternal(RCommandChain *child, RCommandChain *parent) {
	RK_TRACE(RBACKEND);
	RK_ASSERT(parent);

	int pos = parent->sub_commands.size();
	if (child->is_command && (child->toCommand()->type() & RCommand::PriorityCommand)) {
		// Add priority commands before any regular command, after other priority commands. Always in main chain.
		RK_ASSERT(parent == regular_stack);
		for (pos = 0; pos < parent->sub_commands.size(); ++pos) {
			RCommand *com = parent->sub_commands[pos]->toCommand();
			if (!(com && (com->type() & RCommand::PriorityCommand))) break;
		}
	}
	RCommandStackModel::getModel()->aboutToAdd(parent, pos);
	parent->sub_commands.insert(pos, child);
	child->parent = parent;
	RCommandStackModel::getModel()->addComplete();
}

bool RCommandStack::removeFromParent(RCommandChain *child) {
	RK_TRACE(RBACKEND);
	RCommandChain *parent = child->parent;
	RK_ASSERT(parent);

	int index = parent->sub_commands.indexOf(child);
	if (index < 0) return false;

	RCommandStackModel::getModel()->aboutToPop(parent, index);
	parent->sub_commands.removeAt(index);
	if (!child->is_command) delete child;
	//	else child->parent = 0;        // There is at least on point in the code, where we access the parent (chain) after the command was popped. This is rather unsafe in the first place, but...
	RCommandStackModel::getModel()->popComplete();

	return true;
}

RCommandChain *RCommandStack::startChain(RCommandChain *parent) {
	RK_TRACE(RBACKEND);
	if (!parent) parent = regular_stack;

	RCommandChain *chain = new RCommandChain();
	issueCommandInternal(chain, parent);
	return chain;
}

void RCommandStack::closeChain(RCommandChain *chain) {
	RK_TRACE(RBACKEND);
	if (!chain) return;

	chain->closed = true;
	RCommandStackModel::getModel()->itemChange(chain);
	popIfCompleted(chain);
}

RCommand *RCommandStack::currentCommand() {
	RK_TRACE(RBACKEND);

	RCommandChain *dummy;
	do { // first pop any empty things in the way
		dummy = activeSubItemOf(regular_stack);
	} while (popIfCompleted(dummy));

	RCommandChain *ret = activeSubItemOf(regular_stack);
	if (ret) return ret->toCommand(); // might still be 0, if it is not a command
	return nullptr;
}

RCommandChain *RCommandStack::activeSubItemOf(RCommandChain *item) {
	RK_TRACE(RBACKEND);

	if (item->sub_commands.isEmpty() && item->is_command && item->isClosed()) return item;
	if (item->sub_commands.isEmpty()) return item;
	return activeSubItemOf(item->sub_commands.first());
}

void RCommandStack::listCommandsRecursive(QList<RCommand *> *list, const RCommandChain *chain) {
	RK_TRACE(RBACKEND);

	if (chain->is_command) list->append(const_cast<RCommandChain *>(chain)->toCommand());
	for (const RCommandChain *coc : std::as_const(chain->sub_commands)) {
		listCommandsRecursive(list, coc);
	}
}

QList<RCommand *> RCommandStack::allCommands() {
	RK_TRACE(RBACKEND);

	QList<RCommand *> ret;
	listCommandsRecursive(&ret, regular_stack);
	return ret;
}

void RCommandStack::pop(RCommandChain *item) {
	RK_TRACE(RBACKEND);

	RCommandChain *parent = item->parent;
	RK_DEBUG(RBACKEND, DL_DEBUG, "removing form parent: %s", item->isCommand() ? qPrintable(item->toCommand()->command()) : "<chain>");
	removeFromParent(item);
	popIfCompleted(parent);
}

bool RCommandStack::popIfCompleted(RCommandChain *item) {
	RK_TRACE(RBACKEND);

	if (item->isClosed() && item->sub_commands.isEmpty() && item->parent && (!item->is_command)) { // if the item has no parent, it is the main stack. If it is a command, it will be popped from the RInterface.
		RK_DEBUG(RBACKEND, DL_DEBUG, "popping completed chain: %p", item);
		pop(item);
		return true;
	}
	return false;
}

/////////////////////// RCommandStackModel ////////////////////

#define MAIN_COL 0
#define STATUS_COL 1
#define FLAGS_COL 2
#define DESC_COL 3
#define NUM_COLS 4

// static
RCommandStackModel *RCommandStackModel::static_model = nullptr;

RCommandStackModel::RCommandStackModel(QObject *parent) : QAbstractItemModel(parent) {
	RK_TRACE(RBACKEND);
	RK_ASSERT(static_model == nullptr); // only one instance should be created

	static_model = this;
	listeners = 0;
}

RCommandStackModel::~RCommandStackModel() {
	RK_TRACE(RBACKEND);

	static_model = nullptr;
	RK_ASSERT(!listeners);
}

void RCommandStackModel::addListener() {
	RK_TRACE(RBACKEND);

	++listeners;
}

void RCommandStackModel::removeListener() {
	RK_TRACE(RBACKEND);

	--listeners;
	RK_ASSERT(listeners >= 0);
}

QModelIndex RCommandStackModel::index(int row, int column, const QModelIndex &parent) const {
	RK_ASSERT(listeners > 0);
	RK_TRACE(RBACKEND);

	RCommandChain *index_data = nullptr;
	if (!parent.isValid()) {
		index_data = RCommandStack::regular_stack;
	} else {
		RCommandChain *parent_index = static_cast<RCommandChain *>(parent.internalPointer());
		RK_ASSERT(parent_index);

		if (parent_index->sub_commands.size() <= row) {
			RK_ASSERT(false);
			return QModelIndex();
		}
		index_data = parent_index->sub_commands[row];
	}

	return (createIndex(row, column, index_data));
}

QModelIndex RCommandStackModel::parent(const QModelIndex &child) const {
	RK_ASSERT(listeners);
	RK_TRACE(RBACKEND);

	if (child.isValid()) {
		RCommandChain *child_index = static_cast<RCommandChain *>(child.internalPointer());
		RK_ASSERT(child_index);

		RCommandChain *index_data = child_index->parent;
		if (!index_data) return QModelIndex(); // probably the regular_stack
		if (index_data) return (createIndex(0, 0, index_data));
	}

	return QModelIndex();
}

int RCommandStackModel::rowCount(const QModelIndex &parent) const {
	RK_ASSERT(listeners);
	RK_TRACE(RBACKEND);

	if (!parent.isValid()) return 1;

	RCommandChain *index_data = static_cast<RCommandChain *>(parent.internalPointer());
	RK_ASSERT(index_data);
	return (index_data->sub_commands.size());
}

int RCommandStackModel::columnCount(const QModelIndex &) const {
	RK_ASSERT(listeners);
	RK_TRACE(RBACKEND);

	return NUM_COLS;
}

QVariant RCommandStackModel::data(const QModelIndex &index, int role) const {
	RK_ASSERT(listeners);
	RK_TRACE(RBACKEND);

	if (!index.isValid()) return QVariant();
	RK_ASSERT(index.model() == this);

	RCommandChain *index_data = static_cast<RCommandChain *>(index.internalPointer());

	if (index_data->is_command) {
		RCommand *command = index_data->toCommand();
		if ((index.column() == MAIN_COL) && (role == Qt::DisplayRole)) return (command->command());
		if ((index.column() == FLAGS_COL) && (role == Qt::DisplayRole)) {
			QString ret;
			if (command->type() & RCommand::User) ret += u'U';
			if (command->type() & RCommand::Plugin) ret += u'P';
			if (command->type() & RCommand::App) ret += u'A';
			if (command->type() & RCommand::Sync) ret += u'S';
			if (command->type() & RCommand::EmptyCommand) ret += u'E';
			if (command->type() & (RCommand::GetIntVector | RCommand::GetRealVector | RCommand::GetStringVector | RCommand::GetStructuredData)) ret += u'D';
			if (command->type() & RCommand::CCOutput) ret += u'O';
			return (ret);
		}
		if ((index.column() == STATUS_COL) && (role == Qt::DisplayRole)) {
			QString ret;
			if (command->status & RCommand::Running) ret += i18n("Running");
			if (command->status & RCommand::Canceled) {
				if (!ret.isEmpty()) ret += u", "_s;
				ret += i18n("Canceled");
			}
			return (ret);
		}
		if ((index.column() == DESC_COL) && (role == Qt::DisplayRole)) {
			return (command->rkEquivalent());
		}
	} else {
		if (index_data->parent) {
			if ((index.column() == MAIN_COL) && (role == Qt::DisplayRole)) return (i18n("Command Chain"));
			if ((index.column() == STATUS_COL) && (role == Qt::DisplayRole)) {
				if (index_data->closed) return (i18n("Closed"));
				return (i18n("Waiting"));
			}
		} else {
			if ((index.column() == MAIN_COL) && (role == Qt::DisplayRole)) return (i18n("Command Stack"));
		}
	}

	return (QVariant());
}

Qt::ItemFlags RCommandStackModel::flags(const QModelIndex &index) const {
	RK_ASSERT(listeners);
	RK_TRACE(RBACKEND);

	if (!index.isValid()) return Qt::NoItemFlags;
	RK_ASSERT(index.model() == this);

	RCommandChain *index_data = static_cast<RCommandChain *>(index.internalPointer());
	RK_ASSERT(index_data);

	if (index_data->is_command) return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	return Qt::ItemIsEnabled;
}

QVariant RCommandStackModel::headerData(int section, Qt::Orientation orientation, int role) const {
	RK_ASSERT(listeners);
	RK_TRACE(RBACKEND);

	if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
		if (section == MAIN_COL) return (i18n("Command"));
		if (section == FLAGS_COL) return (i18n("Type"));
		if (section == STATUS_COL) return (i18n("Status"));
		if (section == DESC_COL) return (i18n("Description"));
	}

	return QVariant();
}

QModelIndex RCommandStackModel::indexFor(RCommandChain *item) {
	RK_ASSERT(listeners);
	RK_TRACE(RBACKEND);

	if (!item) return (QModelIndex());

	if (!item->parent) {
		// stack is always the first (and only) child
		return (createIndex(0, 0, item));
	}

	int row = item->parent->sub_commands.indexOf(item);
	if (row < 0) {
		RK_ASSERT(false);
		return (QModelIndex());
	}
	return (createIndex(row, 0, item));
}

void RCommandStackModel::aboutToPop(RCommandChain *parent, int index) {
	if (!listeners) return;
	RK_TRACE(RBACKEND);

	QModelIndex parent_index = indexFor(parent);
	beginRemoveRows(parent_index, index, index);
}

void RCommandStackModel::popComplete() {
	if (!listeners) return;
	RK_TRACE(RBACKEND);

	endRemoveRows();
}

void RCommandStackModel::aboutToAdd(RCommandChain *parent, int index) {
	if (!listeners) return;
	RK_TRACE(RBACKEND);

	QModelIndex parent_index = indexFor(parent);
	beginInsertRows(parent_index, index, index);
}

void RCommandStackModel::addComplete() {
	if (!listeners) return;
	RK_TRACE(RBACKEND);

	endInsertRows();
}

void RCommandStackModel::itemChange(RCommandChain *item) {
	if (!listeners) return;
	RK_TRACE(RBACKEND);

	QModelIndex item_index = indexFor(item);
	Q_EMIT dataChanged(item_index, item_index);
}
