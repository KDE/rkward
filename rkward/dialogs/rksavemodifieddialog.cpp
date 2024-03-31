/*
rksavemodifieddialog - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jul 12 2017
SPDX-FileCopyrightText: 2017 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rksavemodifieddialog.h"

#include <QTreeWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QLabel>
#include <QPointer>

#include <KLocalizedString>

#include "../windows/rkworkplace.h"
#include "../windows/rkhtmlwindow.h"
#include "../misc/rkoutputdirectory.h"
#include "../agents/rksaveagent.h"

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

QTreeWidgetItem* makeHeaderItem (const QString &label, QTreeWidget* tree) {
	RK_TRACE (APP);

	QTreeWidgetItem* header = new QTreeWidgetItem (QStringList (label));
	header->setFirstColumnSpanned (true);
	header->setFlags (Qt::ItemIsEnabled);
	QFont f = tree->font ();
	f.setBold (true);
	header->setFont (0, f);
	tree->addTopLevelItem (header);
	header->setExpanded (true);

	return header;
}

RKSaveModifiedDialog::RKSaveModifiedDialog (QWidget* parent, QList<RKMDIWindow*> modified_wins, bool project) : QDialog (parent) {
	RK_TRACE (APP);

	setWindowTitle (i18n ("Save modified"));

	QVBoxLayout* v_layout = new QVBoxLayout (this);
	QLabel *label = new QLabel (i18n ("The following items have been modified. Do you want to save them before closing?"));
	v_layout->addWidget (label);

	QTreeWidget *tree = new QTreeWidget ();
	v_layout->addWidget (tree);

	save_project_check = nullptr;
	tree->header ()->hide ();

	if (project) {
		QTreeWidgetItem *header = makeHeaderItem(i18n("R Workspace (Data and Functions)"), tree);
		QString url = RKWorkplace::mainWorkplace()->workspaceURL().toDisplayString();
		if (url.isEmpty()) {
			url = i18n("[Not saved]");
		}
		save_project_check = new QTreeWidgetItem(QStringList(url));
		header->addChild(save_project_check);
		save_project_check->setCheckState(0, Qt::Checked);

		auto modified_outputs = RKOutputDirectory::modifiedOutputDirectories();
		if (!modified_outputs.isEmpty()) {
			QTreeWidgetItem *header = makeHeaderItem(i18n("Output files"), tree);
			for (int i = 0; i < modified_outputs.size(); ++i) {
				QTreeWidgetItem *item = new QTreeWidgetItem();
				item->setText(0, modified_outputs[i]->caption());
				item->setFirstColumnSpanned(true);
				header->addChild(item);
				item->setCheckState(0, Qt::Checked);
				outputdir_checklist.insert(item, modified_outputs[i]->getId());
			}
		}
	}
	if (!modified_wins.isEmpty ()) {
		QTreeWidgetItem* header = makeHeaderItem (i18n ("Scripts"), tree);
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

#include <KMessageBox>
void RKSaveModifiedDialog::saveSelected() {
	RK_TRACE (APP);

	bool all_ok = true;
	for (QMap<QTreeWidgetItem *, QPointer<RKMDIWindow>>::const_iterator it = window_checklist.constBegin (); it != window_checklist.constEnd (); ++it) {
		if (it.key ()->checkState (0) != Qt::Checked) continue;
		if (!it.value ()) continue;
		if (!it.value ()->save ()) all_ok = false; // but we proceed with the others
	}

	for (auto it = outputdir_checklist.constBegin(); it != outputdir_checklist.constEnd(); ++it) {
		if (it.key ()->checkState (0) != Qt::Checked) continue;
		RKOutputDirectory *dir = RKOutputDirectory::findOutputById(it.value());
		if (dir) {
			if (dir->save(dir->filename()).failed()) all_ok = false;
		}
		else RK_ASSERT(dir);
	}

	// Save workspace (+ workplace!) last, as some urls may still have changed, above.
	if (save_project_check && save_project_check->checkState(0) == Qt::Checked) {
		if (!RKSaveAgent::saveWorkspace()) all_ok = false;
	}

	if (all_ok) accept();
	else reject();
}
