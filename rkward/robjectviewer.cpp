/*
robjectviewer - This file is part of RKWard (https://rkward.kde.org). Created: Tue Aug 24 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "robjectviewer.h"

#include <QLabel>
#include <QTextEdit>
#include <QFont>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>

#include <KLocalizedString>
#include <QFontDatabase>

#include "rbackend/rkrinterface.h"

#include "core/robject.h"
#include "misc/rkdummypart.h"
#include "../misc/rkcommonfunctions.h"
#include "misc/rkprogresscontrol.h"

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

	description_label = RKCommonFunctions::wordWrappedLabel (QString ());
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
	connect (tabs, &QTabWidget::currentChanged, this, &RObjectViewer::currentTabChanged);

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

		QString reason = i18n ("<b>Object was deleted</b>");
		summary_widget->invalidate (reason);
		print_widget->invalidate (reason);
		structure_widget->invalidate (reason);

		QPalette palette = status_label->palette ();
		palette.setColor (status_label->foregroundRole (), Qt::red);
		status_label->setPalette (palette);
		status_label->setText (reason);
		status_label->show ();

		stopListenForObject (_object);
		_object = nullptr;
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
	// make the description use less height. Trying to specify <nobr>s, here, is no good idea (see http://sourceforge.net/p/rkward/bugs/55/)
	description_label->setText("<i>" + _object->getShortName().replace('<', "&lt;") + "</i> &nbsp; " + _object->getObjectDescription ().replace ("<br>", "&nbsp; &nbsp; "));
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

RObjectViewerWidget::RObjectViewerWidget(QWidget* parent, RObject* object) : QWidget(parent) {
	RK_TRACE (APP);

	_object = object;
	QVBoxLayout* main_layout = new QVBoxLayout(this);
	main_layout->setContentsMargins(0, 0, 0, 0);
	QHBoxLayout* status_layout = new QHBoxLayout();
	main_layout->addLayout(status_layout);

	status_label = new QLabel();
	status_layout->addWidget(status_label);

	status_layout->addStretch ();

	update_button = new QPushButton(i18n("Update"));
	connect(update_button, &QPushButton::clicked, this, &RObjectViewerWidget::update);
	status_layout->addWidget(update_button);

	area = new QTextEdit(this);
	area->setReadOnly(true);
	area->setLineWrapMode(QTextEdit::NoWrap);
	main_layout->addWidget(area);

	initialized = false;
}

RObjectViewerWidget::~RObjectViewerWidget () {
	RK_TRACE (APP);
}

void RObjectViewerWidget::invalidate (const QString& reason) {
	RK_TRACE (APP);

	QPalette palette = status_label->palette ();
	palette.setColor (status_label->foregroundRole (), Qt::red);
	status_label->setPalette (palette);

	status_label->setText(reason);
	update_button->setEnabled(_object != nullptr);
}

void RObjectViewerWidget::initialize () {
	RK_TRACE (APP);

	if (initialized) return;
	update ();
	initialized = true;
}

void RObjectViewerWidget::update() {
	RK_TRACE(APP);

	if (!_object) {
		RK_ASSERT(_object);
		return;
	}
	update_button->setEnabled(false);

	auto command = makeCommand();
	auto control = new RKInlineProgressControl(this, true);
	control->addRCommand(command);
	control->setAutoCloseWhenCommandsDone(true);
	control->setText(i18n("Fetching information"));
	control->show(100);
	command->whenFinished(this, [this](RCommand *command) {
		setText(command->fullOutput());
		update_button->setEnabled(_object != nullptr);
	});
	RInterface::issueCommand(command);
}

void RObjectViewerWidget::setText (const QString& text) {
	RK_TRACE (APP);

	area->setPlainText (QString ());
	QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	area->setCurrentFont (font);

	area->insertPlainText (text);
}

////////////////// summary widget /////////////////

RCommand* RObjectSummaryWidget::makeCommand() {
	RK_TRACE(APP);
	return new RCommand("print(summary(" + _object->getFullName() + "))", RCommand::App);
}

////////////////// print widget /////////////////

RCommand* RObjectPrintWidget::makeCommand() {
	RK_TRACE(APP);

	// make sure to print as wide as possible
	return new RCommand("local({\n"
	                                  "\trk.temp.width.save <- getOption(\"width\")\n"
	                                  "\toptions(width=10000)\n"
	                                  "\ton.exit(options(width=rk.temp.width.save))\n"
	                                  "\tprint(" + _object->getFullName() + ")\n"
	                                  "})", RCommand::App);
}

////////////////// structure widget /////////////////

RCommand* RObjectStructureWidget::makeCommand() {
	RK_TRACE(APP);
	return new RCommand("str(" + _object->getFullName() + ')', RCommand::App);
}

