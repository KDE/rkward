/***************************************************************************
                          rkeditor  -  description
                             -------------------
    begin                : Fri Aug 20 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef RKEDITOR_H
#define RKEDITOR_H

#include <qwidget.h>

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

	QString getDescription ();
	bool isModified () { return false; };
protected:
	RObject *object;
};

#endif
