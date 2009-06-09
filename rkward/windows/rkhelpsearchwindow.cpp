/***************************************************************************
                          rkhelpsearchwindow  -  description
                             -------------------
    begin                : Fri Feb 25 2005
    copyright            : (C) 2005, 2006, 2007, 2009 by Thomas Friedrichsmeier
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

#include "rkhelpsearchwindow.h"

#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <QTreeView>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <QHBoxLayout>
#include <QFocusEvent>
#include <QVBoxLayout>

#include "../rbackend/rinterface.h"
#include "../rbackend/rcommandreceiver.h"
#include "../debug.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkdummypart.h"

#define GET_HELP_URL 1
#define HELP_SEARCH 2
#define GET_INSTALLED_PACKAGES 3

// result columns
#define COL_TOPIC 0
#define COL_TITLE 1
#define COL_PACKAGE 2
#define COL_COUNT 3

RKHelpSearchWindow* RKHelpSearchWindow::main_help_search = 0;

RKHelpSearchWindow::RKHelpSearchWindow (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, SearchHelpWindow, tool_window, name) {
	RK_TRACE (APP);
	setPart (new RKDummyPart (0, this));
	initializeActivationSignals ();
	setFocusPolicy (Qt::ClickFocus);

	QVBoxLayout* main_layout = new QVBoxLayout (this);
	QHBoxLayout* selection_layout = new QHBoxLayout ();
	main_layout->addLayout (selection_layout);
	selection_layout->setContentsMargins (0, 0, 0, 0);

	QVBoxLayout* labels_layout = new QVBoxLayout ();
	selection_layout->addLayout (labels_layout);
	labels_layout->setContentsMargins (0, 0, 0, 0);
	QLabel *label = new QLabel (i18n ("Find:"), this);
	label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Minimum);
	labels_layout->addWidget (label);
	label = new QLabel (i18n ("Fields:"), this);
	label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Minimum);
	labels_layout->addWidget (label);

	QVBoxLayout* main_settings_layout = new QVBoxLayout ();
	selection_layout->addLayout (main_settings_layout);
	main_settings_layout->setContentsMargins (0, 0, 0, 0);
	field = new QComboBox (this);
	field->setEditable (true);
	field->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	connect (field->lineEdit () , SIGNAL (returnPressed ()), this, SLOT (slotFindButtonClicked ()));
	main_settings_layout->addWidget (field);

	QHBoxLayout* fields_packages_layout = new QHBoxLayout ();
	main_settings_layout->addLayout (fields_packages_layout);
	fields_packages_layout->setContentsMargins (0, 0, 0, 0);
	fieldsList = new QComboBox (this);
	fieldsList->setEditable (false);
	fieldsList->addItem (i18n ("All"), "c(\"alias\", \"concept\", \"title\",\"keyword\")");
	fieldsList->addItem (i18n ("All but keywords"), "c(\"alias\", \"concept\", \"title\")");
	fieldsList->addItem (i18n ("Keywords"), "c(\"keyword\")");
	fieldsList->addItem (i18n ("Title"), "c(\"title\")");
	fields_packages_layout->addWidget (fieldsList);

	label = new QLabel (i18n ("Package:"), this);
	label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Minimum);
	fields_packages_layout->addWidget (label);

	packagesList = new QComboBox (this);
	packagesList->setEditable (false);
	packagesList->addItem (i18n("All"));
	fields_packages_layout->addWidget (packagesList);

	QVBoxLayout* checkboxes_layout = new QVBoxLayout ();
	selection_layout->addLayout (checkboxes_layout);
	checkboxes_layout->setContentsMargins (0, 0, 0, 0);
	caseSensitiveCheckBox = new QCheckBox (i18n ("Case sensitive"), this);
	checkboxes_layout->addWidget (caseSensitiveCheckBox);
	fuzzyCheckBox = new QCheckBox (i18n ("Fuzzy matching"), this);
	fuzzyCheckBox->setChecked (true);
	checkboxes_layout->addWidget (fuzzyCheckBox);

	findButton = new QPushButton (i18n ("Find"), this);
	findButton->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect (findButton, SIGNAL (clicked ()), this, SLOT (slotFindButtonClicked ()));
	selection_layout->addWidget (findButton);

	results = new RKHelpSearchResultsModel (this);
	results_view = new QTreeView (this);
	results_view->setRootIsDecorated (false);
	results_view->setModel (results);
	connect (results_view, SIGNAL (doubleClicked(const QModelIndex&)), this, SLOT (resultDoubleClicked(const QModelIndex&)));
	main_layout->addWidget (results_view);

	RKGlobals::rInterface ()->issueCommand (".rk.get.installed.packages ()[[1]]", RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, GET_INSTALLED_PACKAGES, 0);

	setCaption (i18n ("Help search"));
}

RKHelpSearchWindow::~RKHelpSearchWindow () {
	RK_TRACE (APP);
}

void RKHelpSearchWindow::focusInEvent (QFocusEvent *e) {
	RK_TRACE (APP);

	RKMDIWindow::focusInEvent (e);
	if (e->reason () != Qt::MouseFocusReason) {
		field->setFocus ();
	}
}

void RKHelpSearchWindow::getContextHelp (const QString &context_line, int cursor_pos) {
	RK_TRACE (APP);
	QString result = RKCommonFunctions::getCurrentSymbol (context_line, cursor_pos);
	if (result.isEmpty ()) return;

	getFunctionHelp (result);
}

void RKHelpSearchWindow::getFunctionHelp (const QString &function_name, const QString &package) {
	RK_TRACE (APP);

	QString command = "help(\"" + function_name + '\"';
	if (!package.isEmpty ()) command.append (", package=" + package);
	command.append (", chmhelp=FALSE, htmlhelp=TRUE)[1]");

	RKGlobals::rInterface ()->issueCommand (command, RCommand::App | RCommand::GetStringVector, i18n ("Find HTML help for %1").arg (function_name), this, GET_HELP_URL);

	// we *could* simply call show() on the object that help() returns. However, since this function
	// may be called externally, we need to handle the case when no help can be found. So we use
	// this two-stage approach, instead.
}

void RKHelpSearchWindow::slotFindButtonClicked () {
	RK_TRACE (APP);

	if (field->currentText ().isEmpty ()) {
		return;
	}
	
	QString agrep = "FALSE";
	if (fuzzyCheckBox->isChecked ()) {
		agrep="NULL";
	}
	
	QString ignoreCase = "TRUE";
	if(caseSensitiveCheckBox->isChecked ()) {
		ignoreCase="FALSE";
	}
	
	QString package = "NULL";
	if (packagesList->currentIndex () != 0) {
		package= "\"" +	packagesList->currentText () + "\"";
	}

	QString fields = fieldsList->itemData (fieldsList->currentIndex ()).toString ();

	QString s = ".rk.get.search.results (\"" + field->currentText () + "\",agrep=" + agrep + ", ignore.case=" + ignoreCase + ", package=" + package + ", fields=" + fields +")";
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, HELP_SEARCH, 0);
	setEnabled (false);
	field->addItem (field->currentText ());
}

void RKHelpSearchWindow::resultDoubleClicked (const QModelIndex& index) {
	RK_TRACE (APP);

	if (!index.isValid ()) return;

	int row = index.row ();
	QString topic = results->data (results->index (row, COL_TOPIC)).toString ();
	QString package = results->data (results->index (row, COL_PACKAGE)).toString ();
	if (topic.isEmpty ()) return;

	getFunctionHelp (topic, package);
}

void RKHelpSearchWindow::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	if (command->getFlags () == HELP_SEARCH) {
		RK_ASSERT ((command->getDataLength () % 3) == 0);
		RK_ASSERT (command->getDataType () == RData::StringVector);

		results->setResults (command->getStringVector (), command->getDataLength () / 3);
		command->detachData ();		// now owned by the model

		for (int i = 0; i < COL_COUNT; ++i) results_view->resizeColumnToContents (i);
		setEnabled(true);
	} else if (command->getFlags () == GET_HELP_URL) {
		KUrl url;

		if (command->getDataLength ()) {
			RK_ASSERT (command->getDataType () == RData::StringVector);
			url.setPath(command->getStringVector ()[0]);
		}
		if (QFile::exists (url.path ())) {
			RKWardMainWindow::getMain ()->openHTML (url);
			return;
		} else {
			KMessageBox::sorry (this, i18n ("No help found on '%1'. Maybe the corresponding package is not installed/loaded, or maybe you mistyped the command. Try using Help->Search R Help for more options.", command->command ().section ("\"", 1, 1)), i18n ("No help found"));
		}
	} else if (command->getFlags () == GET_INSTALLED_PACKAGES) {
		RK_ASSERT (command->getDataType () == RData::StringVector);
		unsigned int count = command->getDataLength ();
		for (unsigned int i=0; i < count; ++i) {
			packagesList->addItem (command->getStringVector ()[i]);
		}
	} else {
		RK_ASSERT (false);
	}
}

//////////////// RKHelpResultsModel ////////////////

RKHelpSearchResultsModel::RKHelpSearchResultsModel (QObject *parent) : QAbstractTableModel (parent) {
	RK_TRACE (APP);

	results = 0;
	result_count = 0;
}

RKHelpSearchResultsModel::~RKHelpSearchResultsModel () {
	RK_TRACE (APP);

	delete [] results;
}

void RKHelpSearchResultsModel::setResults (QString* new_results, int new_result_count) {
	RK_TRACE (APP);

	delete [] results;
	results = new_results;
	result_count = new_result_count;

	reset ();
}

int RKHelpSearchResultsModel::rowCount (const QModelIndex& parent) const {
	// don't trace
	if (parent.isValid ()) return 0;
	return result_count;
}

int RKHelpSearchResultsModel::columnCount (const QModelIndex& parent) const {
	// don't trace
	if (parent.isValid ()) return 0;
	return COL_COUNT;
}

QVariant RKHelpSearchResultsModel::data (const QModelIndex& index, int role) const {
	// don't trace

	// easier typing
	int row = index.row ();
	int col = index.column ();
	if (results) {
		if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
			if (row < result_count) {
				if (col < COL_COUNT) {
					return results[row + col * result_count];
				} else {
				
				}
			} else {
				RK_ASSERT (false);
			}
		}
	} else {
		RK_ASSERT (false);
	}

	return QVariant ();
}

QVariant RKHelpSearchResultsModel::headerData (int section, Qt::Orientation orientation, int role) const {
	// don't trace

	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			if (section == COL_TOPIC) return (i18n ("Topic"));
			if (section == COL_TITLE) return (i18n ("Title"));
			if (section == COL_PACKAGE) return (i18n ("Package"));
		}
	}

	return QVariant ();
}

#include "rkhelpsearchwindow.moc"
