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
