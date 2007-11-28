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
#ifndef RCOMMANDSTACK_H
#define RCOMMANDSTACK_H

#include "rcommand.h"

/**
This class represents a top-level RCommandChain. There are/will be two (types of) such chains: One for commands sent to the R-backend for "regular" evaluation, and one for commands sent in reply to a (modal) request from the R-backend via the RRequestHandler.
The class provides convenience functions for determining the state of the Stack (empty/blocked/active) and inserting and fetching commands in the correct order.
Remember to lock RInterface::mutex before accessing any of these functions!

@author Thomas Friedrichsmeier
*/
class RCommandStack : public RCommandChain {
public:
	RCommandStack (RCommand *parent_command);
	~RCommandStack ();

/** add a command to the given chain (static, as it does no matter, which stack the chain belongs to) */
	static void issueCommand (RCommand *command, RCommandChain *chain);

/** add a sub-chain to the given chain (static, as it does no matter, which stack the chain belongs to) */
	static RCommandChain *startChain (RCommandChain *parent);
/** close the given chain, i.e. signal the chain may be deleted once its remaining commands are done
 (static, as it does no matter, which stack the chain belongs to). */
	static void closeChain (RCommandChain *chain);

/** @returns true, if there are no commands or open chains waiting in this stack */
	bool isEmpty ();
/** @returns true, if there are commands to be processed in the current chain */
	bool isActive ();

/** removes the RCommand to be processed next from the stack and sets it as the currentCommand(). If there is no command to process right now, currentCommand() will be 0. Note: we can't just return the popped command for internal reasons. */
	void pop ();

/** see pop() */
	RCommand* currentCommand ();

/** the regular command stack, i.e. not a callback */
	static RCommandStack *regular_stack;

	static RCommandStack* currentStack ();

/** return the parent RCommandStack of the given RCommandChain */
	static RCommandStack *chainStack (RCommandChain *child);
/** return the parent RCommandStack of the given RCommand */
	static RCommandStack *stackForCommand (RCommand *child);
private:
friend class RCommandStackModel;
	RCommandChain *current_chain;
/** super-ordinated command. 0 for the regular_stack */
	RCommand *parent_command;
/** pointer to any substack. Will only be non-zero, if the substack is active */
	RCommandStack *sub_stack;
	void clearFinishedChains ();
};

#include <QAbstractItemModel>

/** The model used to fetch a representation of and signal changes in the RCommandStack. Used for RKControlWindow.

- All insertions / removals are signalled to the (single) model
- it is ok for the model to be slow.
- the model keeps track of (the number of) listeners, and does not do anything unless there are any listeners (including walking the stack)
- RControlWindow will only be constructed on show, and destructed on hide, so as not to eat resources

@author Thomas Friedrichsmeier
*/
class RCommandStackModel : public QAbstractItemModel {
	Q_OBJECT
public:
	RCommandStackModel (QObject *parent);
	~RCommandStackModel ();

	/** implements QAbstractItemModel::index() */
	QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex ()) const;
	/** implements QAbstractItemModel::parent() */
	QModelIndex parent (const QModelIndex& index) const;
	/** implements QAbstractItemModel::rowCount() */
	int rowCount (const QModelIndex& parent = QModelIndex ()) const;
	/** implements QAbstractItemModel::columnCount(). This is identical for all items */
	int columnCount (const QModelIndex& parent = QModelIndex ()) const;
	/** implements QAbstractItemModel::data() */
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
	/** reimplemented from  QAbstractItemModel::headerData() to make only commands (not chains/stacks) selectable */
	Qt::ItemFlags flags (const QModelIndex& index) const;
	/** reimplemented from  QAbstractItemModel::headerData() to provide column names */
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	/** static pointer to the model. Only one model will ever be around. */
	static RCommandStackModel* getModel () { return static_model; };

	/** add a listener. The model will do nothing, if there are no listeners. Remember to remove the listener again as soon as possible. @see removeListener() */
	void addListener ();
	/** @see addListener() */
	void removeListener ();

	/** call this, when you are about to remove an item from a command stack/chain, *before* you actually remove the item. When done, call popComplete().
	@param parent The parent of the item to be removed */
	void aboutToPop (RCommandBase* parent);
	/** @see aboutToPop () */
	void popComplete ();
	/** call this, when you are about to add an item to a command stack/chain, *before* you actually add the item. When done, call addComplete().
	@param parent The parent of the item to be removed */
	void aboutToAdd (RCommandBase* parent);
	/** @see aboutToAdd () */
	void addComplete ();
	/** call this, when you have made changes to an item, that should be reflected in RControlWindow
	@param item The item that was changed */
	void itemChange (RCommandBase* item);
private slots:
	/** compagnon to lockMutex () */
	void unlockMutex ();
	/** the purpose of these signals and slots is to make sure the aboutToPop(), popComplete(), aboutToAdd(), addComplete(), and itemChange() methods are *always* dealt with in the main (GUI) thread. */
	void relayItemAboutToBeAdded (RCommandBase* parent);
	/** @see relayItemAboutToBeAdded() */
	void relayItemAdded ();
	/** @see relayItemAboutToBeAdded() */
	void relayItemAboutToBeRemoved (RCommandBase* parent);
	/** @see relayItemAboutToBeAdded() */
	void relayItemRemoved ();
	/** @see relayItemAboutToBeAdded() */
	void relayItemChanged (RCommandBase* item);
signals:
	/** @see relayItemAboutToBeAdded() */
	void itemAboutToBeAdded (RCommandBase* parent);
	/** @see relayItemAboutToBeAdded() */
	void itemAdded ();
	/** @see relayItemAboutToBeAdded() */
	void itemAboutToBeRemoved (RCommandBase* parent);
	/** @see relayItemAboutToBeAdded() */
	void itemRemoved ();
	/** @see relayItemAboutToBeAdded() */
	void itemChanged (RCommandBase* item);
private:
	/** locks the mutex temporarily (until the event loop is next entered again), and only, if not already locked */
	void lockMutex () const;
	/** number of listeners. If there are no listeners, the model will do almost nothing at all */
	int listeners;
	static RCommandStackModel* static_model;
	/** @see lockMutex() */
	bool have_mutex_lock;

	/** create a model index for the given item */
	QModelIndex indexFor (RCommandBase *item);
};

#endif
