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

    ~RKErrorDialog ();
	
	void newError (const QString &error);
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
};

#endif
