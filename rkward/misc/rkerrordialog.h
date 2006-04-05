/***************************************************************************
                          rkerrordialog  -  description
                             -------------------
    begin                : Fri Jul 30 2004
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
#ifndef RKERRORDIALOG_H
#define RKERRORDIALOG_H

#include <qobject.h>

#include <qstring.h>

class QDialog;
class QTextEdit;

/**
This class can be used to display errors (non-blocking). The dialog itself is only generated when there really are errors to display, otherwise the class is just a few QStrings waiting to be used. It's essentially useful in the case where error-messages may come in one by one: You simply call newError (...) - if the dialog is already being shown, the new error-messages simply get appended, so as not to clutter the desktop with loads of error-dialogs. if the dialog is not yet/no longer shown, it will be created/shown.

@author Thomas Friedrichsmeier
*/
class RKErrorDialog : public QObject {
	Q_OBJECT
public:
	RKErrorDialog (const QString &text, const QString &caption, bool modal=false);

	virtual ~RKErrorDialog ();

	void newError (const QString &error);
/** usually you will call newError instead. However, if in case of an error, you also want to show the regular output, use this function to add output. The output is added to the internal error_log, but the dialog is not shown until you call newError (). */
	void newOutput (const QString &output);
/** you don't need this, unless you feed regular output to the dialog using newOutput. */
	void resetOutput ();
public slots:
// so we can easily keep track of whether the dialog is alive or not
	void dialogDestroyed ();
private:
	void createDialog ();

	QDialog *dialog;
	
	QTextEdit *error_log;
	
	bool show_modal;
	
	QString text;
	QString caption;
	QString stored_output;
};

#include "../rbackend/rcommandreceiver.h"
/** A subclass of RKErrorDialog, that has special helper functions to deal with RCommands. It can be set as the receiver of RCommands, or can be fed RCommands, and will extract any errors associated with those commands. If you use the RKRErrorDialog as a command receiver, be sure to always call deleteThis () instead of delete, so pending commands don't go to a destroyed object. */
class RKRErrorDialog : public RKErrorDialog, public RCommandReceiver {
public:
/** constructor. See RKErrorDialog::RKErrorDialog () for details */
	RKRErrorDialog (const QString &text, const QString &caption, bool modal=false);
/** constructor. See RKErrorDialog::~RKErrorDialog () for details */
	~RKRErrorDialog ();

/** Use this mechanism to feed RCommands to the RKRErrorDialog manually. The alternative is to simply specify the error-dialog as the receiver of your RCommands (but then you'll never see tehm first). Errors contained in the command will be extracted. */
	void addRCommand (RCommand *command);
protected:
	void rCommandDone (RCommand *command);
};

#endif
