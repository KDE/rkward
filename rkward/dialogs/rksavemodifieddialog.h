/*
rksavemodifieddialog - This file is part of the RKWard project. Created: Wed Jul 12 2017
SPDX-FileCopyrightText: 2017 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSAVEMODIFIEDDIALOG_H
#define RKSAVEMODIFIEDDIALOG_H

#include <QDialog>
#include <QList>
#include <QMap>
#include <QPointer>

#include "../windows/rkmdiwindow.h"

class QTreeWidgetItem;

class RKSaveModifiedDialog : public QDialog {
	Q_OBJECT
public:
/** Provide a combined dialog to ask for saving and modified documents among the given list of windows. Call before closing the app, merging workspaces, etc.
 *  @param windows List of windows to ask about
 *  @param project Whether to also ask about saving project related resources: Outputs, Workplace, Workspace.
 *  @returns true, if the user chose to proceed (with or without saving modications), _and_ saving was successful; false, if the user cancelled the operation, or some saves failed. */
	static bool askSaveModified (QWidget* parent, QList<RKMDIWindow*> windows, bool project);
private:
	RKSaveModifiedDialog (QWidget* parent, QList<RKMDIWindow*> modified_windows, bool project);
	virtual ~RKSaveModifiedDialog ();
	QMap<QTreeWidgetItem *, QPointer<RKMDIWindow>> window_checklist;
	QTreeWidgetItem *save_project_check;
	QMap<QTreeWidgetItem *, QString> outputdir_checklist;
private Q_SLOTS:
	void saveWorkplaceChanged ();
	void saveSelected ();
};

#endif
