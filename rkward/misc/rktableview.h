/***************************************************************************
                          rktableview  -  description
                             -------------------
    begin                : Tue Nov 06
    copyright            : (C) 2012 by Thomas Friedrichsmeier
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

#ifndef RKTABLEVIEW_H
#define RKTABLEVIEW_H

#include <QTableView>
#include <QItemDelegate>

class RKVarEditMetaModel;
class RKVarEditModel;

/** Item delegate for TwinTableMembers, and RKTableView
@author Thomas Friedrichsmeier */
class RKItemDelegate : public QItemDelegate {
	Q_OBJECT
public:
	RKItemDelegate (QObject *parent, RKVarEditModel* datamodel);
	RKItemDelegate (QObject *parent, RKVarEditMetaModel* metamodel);
	/** dummy to avoid casting ambiguity */
	RKItemDelegate (QObject *parent, QAbstractItemModel* model, bool dummy);
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
	QAbstractItemModel* genericmodel;
	bool locked_for_modal_editor;
};

/** simple wrapper around QTableView to fix a couple shortcomings. 
 * 
 *  TODO: merge cut() and copy() slots from TwinTableMember, RKMatrixInput, and EditLabelsDialog, here.
 *        (for paste() this is probably not possible, though)
 */
class RKTableView : public QTableView {
	Q_OBJECT
public:
	RKTableView (QWidget *parent);
	virtual ~RKTableView ();

	virtual int trueRows () const { return apparentRows () + trailing_rows; };
	virtual int trueColumns () const { return apparentColumns () + trailing_columns; };
	int apparentRows () const;
	int apparentColumns () const;

	void setRKItemDelegate (RKItemDelegate* delegate);

	QItemSelectionRange getSelectionBoundaries () const;
	int trailing_rows;
	int trailing_columns;
signals:
	void blankSelectionRequest ();
protected:
/** will emit blankSelectionRequest() on DEL and BACKSPACE. Also scrolls to current index on key presses. */
	void keyPressEvent (QKeyEvent *e);
private slots:
	void editorDone (QWidget* editor, RKItemDelegate::EditorDoneReason reason);
};

#endif
