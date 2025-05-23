/*
robjectlist - This file is part of the RKWard project. Created: Wed Aug 18 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ROBJECTLIST_H
#define ROBJECTLIST_H

#include <qobject.h>

#include <qmap.h>
#include <qstring.h>

#include <QUrl>

#include "rcontainerobject.h"

class QTimer;
class RCommand;
class RCommandChain;
class RKEditor;
class REnvironmentObject;
class RKInlineProgressControl;
class RKOrphanNamespacesObject;

/**
This class is responsible for keeping and updating a list of objects in the R-workspace.
Actually it kind of represents the R-workspace, including methods to save and load the workspace.
It acts as the "document".

@author Thomas Friedrichsmeier
*/
class RObjectList : public QObject, public RContainerObject {
	Q_OBJECT
  public:
	RObjectList();
	~RObjectList();

	void updateFromR(RCommandChain *chain) override;
	/** like updateFromR, but only adjusts to new / missing environments, but does not update the .GlobalEnv. Designed to be used from the backend, when packages were loaded/unloaded . */
	void updateFromR(RCommandChain *chain, const QStringList &current_searchpath, const QStringList &current_namespaces);

	QString getFullName(int) const override { return QString(); };
	QString makeChildName(const QString &short_child_name, int) const override { return short_child_name; };
	/** reimplemented from RContainerObject: do nothing. The object-list has no meta data. */
	void writeMetaData(RCommandChain *) override{};

	REnvironmentObject *findPackage(const QString &namespacename) const;

	static RObjectList *getObjectList() { return object_list; };
	static REnvironmentObject *getGlobalEnv() { return object_list->globalenv; };
	static void init();

	/** detach the given list of packages (if the packages are loaded, and safe to remove)
	@returns a list of error messages (usually empty) */
	QStringList detachPackages(const QStringList &packages, RCommandChain *chain = nullptr, RKInlineProgressControl *control = nullptr);
	/** A pseudo object containing as children all loaded namespaces which do not belong to a package on the search path */
	RKOrphanNamespacesObject *orphanNamespacesObject() const { return orphan_namespaces; };
	QString getObjectDescription() const override;
  Q_SIGNALS:
	/// emitted when the list of objects is about to be updated	// TODO: remove me
	void updateStarted();
	/// emitted when the list of objects has been updated	// TODO: remove me
	void updateComplete();

  protected:
	/** reimplemented from RContainerObject to search the environments in search order */
	RObject::ObjectList findObjects(const QStringList &path, bool partial, const QString &op) override;

	/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString removeChildCommand(RObject *object) const override;
	/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString renameChildCommand(RObject *object, const QString &new_name) const override;
	/// reimplemented from RContainerObject to Q_EMIT a change signal
	void objectsChanged();
	bool updateStructure(RData *new_data) override;
	void updateEnvironments(const QStringList &env_names, bool force_globalenv_update);
	void updateNamespaces(const QStringList &namespace_names);

  private:
	friend class RKLoadAgent;
	friend class RKSaveAgent;
	QTimer *update_timer;

	RCommandChain *update_chain;
	RKOrphanNamespacesObject *orphan_namespaces;

	REnvironmentObject *createTopLevelEnvironment(const QString &name);
	void makeUpdateCompleteCallback();

	REnvironmentObject *globalenv;
	static RObjectList *object_list;
};

/**
\page RepresentationOfRObjectsInRKWard Representation of R objects in RKWard
\brief How objects in R space are represented in RKWard

Due to primarily two reasons, RKWard needs to keep it's own list of objects in the R workspace. The first, and most important reason is threading: R objects might be modified or even removed in the R backend, while the GUI thread is trying to access them. Since we have no control over what's going on inside R, this cannot be solved with a simple mutex. So rather, we copy a representation into memory accessed by the GUI thread only (in the future, maybe the backend thread will get access to this representation for more efficient updating, but still a representation separate from that kept in R itself is needed).

The second reason is that R and Qt includes clash, and we cannot easily use R SEXPs directly in Qt code.

RKWard then uses an own specialized description of R objects. This is slightly more abstracted than objects in R, but stores the most important information about each object, and of course the hierarchical organization of objects.

TODO: write me!

@see RObject
@see RKVariable
@see RContainerObject
@see RObjectList
@see RKModificationTracker

 */

#endif
