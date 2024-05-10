/*
rkeditor - This file is part of the RKWard project. Created: Fri Aug 20 2004
SPDX-FileCopyrightText: 2004 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKEDITOR_H
#define RKEDITOR_H

#include <QWidget>

#include "../core/robject.h"

#include <kparts/part.h>

#include "../windows/rkmdiwindow.h"

class RCommandChain;
class RKDrag;

/**
Use as a base class for all widgets that can be used to display and edit RObjects of whatever type.

// TODO: not sure we really need this any longer
@author Thomas Friedrichsmeier
*/
class RKEditor : public RKMDIWindow {
	Q_OBJECT
protected:
    RKEditor (QWidget *parent);

    virtual ~RKEditor ();
public:
/// flushes all pending edit operations and syncs the data to R. Implement in the child classes
	virtual void flushChanges () = 0;
/// returns the object that is being edited in this editor
	RObject *getObject () { return object; };
	
	enum PasteMode {PasteEverywhere, PasteToTable, PasteToSelection};

/** Tells the editor to restore the given object in the R-workspace from its copy of the data */
	virtual void restoreObject (RObject *object) = 0;

	bool isModified() const override { return false; };
protected:
	RObject *object;
};

#endif
