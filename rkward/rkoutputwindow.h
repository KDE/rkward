/***************************************************************************
                          rkoutputwindow  -  description
                             -------------------
    begin                : Tue Jul 27 2004
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
#ifndef RKOUTPUTWINDOW_H
#define RKOUTPUTWINDOW_H

#include "misc/rktogglewidget.h"

class QTextBrowser;
class KHTMLPart;

/**
The Window where RK displays the (formatted) output

@author Thomas Friedrichsmeier
*/
class RKOutputWindow : public RKToggleWidget {
Q_OBJECT
public:
	RKOutputWindow(QWidget *parent = 0);

	~RKOutputWindow();

	void checkNewInput ();
public slots:
	void configureOutput ();
	void clearOutput ();
	void refreshOutput ();
private:
	KHTMLPart *khtmlpart;
};

#endif
