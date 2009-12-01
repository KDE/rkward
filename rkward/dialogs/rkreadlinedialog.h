/***************************************************************************
                          rkreadlinedialog  -  description
                             -------------------
    begin                : Fri Sep 15 2006
    copyright            : (C) 2006, 2007, 2009 by Thomas Friedrichsmeier
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

#ifndef RKREADLINEDIALOG_H
#define RKREADLINEDIALOG_H

#include <kdialog.h>

class QLineEdit;
class QWidget;
class RCommand;

/** This class represent a dialog asking for a line of input. It is used, when the backend calls readline().
This dialog displays the question asked, the output context (as often times the question is not meaningful without that context), and a QLineEdit to get the user response.

@author Thomas Friedrichsmeier
*/
class RKReadLineDialog : public KDialog {
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
