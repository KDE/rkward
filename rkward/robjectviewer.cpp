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
#include <QScrollArea>
#include <qlabel.h>
#include <QTextEdit>
#include <qfont.h>
#include <QPushButton>
#include <QCloseEvent>
#include <QVBoxLayout>

#include <klocale.h>
#include <kvbox.h>
#include <khbox.h>
#include <kglobalsettings.h>

#include "rbackend/rinterface.h"
#include "rkglobals.h"
#include "core/robject.h"
#include "core/rcontainerobject.h"
#include "misc/rkdummypart.h"
#include "core/rkmodificationtracker.h"

#include "debug.h"

#define SUMMARY_COMMAND 1
#define PRINT_COMMAND 2

RObjectViewer::RObjectViewer (QWidget *parent, RObject *object) : RKMDIWindow (parent, RKMDIWindow::ObjectWindow, false), RObjectListener (RObjectListener::ObjectView) {
// KDE 4: TODO might listen for object meta / data changes as well
	RK_TRACE (APP);
	RK_ASSERT (object);
	_object = object;

	addNotificationType (RObjectListener::ObjectRemoved);
	listenForObject (_object);

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	QScrollArea *wrapper = new QScrollArea (this);
	wrapper->setWidgetResizable (true);
	KVBox *box = new KVBox (wrapper);
	wrapper->setWidget (box);
	layout->addWidget (wrapper);

	wrapper->setFocusPolicy (Qt::StrongFocus);
	setPart (new RKDummyPart (this, wrapper));
	initializeActivationSignals ();

	KHBox *toprow = new KHBox (box);
	description_label = new QLabel (toprow);
	KVBox *statusbox = new KVBox (toprow);
	statusbox->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	status_label = new QLabel (statusbox);
	update_button = new QPushButton (i18n ("Update"), statusbox);
	cancel_button = new QPushButton (i18n ("Cancel"), statusbox);
	connect (update_button, SIGNAL (clicked ()), this, SLOT (update ()));
	connect (cancel_button, SIGNAL (clicked ()), this, SLOT (cancel ()));

	KHBox *row = new KHBox (box);
	QLabel *label = new QLabel (i18n("\nResult of 'summary (%1)':\n", object->getFullName ()), row);
	row->setStretchFactor (label, 10);
	toggle_summary_button = new QPushButton (i18n ("Hide"), row);
	connect (toggle_summary_button, SIGNAL (clicked ()), this, SLOT (toggleSummary ()));

	summary_area = new QTextEdit (box);
	summary_area->setReadOnly (true);
	summary_area->setLineWrapMode (QTextEdit::NoWrap);

	row = new KHBox (box);
	label = new QLabel (i18n("\nResult of 'print (%1)':\n", object->getFullName ()), row);
	row->setStretchFactor (label, 10);
	toggle_print_button = new QPushButton (i18n ("Hide"), row);
	connect (toggle_print_button, SIGNAL (clicked ()), this, SLOT (togglePrint ()));

	print_area = new QTextEdit (box);
	print_area->setReadOnly (true);
	print_area->setLineWrapMode (QTextEdit::NoWrap);

	setCaption (i18n("Object Viewer: ") + object->getShortName ());
	//resize (minimumSizeHint ().expandedTo (QSize (640, 480)));
	update ();
	show ();
}

RObjectViewer::~RObjectViewer () {
	RK_TRACE (APP);

	if (_object) stopListenForObject (_object);
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
	if (!_object) return;

	status_label->setText (i18n ("Fetching information"));
	cancel_button->setEnabled (true);
	update_button->setEnabled (false);
	description_label->setText (_object->getObjectDescription ());

	summary_area->setPlainText (QString::null);
	print_area->setPlainText (QString::null);
	QFont font = KGlobalSettings::fixedFont ();
	summary_area->setCurrentFont (font);
	print_area->setCurrentFont (font);

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
		stopListenForObject (_object);
		_object = 0;
	} else {
		RK_ASSERT (false);
	}
}

void RObjectViewer::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	RK_ASSERT (command);

	if (command->getFlags () == SUMMARY_COMMAND) {
		summary_area->insertPlainText (command->fullOutput ());
	} else if (command->getFlags () == PRINT_COMMAND) {
		print_area->insertPlainText (command->fullOutput ());
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
