/*
rkselectlistdialog - This file is part of the RKWard project. Created: Thu Mar 18 2010
SPDX-FileCopyrightText: 2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSELECTLISTDIALOG_H
#define RKSELECTLISTDIALOG_H

#include <QDialog>

class QListWidget;
class QDialogButtonBox;

/** This class represent a dialog asking for a choice among several options. It is used, when the backend calls select.list() / menu().

@author Thomas Friedrichsmeier
*/
class RKSelectListDialog : public QDialog {
	Q_OBJECT
public:
	/** Construct and run modal RKSelectListDialog.
	@param parent QWidget to center the modal dialog on (0 for application)
	@param caption Dialog title
	@param choices The list of available choices
	@param preselected The preselected choices (QStringList ()) for no preselects
	@param multiple Whether multiple items may be selected
	@returns The selected option(s). An empty list, if cancel was pressed. */
	static QStringList doSelect (QWidget *parent, const QString &caption, const QStringList& choices, const QStringList& preselected, bool multiple);
protected:
	/** ctor. Use the doSelect() instead. */
	RKSelectListDialog (QWidget *parent, const QString &caption, const QStringList& choices, const QStringList& preselected, bool multiple);
	/** destructor */
	~RKSelectListDialog ();
private Q_SLOTS:
	void updateState ();
private:
	QListWidget *input;
	QDialogButtonBox *buttons;
};

#endif
