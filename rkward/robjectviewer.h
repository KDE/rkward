/***************************************************************************
                          robjectviewer  -  description
                             -------------------
    begin                : Tue Aug 24 2004
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
#ifndef ROBJECTVIEWER_H
#define ROBJECTVIEWER_H

#include <qwidget.h>
#include <qstring.h>

#include "rbackend/rcommandreceiver.h"

class RObject;
class QTextEdit;
class QCloseEvent;

/**
An extremely simple object viewer. You pass it an object in the constructor. It will extract some information and display that as text, then call R "print (object)" and display the result. You don't need to store a pointer. The window will self-destruct when closed.

@author Thomas Friedrichsmeier
*/
class RObjectViewer : public QWidget, public RCommandReceiver {
Q_OBJECT
public:
    RObjectViewer(QWidget *parent, RObject *object);

    ~RObjectViewer();
protected:
	void rCommandDone (RCommand *command);
	void closeEvent (QCloseEvent *e);
private:
	QTextEdit *view_area;
	bool waiting;
	bool destruct;
	QString caption;
};

#endif
