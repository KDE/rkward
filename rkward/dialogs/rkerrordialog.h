/*
rkerrordialog - This file is part of the RKWard project. Created: Thu Apr 25 2013
SPDX-FileCopyrightText: 2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
 
#ifndef RKERRORDIALOG_H
#define RKERRORDIALOG_H

#include <QString>

class QWidget;

namespace RKErrorDialog {
/** Displays an error message with a button "Report bug" */
	void reportableErrorMessage (QWidget *parent_widget, const QString &user_message, const QString &details, const QString &caption, const QString &message_code);
/** Open "Report bug" dialog */
	void reportBug (QWidget *parent_widget = nullptr, const QString &message_info=QString ());
};

#endif
