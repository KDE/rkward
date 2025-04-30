/*
renvironmentobject - This file is part of the RKWard project. Created: Wed Sep 27 2006
SPDX-FileCopyrightText: 2006-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RENVIRONMENTOBJECT_H
#define RENVIRONMENTOBJECT_H

#include "rcontainerobject.h"

class RCommand;
class RCommandChain;

/**
This class roughly corresponds to an environment in R. It keeps a list of all objects in that environment.

@author Thomas Friedrichsmeier
*/
class REnvironmentObject : public RContainerObject {
  public:
	REnvironmentObject(RContainerObject *parent, const QString &name);
	~REnvironmentObject();

	using RObject::updateFromR;
	/** like updateFromR, but only update new / removed symbols from R. Theoretically this could be defined in RContainerObject, but the only use case is for environments. */
	virtual void updateFromR(RCommandChain *chain, const QStringList &added_symbols, const QStringList &removed_symbols);

	QString getFullName(int) const override;
	QString makeChildName(const QString &short_child_name, int options) const override;
	/** reimplemented from RContainerObject: If this is an environment var, call RContainerObject::writeMetaData (). Else, do nothing. An environment has no meta data. */
	void writeMetaData(RCommandChain *chain) override;
	QString packageName() const;
	QString getObjectDescription() const override;

  protected:
	bool updateStructure(RData *new_data) override;
	/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString removeChildCommand(RObject *object) const override;
	/// reimplemented from RContainerObject to call "remove (objectname)" instead of "objectname <- NULL"
	QString renameChildCommand(RObject *object, const QString &new_name) const override;
	friend class RObject;
	void updateNamespace(RData *new_data);
};

#endif
