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
#include "robjectviewer.h"

#include <qlayout.h>
#include <q3scrollview.h>
#include <q3vbox.h>
#include <q3hbox.h>
#include <qlabel.h>
#include <q3textedit.h>
#include <qfont.h>
#include <QPushButton>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <QCloseEvent>

#include <klocale.h>

#include "rbackend/rinterface.h"
#include "rkglobals.h"
#include "core/robject.h"
#include "core/rcontainerobject.h"
#include "misc/rkdummypart.h"
#include "core/rkmodificationtracker.h"

#include "debug.h"

#define SUMMARY_COMMAND 1
#define PRINT_COMMAND 2

RObjectViewer::RObjectViewer (QWidget *parent, RObject *object) : RKMDIWindow (parent, RKMDIWindow::ObjectWindow, false) {
	RK_TRACE (APP);
	RK_ASSERT (object);
	_object = object;

	Q3VBoxLayout *layout = new Q3VBoxLayout (this);
	Q3ScrollView *wrapper = new Q3ScrollView (this);
	wrapper->setResizePolicy (Q3ScrollView::AutoOneFit);
	Q3VBox *box = new Q3VBox (wrapper->viewport ());
	wrapper->addChild (box);
	layout->addWidget (wrapper);

	wrapper->setFocusPolicy (Qt::StrongFocus);
	setPart (new RKDummyPart (this, wrapper));
	initializeActivationSignals ();

	QFont font ("Courier");

	Q3HBox *toprow = new Q3HBox (box);
	description_label = new QLabel (toprow);
	Q3VBox *statusbox = new Q3VBox (toprow);
	statusbox->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	status_label = new QLabel (statusbox);
	update_button = new QPushButton (i18n ("Update"), statusbox);
	cancel_button = new QPushButton (i18n ("Cancel"), statusbox);
	connect (update_button, SIGNAL (clicked ()), this, SLOT (update ()));
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (cancel ()));

	Q3HBox *row = new Q3HBox (box);
	QLabel *label = new QLabel (i18n("\nResult of 'summary (%1)':\n", object->getFullName ()), row);
	row->setStretchFactor (label, 10);
	toggle_summary_button = new QPushButton (i18n ("Hide"), row);
	connect (toggle_summary_button, SIGNAL (clicked ()), this, SLOT (toggleSummary ()));

	summary_area = new Q3TextEdit (box);
	summary_area->setTextFormat (Qt::PlainText);
	summary_area->setReadOnly (true);
	summary_area->setCurrentFont (font);
	summary_area->setWordWrap (Q3TextEdit::NoWrap);

	row = new Q3HBox (box);
	label = new QLabel (i18n("\nResult of 'print (%1)':\n", object->getFullName ()), row);
	row->setStretchFactor (label, 10);
	toggle_print_button = new QPushButton (i18n ("Hide"), row);
	connect (toggle_print_button, SIGNAL (clicked ()), this, SLOT (togglePrint ()));

	print_area = new Q3TextEdit (box);
	print_area->setTextFormat (Qt::PlainText);
	print_area->setReadOnly (true);
	print_area->setCurrentFont (font);
	print_area->setWordWrap (Q3TextEdit::NoWrap);

	setCaption (i18n("Object Viewer: ") + object->getShortName ());
	//resize (minimumSizeHint ().expandedTo (QSize (640, 480)));
	update ();
	connect (RKGlobals::tracker (), SIGNAL (objectRemoved(RObject*)), this, SLOT (objectRemoved(RObject*)));
	show ();
}

RObjectViewer::~RObjectViewer () {
	RK_TRACE (APP);
}

void RObjectViewer::toggleSummary () {
	RK_TRACE (APP);

	summary_area->setShown (!summary_area->isShown ());
	if (summary_area->isShown ()) {
		toggle_summary_button->setText (i18n ("Hide"));
	} else {
		toggle_summary_button->setText (i18n ("Show"));
	}
}

void RObjectViewer::togglePrint () {
	RK_TRACE (APP);

	print_area->setShown (!print_area->isShown ());
	if (print_area->isShown ()) {
		toggle_print_button->setText (i18n ("Hide"));
	} else {
		toggle_print_button->setText (i18n ("Show"));
	}
}

void RObjectViewer::cancel () {
	RK_TRACE (APP);

	cancel_button->setEnabled (false);
	update_button->setEnabled (true);
	cancelOutstandingCommands ();
}

void RObjectViewer::update () {
	RK_TRACE (APP);

	status_label->setText (i18n ("Fetching information"));
	cancel_button->setEnabled (true);
	update_button->setEnabled (false);
	description_label->setText (_object->getObjectDescription ());

	summary_area->setText (QString::null);
	print_area->setText (QString::null);

	RCommand *command = new RCommand ("print(summary(" + _object->getFullName () + "))", RCommand::App, QString::null, this, SUMMARY_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);

	// make sure to print as wide as possible
	command = new RCommand ("local({\n"
	                                  "\trk.temp.width.save <- getOption(\"width\")\n"
	                                  "\toptions(width=10000)\n"
	                                  "\ton.exit(options(width=rk.temp.width.save))\n"
	                                  "\tprint(" + _object->getFullName () + ")\n"
	                                  "})", RCommand::App, QString::null, this, PRINT_COMMAND);
	RKGlobals::rInterface ()->issueCommand (command, 0);
}

void RObjectViewer::objectRemoved (RObject *object) {
	RK_TRACE (APP);

	if (object == _object) {
		status_label->setText (i18n ("<b>Object was deleted!</b>"));
		update_button->setEnabled (false);
	}
}

void RObjectViewer::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	RK_ASSERT (command);

	if (command->getFlags () == SUMMARY_COMMAND) {
		summary_area->append (command->fullOutput ());
	} else if (command->getFlags () == PRINT_COMMAND) {
		print_area->append (command->fullOutput ());
		status_label->setText (i18n ("Ready"));
		update_button->setEnabled (true);
		cancel_button->setEnabled (false);
	} else {
		RK_ASSERT (false);
	}

}

void RObjectViewer::closeEvent (QCloseEvent *e) {
	hide ();
	e->accept ();
	deleteLater ();
}

#include "robjectviewer.moc"
