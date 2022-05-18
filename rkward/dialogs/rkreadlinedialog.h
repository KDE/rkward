/*
rkreadlinedialog - This file is part of the RKWard project. Created: Fri Sep 15 2006
SPDX-FileCopyrightText: 2006-2009 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKREADLINEDIALOG_H
#define RKREADLINEDIALOG_H

#include <QDialog>

class QLineEdit;
class QWidget;
class RCommand;

/** This class represent a dialog asking for a line of input. It is used, when the backend calls readline().
This dialog displays the question asked, the output context (as often times the question is not meaningful without that context), and a QLineEdit to get the user response.

@author Thomas Friedrichsmeier
*/
class RKReadLineDialog : public QDialog {
public:
	/** Construct and run modal RKReadLineDialog.
	@param parent QWidget to center the modal dialog on (0 for application)
	@param prompt The question
	@param command The command the readline() has arisen from. This is used to get the relevant output context
	@param result The answer to the question (the line read) will be stored in this string
	@returns true if ok was pressed (or the dialog was closed), false if cancel was pressed (i.e. the command should be cancelled) */
	static bool readLine (QWidget *parent, const QString &caption, const QString &prompt, RCommand *command, QString *result);
protected:
	/** ctor. Use the static readLine() instead. */
	RKReadLineDialog (QWidget *parent, const QString &caption, const QString &prompt, RCommand *command);
	/** destructor */
	~RKReadLineDialog ();
private:
	QLineEdit *input;
	static QByteArray stored_geom;
};

#endif
