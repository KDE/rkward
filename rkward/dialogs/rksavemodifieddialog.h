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

#ifndef RKSAVEMODIFIEDDIALOG_H
#define RKSAVEMODIFIEDDIALOG_H

#include <QDialog>
#include <QList>

#include "../windows/rkmdiwindow.h"

class QTreeWidget;

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
	QTreeWidget *tree;
private slots:
	void saveWorkplaceChanged ();
	void saveSelected ();
};

#endif
