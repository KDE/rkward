/*
rkselectlistdialog - This file is part of RKWard (https://rkward.kde.org). Created: Thu Mar 18 2010
SPDX-FileCopyrightText: 2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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

