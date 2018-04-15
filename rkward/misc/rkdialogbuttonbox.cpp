/***************************************************************************
                          rkdialogbuttonbox  -  description
                             -------------------
    begin                : Sat Feb 13 2016
    copyright            : (C) 2016 by Thomas Friedrichsmeier
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

#include "rkdialogbuttonbox.h"

#include <QPushButton>
#include <QDialog>

#include "../debug.h"

RKDialogButtonBox::RKDialogButtonBox (QDialogButtonBox::StandardButtons buttons, QDialog *parent) : QDialogButtonBox (buttons, parent) {
	RK_TRACE (MISC);
	if (buttons & QDialogButtonBox::Ok) {
		connect (button (QDialogButtonBox::Ok), &QPushButton::clicked, parent, &QDialog::accept);
		button (QDialogButtonBox::Ok)->setShortcut (Qt::CTRL | Qt::Key_Return);
	}
	if (buttons & QDialogButtonBox::Cancel) {
		connect (button (QDialogButtonBox::Cancel), &QPushButton::clicked, parent, &QDialog::reject);
	}
}
