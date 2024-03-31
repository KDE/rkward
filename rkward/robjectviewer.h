/*
robjectviewer - This file is part of the RKWard project. Created: Tue Aug 24 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ROBJECTVIEWER_H
#define ROBJECTVIEWER_H

#include <qwidget.h>
#include <qstring.h>

#include "core/rkmodificationtracker.h"
#include "windows/rkmdiwindow.h"

class RCommand;
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
class RObjectViewer : public RKMDIWindow, public RObjectListener {
Q_OBJECT
public:
	~RObjectViewer ();

	RObject *object () { return _object; };

	enum ViewerPage {
		SummaryPage = 0,
		PrintPage = 1,
		StructurePage = 2
	};
public Q_SLOTS:
	void currentTabChanged (int new_current);
protected:
	friend class RKWorkplace;
	RObjectViewer (QWidget *parent, RObject *object, ViewerPage initial_page = SummaryPage);

	void objectRemoved (RObject *object) override;
	void objectMetaChanged (RObject *object) override;
	void objectDataChanged (RObject *object, const RObject::ChangeSet*) override;
private:
	void initDescription (bool notify);

	QLabel *status_label;
	QLabel *description_label;
	QTabWidget* tabs;
	RObjectViewerWidget* summary_widget;
	RObjectViewerWidget* print_widget;
	RObjectViewerWidget* structure_widget;

	RObject *_object;
};

/** Since the two widgets in the RObjectViewer are largely similar, this is the common base for them. The base class itself is *not* useful.

@author Thomas Friedrichsmeier */
class RObjectViewerWidget : public QWidget {
Q_OBJECT
protected:
	RObjectViewerWidget (QWidget* parent, RObject* object);
	virtual ~RObjectViewerWidget ();
public:
	void objectKilled() { _object = nullptr; };

	void invalidate (const QString& reason);
	void initialize ();
	void setText (const QString& text);
public Q_SLOTS:
	virtual void update ();
protected:
	virtual RCommand* makeCommand() = 0;

	QLabel* status_label;

	QPushButton *update_button;
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
	RCommand* makeCommand() override;
};

/** Represents the "print" area in an RObjectViewer */
class RObjectPrintWidget : public RObjectViewerWidget {
public:
	RObjectPrintWidget (QWidget* parent, RObject* object) : RObjectViewerWidget (parent, object) {}
	~RObjectPrintWidget () {};

	/** reimplemented from RObjectViewerWidget to call "print" */
	RCommand* makeCommand() override;
};

/** Represents the "str" area in an RObjectViewer */
class RObjectStructureWidget : public RObjectViewerWidget {
public:
	RObjectStructureWidget (QWidget* parent, RObject* object) : RObjectViewerWidget (parent, object) {};
	~RObjectStructureWidget () {};

	/** reimplemented from RObjectViewerWidget to call "str" */
	RCommand* makeCommand() override;
};

#endif
