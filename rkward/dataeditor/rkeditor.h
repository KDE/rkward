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

class RCommandChain;
class RObject;
class RKDrag;

/**
Use a a base class for all widgets that can be used to display and edit RObjects of whatever type.

@author Thomas Friedrichsmeier
*/
class RKEditor : public QWidget
{
Q_OBJECT
protected:
    RKEditor (QWidget *parent);

    virtual ~RKEditor ();
public:
/// syncs changes done in the editor (if any) to the R workspace. Implement in the child classes
	virtual void syncToR (RCommandChain *chain) = 0;
/// returns the object that is being edited in this editor
	RObject *getObject () { return object; };
	
/// editing functions:
	virtual void clearSelected () = 0;
	virtual RKDrag *makeDrag () = 0;
	virtual void pasteEncoded (QByteArray content) = 0;
	enum PasteMode {PasteEverywhere, PasteToTable, PasteToSelection};
	virtual void setPasteMode (PasteMode mode) = 0;
protected:
friend class RKEditorManager;
/// opens the given object. Implement in the child-classes
	virtual void openObject (RObject *object) = 0;
	RObject *object;
};

#endif
