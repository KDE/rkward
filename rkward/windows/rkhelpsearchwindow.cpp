/***************************************************************************
                          rkhelpsearchwindow  -  description
                             -------------------
    begin                : Fri Feb 25 2005
    copyright            : (C) 2005, 2006, 2007, 2009, 2010, 2011 by Thomas Friedrichsmeier
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
#include <QSortFilterProxyModel>

#include "../rbackend/rinterface.h"
#include "../rbackend/rcommandreceiver.h"
#include "../debug.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "../core/robject.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkdummypart.h"
#include "../misc/rkstandardicons.h"

#define GET_HELP 1
#define HELP_SEARCH 2
#define GET_INSTALLED_PACKAGES 3

// result columns
#define COL_TYPE 0
#define COL_TOPIC 1
#define COL_TITLE 2
#define COL_PACKAGE 3
#define COL_COUNT 4

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
	packagesList->addItem (i18n("All installed packages"));
	packagesList->addItem (i18n("All loaded packages"));
#if QT_VERSION >= 0x040400
	packagesList->insertSeparator (2);
#endif
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
	proxy_model = new QSortFilterProxyModel (this);
	proxy_model->setSourceModel (results);
	results_view = new QTreeView (this);
	results_view->setRootIsDecorated (false);
	results_view->setModel (proxy_model);
	results_view->setSortingEnabled (true);
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

void RKHelpSearchWindow::getFunctionHelp (const QString &function_name, const QString &package, const QString &type) {
	RK_TRACE (APP);

	// we use .rk.getHelp() instead of plain help() to receive an error, if no help could be found
	QString command = ".rk.getHelp(";
	if (type == "demo") command = "rk.demo(";
	else if (type == "vignette") command = "print (vignette(";

	command.append (RObject::rQuote (function_name));
	if (!package.isEmpty ()) command.append (", package=" + RObject::rQuote (package));
	command.append (")");
	if (type == "vignette") command.append (")");

	RKGlobals::rInterface ()->issueCommand (command, RCommand::App | RCommand::GetStringVector, i18n ("Find HTML help for %1").arg (function_name), this, GET_HELP);
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
	
	QString package = ", package=";
	if (packagesList->currentIndex () == 0) {
		package.append ("NULL");	// all installed packages; actually we could also use package.clear(), here.
	} else if (packagesList->currentIndex () == 1) {
		package.append (".packages()");	// all loaded packages
	} else {
		package.append ("\"" + packagesList->currentText () + "\"");
	}

	QString fields = fieldsList->itemData (fieldsList->currentIndex ()).toString ();

	QString s = ".rk.get.search.results (" + RObject::rQuote (field->currentText ()) + ", agrep=" + agrep + ", ignore.case=" + ignoreCase + package + ", fields=" + fields +")";
	
	RKGlobals::rInterface ()->issueCommand (s, RCommand::App | RCommand::Sync | RCommand::GetStringVector, QString::null, this, HELP_SEARCH, 0);
	setEnabled (false);
	field->addItem (field->currentText ());
}

void RKHelpSearchWindow::resultDoubleClicked (const QModelIndex& index) {
	RK_TRACE (APP);

	if (!index.isValid ()) return;

	int row = proxy_model->mapToSource (index).row ();
	QString topic = results->data (results->index (row, COL_TOPIC)).toString ();
	if (topic.isEmpty ()) return;
	QString package = results->data (results->index (row, COL_PACKAGE)).toString ();
	QString type = results->resultsType (row);

	getFunctionHelp (topic, package, type);
}

void RKHelpSearchWindow::rCommandDone (RCommand *command) {
	RK_TRACE (APP);
	if (command->getFlags () == HELP_SEARCH) {
		if (command->failed ()) {
			RK_ASSERT (false);
			return;
		}
		RK_ASSERT (command->getDataType () == RData::StringVector);

		results->setResults (command->getStringVector ());

		for (int i = 0; i < COL_COUNT; ++i) results_view->resizeColumnToContents (i);
		setEnabled(true);
	} else if (command->getFlags () == GET_HELP) {
		if (command->failed ()) {
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

	result_count = 0;
}

RKHelpSearchResultsModel::~RKHelpSearchResultsModel () {
	RK_TRACE (APP);
}

void RKHelpSearchResultsModel::setResults (const QStringList &results) {
	RK_TRACE (APP);

	RK_ASSERT ((results.size () % 4) == 0);

	result_count = results.size () / 4;
	topics = results.mid (0, result_count);
	titles = results.mid (result_count, result_count);
	packages = results.mid (result_count*2, result_count);
	types = results.mid (result_count*3, result_count);

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

QString RKHelpSearchResultsModel::resultsType (int row) {
	RK_TRACE (APP);

	if (row >= result_count) {
		RK_ASSERT (false);
		return QString ();
	}
	return types[row];
}

QVariant RKHelpSearchResultsModel::data (const QModelIndex& index, int role) const {
	// don't trace

	// easier typing
	int row = index.row ();
	int col = index.column ();
	if (result_count && (row < result_count)) {
		if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
			if (col == COL_TOPIC) return topics[row];
			if (col == COL_TITLE) return titles[row];
			if (col == COL_PACKAGE) return packages[row];
			if (col == COL_TYPE) return types[row];
		} else if ((col == 0) && (role == Qt::DecorationRole)) {
			if (types[row] == "help") return RKStandardIcons::getIcon (RKStandardIcons::WindowHelp);
			if (types[row] == "demo") return RKStandardIcons::getIcon (RKStandardIcons::WindowCommandEditor);
			if (types[row] == "vignette") return RKStandardIcons::getIcon (RKStandardIcons::DocumentPDF);
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
			if (section == COL_TYPE) return (i18n ("Type"));
		}
	}

	return QVariant ();
}

#include "rkhelpsearchwindow.moc"
