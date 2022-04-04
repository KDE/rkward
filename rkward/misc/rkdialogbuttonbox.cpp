/*
rkdialogbuttonbox - This file is part of RKWard (https://rkward.kde.org). Created: Sat Feb 13 2016
SPDX-FileCopyrightText: 2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
