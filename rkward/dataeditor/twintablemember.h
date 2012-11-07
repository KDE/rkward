/***************************************************************************
                          twintablemember.h  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002, 2006, 2007, 2010, 2012 by Thomas Friedrichsmeier
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

#ifndef TWINTABLEMEMBER_H
#define TWINTABLEMEMBER_H

#include <qpoint.h>
#include <QEvent>
#include <QKeyEvent>

class TwinTable;
class CellEditor;
class RKVarEditModelBase;

#include "../misc/rktableview.h"
#include "rkeditor.h"

/** One of the tables used in a TwinTable.
@author Thomas Friedrichsmeier
*/
class TwinTableMember : public RKTableView {
	Q_OBJECT
public: 
	TwinTableMember (QWidget *parent);
	~TwinTableMember ();
	TwinTableMember *getTwin () { return twin; };
/** ends editing. Actually it's just a simple wrapper around QTable::endEdit () */
	void stopEditing ();

	void copy ();
	void paste (RKEditor::PasteMode mode);

	void setRKModel (RKVarEditModelBase* model);
	int trueRows () const;	// re-implemented from RKTableView
	int trueColumns () const;	// re-implemented from RKTableView
public slots:
/** blanks out the currently selected cells (or the currently active cell, if there is no selection) */
	void blankSelected ();
signals:
	void contextMenuRequest (int row, int col, const QPoint& pos);
protected:
	TwinTableMember *twin;
	bool updating_twin;

/** reimplemented from QTableView to also adjust the twin */
	void scrollContentsBy (int dx, int dy);

	RKVarEditModelBase* mymodel;

	bool rw;
friend class TwinTable;
	void setTwin (TwinTableMember *new_twin);
protected slots:
	void handleContextMenuRequest (const QPoint& pos);
	void updateColWidth (int section, int old_w, int new_w);
	void tableSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected);
};

#endif
