/*
twintablemember.h - This file is part of the RKWard project. Created: Tue Oct 29 2002
SPDX-FileCopyrightText: 2002-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	explicit TwinTableMember (QWidget *parent);
	~TwinTableMember ();
	TwinTableMember *getTwin () { return twin; };
/** ends editing. Actually it's just a simple wrapper around QTable::endEdit () */
	void stopEditing ();

	void copy ();
	void paste (RKEditor::PasteMode mode);

	void setRKModel (RKVarEditModelBase* model);
	int trueRows () const override;	// re-implemented from RKTableView
	int trueColumns () const override;	// re-implemented from RKTableView
public Q_SLOTS:
/** blanks out the currently selected cells (or the currently active cell, if there is no selection) */
	void blankSelected ();
Q_SIGNALS:
	void contextMenuRequest (int row, int col, const QPoint& pos);
protected:
	TwinTableMember *twin;
	bool updating_twin;

/** reimplemented from QTableView to also adjust the twin */
	void scrollContentsBy (int dx, int dy) override;

	RKVarEditModelBase* mymodel;

	bool rw;
friend class TwinTable;
	void setTwin (TwinTableMember *new_twin);
protected Q_SLOTS:
	void handleContextMenuRequest (const QPoint& pos);
	void updateColWidth (int section, int old_w, int new_w);
	void tableSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected);
};

#endif
