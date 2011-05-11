/***************************************************************************
                          robjectviewer  -  description
                             -------------------
    begin                : Tue Aug 24 2004
    copyright            : (C) 2004, 2007, 2009 by Thomas Friedrichsmeier
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
#include "robjectviewer.h"

#include <QLabel>
#include <QTextEdit>
#include <QFont>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>

#include <klocale.h>
#include <kglobalsettings.h>

#include "rbackend/rinterface.h"
#include "rkglobals.h"
#include "core/robject.h"
#include "misc/rkdummypart.h"

#include "debug.h"

RObjectViewer::RObjectViewer (QWidget *parent, RObject *object, ViewerPage initial_page) : RKMDIWindow (parent, RKMDIWindow::ObjectWindow, false), RObjectListener (RObjectListener::ObjectView) {
	RK_TRACE (APP);
	RK_ASSERT (object);
	_object = object;

	addNotificationType (RObjectListener::ObjectRemoved);
	addNotificationType (RObjectListener::MetaChanged);
	addNotificationType (RObjectListener::DataChanged);
	listenForObject (_object);

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);

	description_label = new QLabel (this);
	description_label->setWordWrap (true);
	layout->addWidget (description_label);
	status_label = new QLabel (this);
	status_label->hide ();
	layout->addWidget (status_label);

	tabs = new QTabWidget (this);
	tabs->insertTab (SummaryPage, summary_widget = new RObjectSummaryWidget (tabs, object), i18n ("summary (x)"));
	tabs->insertTab (PrintPage, print_widget = new RObjectPrintWidget (tabs, object), i18n ("print (x)"));
	tabs->insertTab (StructurePage, structure_widget = new RObjectStructureWidget (tabs, object), i18n ("str (x)"));
	layout->addWidget (tabs);

	tabs->setFocusPolicy (Qt::StrongFocus);
	setPart (new RKDummyPart (this, tabs));
	initializeActivationSignals ();

	tabs->setCurrentIndex (initial_page);
	currentTabChanged (initial_page);
	connect (tabs, SIGNAL (currentChanged(int)), this, SLOT (currentTabChanged (int)));

	initDescription (false);
}

RObjectViewer::~RObjectViewer () {
	RK_TRACE (APP);

	if (_object) stopListenForObject (_object);
}

void RObjectViewer::objectRemoved (RObject *object) {
	RK_TRACE (APP);

	if (object == _object) {
		summary_widget->objectKilled ();
		print_widget->objectKilled ();
		structure_widget->objectKilled ();

		QString reason = i18n ("<b>Object was deleted!</b>");
		summary_widget->invalidate (reason);
		print_widget->invalidate (reason);
		structure_widget->invalidate (reason);

		QPalette palette = status_label->palette ();
		palette.setColor (status_label->foregroundRole (), Qt::red);
		status_label->setPalette (palette);
		status_label->setText (reason);
		status_label->show ();

		stopListenForObject (_object);
		_object = 0;
	} else {
		RK_ASSERT (false);
	}
}

void RObjectViewer::objectMetaChanged (RObject* object) {
	RK_TRACE (APP);

	if (object == _object) {
		initDescription (true);
	} else {
		RK_ASSERT (false);
	}
}

void RObjectViewer::objectDataChanged (RObject* object, const RObject::ChangeSet*) {
	RK_TRACE (APP);

	if (object == _object) {
		initDescription (true);
	} else {
		RK_ASSERT (false);
	}
}

void RObjectViewer::initDescription (bool notify) {
	RK_TRACE (APP);

	if (!_object) return;

	setCaption (i18n("Object Viewer: %1", _object->getShortName ()));
	// make the description use less height. Trying to specify <nobr>s, here, is no good idea (see https://sourceforge.net/tracker/?func=detail&atid=459007&aid=2859182&group_id=50231)
	description_label->setText (_object->getObjectDescription ().replace ("<br>", "&nbsp; &nbsp; "));
	if (notify) {
		QString reason = i18n ("The object was changed. You may want to click \"Update\"");
		summary_widget->invalidate (reason);
		print_widget->invalidate (reason);
		structure_widget->invalidate (reason);
	}
}

void RObjectViewer::currentTabChanged (int new_current) {
	RK_TRACE (APP);

	if (new_current == SummaryPage) {
		summary_widget->initialize ();
	} else if (new_current == PrintPage) {
		print_widget->initialize ();
	} else if (new_current == StructurePage) {
		structure_widget->initialize ();
	} else {
		RK_ASSERT (false);
	}
}

///////////////// RObjectViewerWidget /////////////////////

RObjectViewerWidget::RObjectViewerWidget (QWidget* parent, RObject* object) : QWidget (parent), RCommandReceiver () {
	RK_TRACE (APP);

	_object = object;
	QVBoxLayout* main_layout = new QVBoxLayout (this);
	main_layout->setContentsMargins (0, 0, 0, 0);
	QHBoxLayout* status_layout = new QHBoxLayout ();
	main_layout->addLayout (status_layout);

	status_label = new QLabel (this);
	status_layout->addWidget (status_label);

	status_layout->addStretch ();

	update_button = new QPushButton (i18n ("Update"), this);
	connect (update_button, SIGNAL (clicked ()), this, SLOT (update ()));
	status_layout->addWidget (update_button);

	cancel_button = new QPushButton (i18n ("Cancel"), this);
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (cancel ()));
	status_layout->addWidget (cancel_button);

	area = new QTextEdit (this);
	area->setReadOnly (true);
	area->setLineWrapMode (QTextEdit::NoWrap);
	main_layout->addWidget (area);

	initialized = false;
}

RObjectViewerWidget::~RObjectViewerWidget () {
	RK_TRACE (APP);
}

void RObjectViewerWidget::invalidate (const QString& reason) {
	RK_TRACE (APP);

	if (outstanding_commands.isEmpty ()) {
		QPalette palette = status_label->palette ();
		palette.setColor (status_label->foregroundRole (), Qt::red);
		status_label->setPalette (palette);

		status_label->setText (reason);
		update_button->setEnabled (_object != 0);
		cancel_button->setEnabled (false);
	}
}

void RObjectViewerWidget::initialize () {
	RK_TRACE (APP);

	if (initialized) return;
	update ();
	initialized = true;
}

void RObjectViewerWidget::update () {
	RK_TRACE (APP);

	RK_ASSERT (outstanding_commands.isEmpty ());
	RK_ASSERT (_object);

	setText (i18n ("Fetching information. Please wait."));

	update_button->setEnabled (false);
	cancel_button->setEnabled (true);
}

void RObjectViewerWidget::cancel () {
	RK_TRACE (APP);

	cancelOutstandingCommands ();
	setText (i18n ("Click \"Update\" to fetch information"));
	cancel_button->setEnabled (false);
	update_button->setEnabled (_object != 0);
}

void RObjectViewerWidget::setText (const QString& text) {
	RK_TRACE (APP);

	area->setPlainText (QString ());
	QFont font = KGlobalSettings::fixedFont ();
	area->setCurrentFont (font);

	area->insertPlainText (text);
}

void RObjectViewerWidget::ready () {
	RK_TRACE (APP);

	QPalette palette = status_label->palette ();
	palette.setColor (status_label->foregroundRole (), Qt::black);
	status_label->setPalette (palette);
	status_label->setText (i18n ("Ready"));
	cancel_button->setEnabled (false);
	update_button->setEnabled (_object != 0);
}

void RObjectViewerWidget::rCommandDone (RCommand* command) {
	RK_TRACE (APP);

	if (command->wasCanceled ()) {
		cancel ();
	} else {
		setText (command->fullOutput ());
		ready ();
	}
}

////////////////// summary widget /////////////////

void RObjectSummaryWidget::update () {
	RK_TRACE (APP);

	if (!_object) {
		RK_ASSERT (false);
		return;
	}

	RObjectViewerWidget::update ();

	RCommand *command = new RCommand ("print(summary(" + _object->getFullName () + "))", RCommand::App, QString (), this);
	RKGlobals::rInterface ()->issueCommand (command, 0);
}

////////////////// print widget /////////////////

void RObjectPrintWidget::update () {
	RK_TRACE (APP);

	if (!_object) {
		RK_ASSERT (false);
		return;
	}

	RObjectViewerWidget::update ();

	// make sure to print as wide as possible
	RCommand* command = new RCommand ("local({\n"
	                                  "\trk.temp.width.save <- getOption(\"width\")\n"
	                                  "\toptions(width=10000)\n"
	                                  "\ton.exit(options(width=rk.temp.width.save))\n"
	                                  "\tprint(" + _object->getFullName () + ")\n"
	                                  "})", RCommand::App, QString (), this);
	RKGlobals::rInterface ()->issueCommand (command, 0);
}

////////////////// structure widget /////////////////

void RObjectStructureWidget::update () {
	RK_TRACE (APP);

	if (!_object) {
		RK_ASSERT (false);
		return;
	}

	RObjectViewerWidget::update ();

	RCommand *command = new RCommand ("str(" + _object->getFullName () + ")", RCommand::App, QString (), this);
	RKGlobals::rInterface ()->issueCommand (command, 0);
}

#include "robjectviewer.moc"
