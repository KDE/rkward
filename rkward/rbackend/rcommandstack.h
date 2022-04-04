/*
rcommandstack - This file is part of the RKWard project. Created: Mon Sep 6 2004
SPDX-FileCopyrightText: 2004-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RCOMMANDSTACK_H
#define RCOMMANDSTACK_H

#include "rcommand.h"

/**
* This class represents the top-level RCommandChain, which persists for the entire session.
* Having a separate class for this (singleton!) is a bit of a historical left-over. However, it does make a bit of sense, as this essentially provides the API
* for manipulation of RCommandChain s, which should be used by RInterface, only.
* 
* The main job of this class is to allow fetching commands in the correct order.
* 
* @author Thomas Friedrichsmeier
*/
class RCommandStack : public RCommandChain {
public:
	RCommandStack ();
	~RCommandStack ();

/** add a command to the given chain (static, as it does no matter, which stack the chain belongs to) */
	static void issueCommand (RCommand *command, RCommandChain *chain);

/** add a sub-chain to the given chain (static, as it does no matter, which stack the chain belongs to) */
	static RCommandChain *startChain (RCommandChain *parent);
/** close the given chain, i.e. signal the chain may be deleted once its remaining commands are done
 (static, as it does no matter, which stack the chain belongs to). */
	static void closeChain (RCommandChain *chain);

/** removes the given RCommand from the stack. */
	static void pop (RCommandChain *item);
	static bool popIfCompleted (RCommandChain *item);
	static RCommandChain* activeSubItemOf (RCommandChain *item);

/** returns a pointer to the current command to be processed. NOTE: This is really non-const. Chains which have been closed might be removed. */
	static RCommand* currentCommand ();
	static QList<RCommand*> allCommands ();

/** the regular command stack, i.e. not a callback */
	static RCommandStack *regular_stack;
private:
	static void issueCommandInternal (RCommandChain *child, RCommandChain *parent);
	static bool removeFromParent (RCommandChain *child);
friend class RCommandStackModel;
	static void listCommandsRecursive (QList<RCommand*> *list, const RCommandChain *chain);
};

#include <QAbstractItemModel>

/** The model used to fetch a representation of and signal changes in the RCommandStack. Used for RKControlWindow.

- All insertions / removals are signaled to the (single) model
- it is ok for the model to be slow.
- the model keeps track of (the number of) listeners, and does not do anything unless there are any listeners (including walking the stack)
- RControlWindow will only be constructed on show, and destructed on hide, so as not to eat resources

@author Thomas Friedrichsmeier
*/
class RCommandStackModel : public QAbstractItemModel {
	Q_OBJECT
public:
	explicit RCommandStackModel (QObject *parent);
	~RCommandStackModel ();

	/** implements QAbstractItemModel::index() */
	QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex ()) const override;
	/** implements QAbstractItemModel::parent() */
	QModelIndex parent (const QModelIndex& index) const override;
	/** implements QAbstractItemModel::rowCount() */
	int rowCount (const QModelIndex& parent = QModelIndex ()) const override;
	/** implements QAbstractItemModel::columnCount(). This is identical for all items */
	int columnCount (const QModelIndex& parent = QModelIndex ()) const override;
	/** implements QAbstractItemModel::data() */
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;
	/** reimplemented from  QAbstractItemModel::headerData() to make only commands (not chains/stacks) selectable */
	Qt::ItemFlags flags (const QModelIndex& index) const override;
	/** reimplemented from  QAbstractItemModel::headerData() to provide column names */
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	/** static pointer to the model. Only one model will ever be around. */
	static RCommandStackModel* getModel () { return static_model; };

	/** add a listener. The model will do nothing, if there are no listeners. Remember to remove the listener again as soon as possible. @see removeListener() */
	void addListener ();
	/** @see addListener() */
	void removeListener ();

	/** call this, when you are about to remove an item from a command stack/chain, *before* you actually remove the item. When done, call popComplete().
	@param parent The parent of the item to be removed */
	void aboutToPop (RCommandChain* parent, int index);
	/** @see aboutToPop () */
	void popComplete ();
	/** call this, when you are about to add an item to a command stack/chain, *before* you actually add the item. When done, call addComplete().
	@param parent The parent of the item to be removed */
	void aboutToAdd (RCommandChain* parent, int index);
	/** @see aboutToAdd () */
	void addComplete ();
	/** call this, when you have made changes to an item, that should be reflected in RControlWindow
	@param item The item that was changed */
	void itemChange (RCommandChain* item);
private:
	/** number of listeners. If there are no listeners, the model will do almost nothing at all */
	int listeners;
	static RCommandStackModel* static_model;

	/** create a model index for the given item */
	QModelIndex indexFor (RCommandChain *item);
};

#endif
