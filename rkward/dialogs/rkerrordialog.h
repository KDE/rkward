/***************************************************************************
                          rkerrordialog  -  description
                             -------------------
    begin                : Thu Apr 25 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier
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

#include <QString>

class QWidget;

namespace RKErrorDialog {
/** Displays an error message with a button "Report bug" */
	void reportableErrorMessage (QWidget *parent_widget, const QString &user_message, const QString &details, const QString &caption, const QString &message_code);
/** Open "Report bug" dialog */
	void reportBug (QWidget *parent_widget = 0, const QString &message_info=QString ());
};

#endif
