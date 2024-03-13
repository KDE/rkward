/*
rktableview - This file is part of the RKWard project. Created: Tue Nov 06
SPDX-FileCopyrightText: 2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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

	QWidget* createEditor (QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void setEditorData (QWidget* editor, const QModelIndex& index) const override;
	void setModelData (QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
	bool eventFilter (QObject* editor, QEvent* event) override;

	enum EditorDoneReason {
		EditorExitLeft,
		EditorExitRight,
		EditorExitUp,
		EditorExitDown,
		EditorReject,
		EditorExit
	};
Q_SIGNALS:
	// much like QAbstractItemDelegate::closeEditor(), but with our own flexible EndEditHint
	void doCloseEditor (QWidget* editor, RKItemDelegate::EditorDoneReason);
public Q_SLOTS:
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
	explicit RKTableView (QWidget *parent);
	virtual ~RKTableView ();

	virtual int trueRows () const { return apparentRows () + trailing_rows; };
	virtual int trueColumns () const { return apparentColumns () + trailing_columns; };
	int apparentRows () const;
	int apparentColumns () const;

	void setRKItemDelegate (RKItemDelegate* delegate);

	QItemSelectionRange getSelectionBoundaries () const;
	int trailing_rows;
	int trailing_columns;
Q_SIGNALS:
	void blankSelectionRequest ();
protected:
/** will Q_EMIT blankSelectionRequest() on DEL and BACKSPACE. Also scrolls to current index on key presses. */
	void keyPressEvent (QKeyEvent *e) override;
private Q_SLOTS:
	void editorDone (QWidget* editor, RKItemDelegate::EditorDoneReason reason);
};

#endif
