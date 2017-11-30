/***************************************************************************
                          rksavemodifieddialog  -  description
                             -------------------
    begin                : Wed Jul 12 2017
    copyright            : (C) 2017 by Thomas Friedrichsmeier
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

#include "rksavemodifieddialog.h"

#include <QTreeWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QLabel>
#include <QPointer>

#include <klocale.h>

#include "../debug.h"

bool RKSaveModifiedDialog::askSaveModified (QWidget* parent, QList <RKMDIWindow*> windows, bool project) {
	RK_TRACE (APP);

	QList<RKMDIWindow*> modified_wins;
	for (int i = 0; i < windows.size (); ++i) {
		if (windows[i]->isModified ()) {
			modified_wins.append (windows[i]);
		}
	}

	if (project || !modified_wins.isEmpty ()) {
		RKSaveModifiedDialog dialog (parent, modified_wins, project);
		dialog.exec ();
		if (dialog.result () == QDialog::Rejected) return false;
	}

	return true;
}

RKSaveModifiedDialog::RKSaveModifiedDialog (QWidget* parent, QList<RKMDIWindow*> modified_wins, bool project) : QDialog (parent) {
	RK_TRACE (APP);

	setWindowTitle (i18n ("Save modified"));

	QVBoxLayout* v_layout = new QVBoxLayout (this);
	QLabel *label = new QLabel (i18n ("The following items have been modified. Do you want to save them before closing?"));
	v_layout->addWidget (label);

	QTreeWidget *tree = new QTreeWidget ();
	v_layout->addWidget (tree);

	tree->header ()->hide ();
	if (project) {
	}
	if (!modified_wins.isEmpty ()) {
		QTreeWidgetItem* header = new QTreeWidgetItem (QStringList (i18n ("Scripts")));
		header->setFirstColumnSpanned (true);
		header->setFlags (Qt::ItemIsEnabled);
		QFont f = tree->font ();
		f.setBold (true);
		header->setFont (0, f);
		tree->addTopLevelItem (header);
		header->setExpanded (true);
		for (int i = 0; i < modified_wins.size (); ++i) {
			QTreeWidgetItem *item = new QTreeWidgetItem (QStringList (modified_wins[i]->fullCaption ()));
			item->setFirstColumnSpanned (true);
			header->addChild (item);
			item->setCheckState (0, Qt::Checked);
			window_checklist.insert (item, QPointer<RKMDIWindow> (modified_wins[i]));
		}
	}

	QDialogButtonBox *buttonbox = new QDialogButtonBox (this);
	v_layout->addWidget (buttonbox);

	buttonbox->setStandardButtons (QDialogButtonBox::Save | QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
	buttonbox->button (QDialogButtonBox::Save)->setDefault (true);
	buttonbox->button (QDialogButtonBox::Save)->setText (i18nc ("Save the selected items", "Save selected"));
	buttonbox->button (QDialogButtonBox::Discard)->setText (i18n ("Discard all"));
	buttonbox->button (QDialogButtonBox::Cancel)->setText (i18n ("Do not close"));

	connect (buttonbox->button (QDialogButtonBox::Save), &QPushButton::clicked, this, &RKSaveModifiedDialog::saveSelected);
	connect (buttonbox->button (QDialogButtonBox::Discard), &QPushButton::clicked, this, &QDialog::accept);
	connect (buttonbox->button (QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);

	setModal (true);
}

RKSaveModifiedDialog::~RKSaveModifiedDialog () {
	RK_TRACE (APP);
}

void RKSaveModifiedDialog::saveWorkplaceChanged () {
	RK_TRACE (APP);
// TODO: enable / disable "save with workplace" option for Output windows
}

void RKSaveModifiedDialog::saveSelected () {
	RK_TRACE (APP);

	bool all_ok = true;
	for (QMap<QTreeWidgetItem *, QPointer<RKMDIWindow>>::const_iterator it = window_checklist.constBegin (); it != window_checklist.constEnd (); ++it) {
		if (it.key ()->checkState (0) != Qt::Checked) continue;
		if (!it.value ()) continue;
		if (!it.value ()->save ()) all_ok = false; // but we proceed with the others
	}
	if (all_ok) accept ();
	else reject ();
}
