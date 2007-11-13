/***************************************************************************
                          robjectviewer  -  description
                             -------------------
    begin                : Tue Aug 24 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
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
#include "core/rkmodificationtracker.h"
#include "windows/rkmdiwindow.h"

class RObject;
class QTextEdit;
class QCloseEvent;
class QLabel;
class QPushButton;

/**
An extremely simple object viewer. You pass it an object in the constructor. It will extract some information and display that as text, then call R "print (object)" and display the result. You don't need to store a pointer. The window will self-destruct when closed.

@author Thomas Friedrichsmeier
*/
class RObjectViewer : public RKMDIWindow, public RCommandReceiver, public RObjectListener {
Q_OBJECT
public:
	~RObjectViewer ();

	RObject *object () { return _object; };
public slots:
	void cancel ();
	void update ();
	void toggleSummary ();
	void togglePrint ();
protected:
	friend class RKWorkplace;
	RObjectViewer (QWidget *parent, RObject *object);

	void rCommandDone (RCommand *command);
	void closeEvent (QCloseEvent *e);
	void objectRemoved (RObject *object);
private:
	QLabel *status_label;
	QLabel *description_label;
	QPushButton *update_button;
	QPushButton *cancel_button;
	QPushButton *toggle_summary_button;
	QPushButton *toggle_print_button;
	QTextEdit *print_area;
	QTextEdit *summary_area;
	QString caption;

	RObject *_object;
};

#endif
