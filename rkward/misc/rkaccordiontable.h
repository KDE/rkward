/*
rkaccordiontable - This file is part of the RKWard project. Created: Fri Oct 24 2015
SPDX-FileCopyrightText: 2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKACCORDIONTABLE_H
#define RKACCORDIONTABLE_H

#include <QWidget>
#include <QTreeView>

class RKAccordionDummyModel;

class RKAccordionTable : public QTreeView {
	Q_OBJECT
public:
	explicit RKAccordionTable (QWidget *parent);
	~RKAccordionTable ();

	QWidget *editorWidget () const { return editor_widget; };

	void setModel (QAbstractItemModel *model) override;
	void setShowAddRemoveButtons (bool show);

	QSize sizeHint () const override;                                                  // reimplemented to assure a proper size for the content
public Q_SLOTS:
	void rowExpanded (QModelIndex row);
	void rowClicked (QModelIndex row);
	void updateWidget ();
	void removeClicked ();
	void activateRow (int row);
Q_SIGNALS:
	void activatedRow(int row);
	void addRow (int where);
	void removeRow (int which);
protected:
	void resizeEvent (QResizeEvent* event) override;                                          // reimplemented to make the current content widget stretch / shrink
	void currentChanged (const QModelIndex& current, const QModelIndex& previous) override;
	void mousePressEvent (QMouseEvent* event) override;
	void drawRow (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
private:
	QSize sizeHintWithoutEditor () const;
	int rowOfButton (QObject *button) const;
	bool show_add_remove_buttons;
	QWidget *editor_widget;
	QWidget *editor_widget_container;
	RKAccordionDummyModel *pmodel;
	bool handling_a_click;
};

#endif
