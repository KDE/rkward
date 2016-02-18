/***************************************************************************
                          rkselectlistdialog  -  description
                             -------------------
    begin                : Thu Mar 18 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rkselectlistdialog.h"

#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollBar>

#include <KLocalizedString>

#include "../misc/rkdialogbuttonbox.h"

#include "../debug.h"

/** A QListWidget with a sane sizeHint() */
class RKSelectListDialogListWidget : public QListWidget {
public:
	explicit RKSelectListDialogListWidget (QWidget* parent) : QListWidget (parent) {};
	QSize sizeHint () const override {
		return (QSize (qMax (50, sizeHintForColumn (0) + verticalScrollBar ()->width ()), qMax (50, sizeHintForRow (0)*(count ()+1))));
	}
};

RKSelectListDialog::RKSelectListDialog (QWidget *parent, const QString &caption, const QStringList& choices, const QStringList& preselected, bool multiple) : QDialog (parent) {
	RK_TRACE (DIALOGS);

	setModal (true);
	setWindowTitle (caption);

	QVBoxLayout *layout = new QVBoxLayout (this);

	if (multiple) layout->addWidget (new QLabel (i18n ("<b>Select one or more:</b>"), this));
	else layout->addWidget (new QLabel (i18n ("<b>Select one:</b>"), this));
	layout->addWidget (new QLabel (caption, this));

	input = new RKSelectListDialogListWidget (this);
	input->addItems (choices);
	if (multiple) input->setSelectionMode (QAbstractItemView::MultiSelection);
	else input->setSelectionMode (QAbstractItemView::SingleSelection);
	for (int i = 0; i < preselected.count (); ++i) {
		int pos = choices.indexOf (preselected[i]);
		if (pos >= 0) input->item (pos)->setSelected (true);
	}
	layout->addWidget (input);

	buttons = new RKDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	layout->addWidget (buttons);

	connect (input, &QListWidget::itemSelectionChanged, this, &RKSelectListDialog::updateState);
	updateState ();
}

RKSelectListDialog::~RKSelectListDialog () {
	RK_TRACE (DIALOGS);
}

void RKSelectListDialog::updateState () {
	RK_TRACE (DIALOGS);

	// TODO is there no QListWidget::hasSelection()?
	buttons->button (QDialogButtonBox::Ok)->setEnabled (!input->selectedItems ().isEmpty ());
}

//static
QStringList RKSelectListDialog::doSelect (QWidget *parent, const QString &caption, const QStringList& choices, const QStringList& preselected, bool multiple) {
	RK_TRACE (DIALOGS);

	RKSelectListDialog *dialog = new RKSelectListDialog (parent, caption, choices, preselected, multiple);
	int res = dialog->exec ();
	if (res != QDialog::Accepted) return QStringList ();

	QStringList list;
	QList<QListWidgetItem*> selected = dialog->input->selectedItems ();
	for (int i = 0; i < selected.count (); ++i) {
		list.append (selected[i]->text ());
	}

	return (list);
}

