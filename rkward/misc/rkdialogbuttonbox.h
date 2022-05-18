/*
rkdialogbuttonbox - This file is part of the RKWard project. Created: Sat Feb 13 2016
SPDX-FileCopyrightText: 2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKDIALOGBUTTONBOX_H
#define RKDIALOGBUTTONBOX_H

#include <QDialogButtonBox>

/** Simple helper class to help keep sanity: Just like QDialogButtonBox, but it "Ok" and / or "Cancel" buttons are requested in the constructor, these are connected to the
 *  dialog's accept() and reject()-slots, automatically. Also, adds the Ctrl+Enter shortcut to the ok button, as suggested by KDE */
class RKDialogButtonBox : public QDialogButtonBox {
public:
	RKDialogButtonBox (QDialogButtonBox::StandardButtons buttons, QDialog *parent);
};

#endif
