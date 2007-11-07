/***************************************************************************
                          rkeditordataframepart  -  description
                             -------------------
    begin                : Wed Sep 14 2005
    copyright            : (C) 2005, 2007 by Thomas Friedrichsmeier
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

#ifndef RKEDITORDATAFRAMEPART_H
#define RKEDITORDATAFRAMEPART_H

#include <kparts/part.h>

#include "rkeditor.h"

class RKEditorDataFrame;
class KAction;

/** This class provides a KPart interface to RKEditorDataFrame. Basically, it is responsible for creating the menu-entries the RKEditorDataFrame provides, and keeps the corresponding Actions. The reason to use this, is so the required menus/menu-items can be merged in on the fly.

@author Thomas Friedrichsmeier
*/
class RKEditorDataFramePart : public KParts::Part {
	Q_OBJECT
protected:
friend class RKEditorDataFrame;
/** ctor. Protected, as this should only be created by an RKEditorDataFrame */
	RKEditorDataFramePart (QObject *parent, RKEditorDataFrame* editor);
/** dtor */
	~RKEditorDataFramePart ();
public slots:
/** put the marked cells into the clipboard and remove them from the table */
	void slotEditCut();
/** put the marked cells into the clipboard */
	void slotEditCopy();
/** paste the clipboard into the table, expanding the table as necessary */
	void slotEditPaste();
/** paste the clipboard into the table, but not beyond table boundaries */
	void slotEditPasteToTable();
/** paste the clipboard into the table, but not beyond selection boundaries	*/
	void slotEditPasteToSelection();
/** return a pointer to the underlying editor widget */
	RKEditorDataFrame *getEditor () const { return editor; };
private:
	QAction* editCut;
	QAction* editCopy;
	QAction* editPaste;
	QAction* editPasteToSelection;
	QAction* editPasteToTable;

/** we could always use static_cast<RKEditorDataFrame*> (widget ()) instead of keeping this pointer. This saves some typing, though */
	RKEditorDataFrame *editor;

	void initializeActions ();

/** Does pasting (called from the respective slots) */
	void doPaste (RKEditor::PasteMode mode);
};

#endif
