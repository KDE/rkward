/***************************************************************************
                          rkcanceldialog  -  description
                             -------------------
    begin                : Wed Sep 8 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RKCANCELDIALOG_H
#define RKCANCELDIALOG_H

#include <qdialog.h>

class RCommand;

/**
A simple modal dialog which can be shown during a lengthy operation (which has to be carried out synchronously) and allows the user to cancel said operation. Connect a signal to the complete ()-slot to signal the operation is done. Returns QDialog::Accepted if succesful or QDialog::rejected, if the user pressed cancel before the operation was completed.
If you specify an associated_commands in the constructor, RKCanceldialog, if cancelled, will also take care of cancelling that RCommand.
Use the static member showCancelDialog () to make it a single-liner.
@author Thomas Friedrichsmeier
*/
class RKCancelDialog : public QDialog {
Q_OBJECT
public:
	RKCancelDialog (const QString &caption, const QString &text, QWidget *parent, RCommand *associated_command=0);
	~RKCancelDialog();
	
	// returns either QDialog::Accepted or QDialog::Rejected
	static int showCancelDialog (const QString &caption, const QString &text, QWidget *parent, QObject *done_sender, const char *done_signal, RCommand *associated_command=0);
public slots:
	void completed ();
protected:
	void closeEvent (QCloseEvent *e);
private:
	RCommand *command;
protected slots:
	virtual void reject ();
};

#endif
