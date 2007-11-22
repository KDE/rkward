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
class QTabWidget;
class QLabel;
class QPushButton;
class RObjectViewerWidget;

/**
A simple object viewer. You pass it an object in the constructor. It will extract some information and display that as text.

@author Thomas Friedrichsmeier
*/
class RObjectViewer : public RKMDIWindow, public RCommandReceiver, public RObjectListener {
Q_OBJECT
public:
	~RObjectViewer ();

	RObject *object () { return _object; };

	enum ViewerPage {
		SummaryPage = 0,
		PrintPage = 1
	};
public slots:
	void currentTabChanged (int new_current);
protected:
	friend class RKWorkplace;
	RObjectViewer (QWidget *parent, RObject *object, ViewerPage initial_page = SummaryPage);

	void objectRemoved (RObject *object);
private:
	QLabel *status_label;
	QLabel *description_label;
	QTabWidget* tabs;
	RObjectViewerWidget* summary_widget;
	RObjectViewerWidget* print_widget;

	RObject *_object;
};

/** Since the two widgets in the RObjectViewer are largely similar, this is the common base for them. The base class itself is *not* useful.

@author Thomas Friedrichsmeier */
class RObjectViewerWidget : public QWidget, public RCommandReceiver {
Q_OBJECT
protected:
	RObjectViewerWidget (QWidget* parent, RObject* object);
	virtual ~RObjectViewerWidget ();
public:
	void objectKilled () { _object = 0; };

	void invalidate (const QString& reason);
	void initialize ();
	void setText (const QString& text);
public slots:
	void cancel ();
	virtual void update ();
protected:
	void rCommandDone (RCommand *command);
	void ready ();

	QLabel* status_label;

	QPushButton *update_button;
	QPushButton *cancel_button;
	QTextEdit *area;

	bool initialized;

	RObject* _object;
};

/** Represents the "summary" area in an RObjectViewer */
class RObjectSummaryWidget : public RObjectViewerWidget {
public:
	RObjectSummaryWidget (QWidget* parent, RObject* object) : RObjectViewerWidget (parent, object) {};
	~RObjectSummaryWidget () {};

	/** reimplemented from RObjectViewerWidget to call "summary" */
	void update ();
};

/** Represents the "print" area in an RObjectViewer */
class RObjectPrintWidget : public RObjectViewerWidget {
public:
	RObjectPrintWidget (QWidget* parent, RObject* object) : RObjectViewerWidget (parent, object) {}
	~RObjectPrintWidget () {};

	/** reimplemented from RObjectViewerWidget to call "print" */
	void update ();
};

#endif
