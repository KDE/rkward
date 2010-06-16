/***************************************************************************
                          twintablemember.h  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002, 2006, 2007, 2010 by Thomas Friedrichsmeier
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

#include <QTableView>
#include <QItemSelectionRange>
#include <QItemDelegate>
#include <qpoint.h>
#include <QEvent>
#include <QKeyEvent>

class TwinTable;
class CellEditor;
class RKVarEditModelBase;

#include "rkeditor.h"

class RKVarEditMetaModel;
class RKVarEditModel;

/** Item delegate for TwinTableMembers.
@author Thomas Friedrichsmeier */
class RKItemDelegate : public QItemDelegate {
	Q_OBJECT
public:
	RKItemDelegate (QObject *parent, RKVarEditModel* datamodel);
	RKItemDelegate (QObject *parent, RKVarEditMetaModel* metamodel);
	~RKItemDelegate ();

	QWidget* createEditor (QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void setEditorData (QWidget* editor, const QModelIndex& index) const;
	void setModelData (QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
	bool eventFilter (QObject* editor, QEvent* event);

	enum EditorDoneReason {
		EditorExitLeft,
		EditorExitRight,
		EditorExitUp,
		EditorExitDown,
		EditorReject,
		EditorExit
	};
signals:
	// much like QAbstractItemDelegate::closeEditor(), but with our own flexible EndEditHint
	void doCloseEditor (QWidget* editor, RKItemDelegate::EditorDoneReason);
public slots:
	void editorDone (QWidget* editor, RKItemDelegate::EditorDoneReason reason);
private:
	RKVarEditModel* datamodel;
	RKVarEditMetaModel* metamodel;
};


/** One of the tables used in a TwinTable.
@author Thomas Friedrichsmeier
*/
class TwinTableMember : public QTableView {
	Q_OBJECT
public: 
	TwinTableMember (QWidget *parent);
	~TwinTableMember ();
	TwinTableMember *getTwin () { return twin; };
/** ends editing. Actually it's just a simple wrapper around QTable::endEdit () */
	void stopEditing ();
/** reimplemented to delete cell contents on DEL and BACKSPACE. Placed in public, here, so CellEditor can have access */
	void keyPressEvent (QKeyEvent *e);

	void copy ();
	void paste (RKEditor::PasteMode mode);

/** blanks out the currently selected cells (or the currently active cell, if there is no selection) */
	void blankSelected ();
/** shortcut to get the boundaries of the current selection */
	QItemSelectionRange getSelectionBoundaries ();

	void setRKModel (RKVarEditModelBase* model);
	void seRKItemDelegate (RKItemDelegate* delegate);
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
public slots:
	void editorDone (QWidget* editor, RKItemDelegate::EditorDoneReason);
protected slots:
	void handleContextMenuRequest (const QPoint& pos);
	void updateColWidth (int section, int old_w, int new_w);
	void tableSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected);
};

#endif
