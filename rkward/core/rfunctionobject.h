/*
rfunctionobject - This file is part of the RKWard project. Created: Wed Apr 26 2006
SPDX-FileCopyrightText: 2006-2019 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RFUNCTION_H
#define RFUNCTION_H

#include "robject.h"

class RCommand;

/**
Internal representation of function objects in the R workspace

@author Thomas Friedrichsmeier
*/

class RFunctionObject : public RObject {
  public:
	RFunctionObject(RObject *parent, const QString &name);
	~RFunctionObject() override;

	/** reimplemented from RObject to handle function arguments */
	bool updateStructure(RData *new_data) override;
	QString printArgs() const;
	QStringList argumentNames() const { return argnames; }
	QStringList argumentDefaults() const { return argvalues; }

  protected:
	QStringList argnames;
	QStringList argvalues;
	bool updateArguments(RData *new_data);
};

#endif
