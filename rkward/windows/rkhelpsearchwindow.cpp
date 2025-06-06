/*
rkhelpsearchwindow - This file is part of RKWard (https://rkward.kde.org). Created: Fri Feb 25 2005
SPDX-FileCopyrightText: 2005-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkhelpsearchwindow.h"

#include <KLocalizedString>
#include <QUrl>
#include <kmessagebox.h>

#include <QFocusEvent>
#include <QHBoxLayout>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include "../debug.h"
#include "../rbackend/rkrinterface.h"
#include "../rbackend/rksessionvars.h"

#include "../core/robject.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkdummypart.h"
#include "../misc/rkstandardicons.h"
#include "../rkward.h"

// result columns
#define COL_TYPE 0
#define COL_TOPIC 1
#define COL_TITLE 2
#define COL_PACKAGE 3
#define COL_COUNT 4

RKHelpSearchWindow *RKHelpSearchWindow::main_help_search = nullptr;

RKHelpSearchWindow::RKHelpSearchWindow(QWidget *parent, bool tool_window, const char *name) : RKMDIWindow(parent, SearchHelpWindow, tool_window, name) {
	RK_TRACE(APP);
	setPart(new RKDummyPart(nullptr, this));
	initializeActivationSignals();
	setFocusPolicy(Qt::ClickFocus);

	QVBoxLayout *main_layout = new QVBoxLayout(this);
	QHBoxLayout *selection_layout = new QHBoxLayout();
	main_layout->addLayout(selection_layout);
	selection_layout->setContentsMargins(0, 0, 0, 0);

	QVBoxLayout *labels_layout = new QVBoxLayout();
	selection_layout->addLayout(labels_layout);
	labels_layout->setContentsMargins(0, 0, 0, 0);
	QLabel *label = new QLabel(i18n("Find:"), this);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	labels_layout->addWidget(label);
	label = new QLabel(i18n("Fields:"), this);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	labels_layout->addWidget(label);

	QVBoxLayout *main_settings_layout = new QVBoxLayout();
	selection_layout->addLayout(main_settings_layout);
	main_settings_layout->setContentsMargins(0, 0, 0, 0);
	field = new QComboBox(this);
	field->setEditable(true);
	field->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	connect(field->lineEdit(), &QLineEdit::returnPressed, this, &RKHelpSearchWindow::slotFindButtonClicked);
	main_settings_layout->addWidget(field);

	QHBoxLayout *fields_packages_layout = new QHBoxLayout();
	main_settings_layout->addLayout(fields_packages_layout);
	fields_packages_layout->setContentsMargins(0, 0, 0, 0);
	fieldsList = new QComboBox(this);
	fieldsList->setEditable(false);
	fieldsList->addItem(i18n("All"), u"c(\"alias\", \"concept\", \"title\",\"keyword\")"_s);
	fieldsList->addItem(i18n("All but keywords"), u"c(\"alias\", \"concept\", \"title\")"_s);
	fieldsList->addItem(i18n("Keywords"), u"c(\"keyword\")"_s);
	fieldsList->addItem(i18n("Title"), u"c(\"title\")"_s);
	fields_packages_layout->addWidget(fieldsList);

	label = new QLabel(i18n("Package:"), this);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	fields_packages_layout->addWidget(label);

	packagesList = new QComboBox(this);
	packagesList->setEditable(false);
	fields_packages_layout->addWidget(packagesList);
	connect(RKSessionVars::instance(), &RKSessionVars::installedPackagesChanged, this, &RKHelpSearchWindow::updateInstalledPackages);
	updateInstalledPackages();

	QVBoxLayout *checkboxes_layout = new QVBoxLayout();
	selection_layout->addLayout(checkboxes_layout);
	checkboxes_layout->setContentsMargins(0, 0, 0, 0);
	caseSensitiveCheckBox = new QCheckBox(i18n("Case sensitive"), this);
	checkboxes_layout->addWidget(caseSensitiveCheckBox);
	fuzzyCheckBox = new QCheckBox(i18n("Fuzzy matching"), this);
	fuzzyCheckBox->setChecked(true);
	checkboxes_layout->addWidget(fuzzyCheckBox);

	findButton = new QPushButton(i18n("Find"), this);
	findButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(findButton, &QPushButton::clicked, this, &RKHelpSearchWindow::slotFindButtonClicked);
	selection_layout->addWidget(findButton);

	results = new RKHelpSearchResultsModel(this);
	proxy_model = new QSortFilterProxyModel(this);
	proxy_model->setSourceModel(results);
	results_view = new QTreeView(this);
	results_view->setRootIsDecorated(false);
	results_view->setModel(proxy_model);
	results_view->setSortingEnabled(true);
	connect(results_view, &QTreeView::doubleClicked, this, &RKHelpSearchWindow::resultDoubleClicked);
	main_layout->addWidget(results_view);

	setCaption(i18n("Help search"));
}

RKHelpSearchWindow::~RKHelpSearchWindow() {
	RK_TRACE(APP);
}

void RKHelpSearchWindow::focusInEvent(QFocusEvent *e) {
	RK_TRACE(APP);

	RKMDIWindow::focusInEvent(e);
	if (e->reason() != Qt::MouseFocusReason) {
		field->setFocus();
	}
}

void RKHelpSearchWindow::getContextHelp(const QString &context_line, int cursor_pos) {
	RK_TRACE(APP);
	QString result = RKCommonFunctions::getCurrentSymbol(context_line, cursor_pos);
	if (result.isEmpty()) return;

	getFunctionHelp(result);
}

void RKHelpSearchWindow::getFunctionHelp(const QString &function_name, const QString &package, const QString &type) {
	RK_TRACE(APP);

	QString command = RObject::rQuote(function_name);
	if (!package.isEmpty()) command.append(u", package="_s + RObject::rQuote(package));

	if (type == QLatin1String("demo")) command = u"rk.demo("_s + command + u')';
	else if (type == QLatin1String("vignette")) command = u"print (vignette("_s + command + u"))"_s;
	else command = u".rk.getHelp("_s + command + u')'; // we use .rk.getHelp() instead of plain help() to receive an error, if no help could be found

	auto c = new RCommand(command, RCommand::App | RCommand::GetStringVector, i18n("Find HTML help for %1", function_name));
	c->whenFinished(this, [this, function_name](RCommand *command) {
		if (command->failed()) {
			KMessageBox::error(this,
			                   i18n("No help found on '%1'. Maybe the corresponding package is not installed/loaded, or maybe you mistyped the command. Try "
			                        "using Help->Search R Help for more options.",
			                        function_name),
			                   i18n("No help found"));
		}
	});
	RInterface::issueCommand(c);
}

void RKHelpSearchWindow::slotFindButtonClicked() {
	RK_TRACE(APP);

	if (field->currentText().isEmpty()) {
		return;
	}

	QString agrep = QStringLiteral("FALSE");
	if (fuzzyCheckBox->isChecked()) {
		agrep = QLatin1String("NULL");
	}

	QString ignoreCase = QStringLiteral("TRUE");
	if (caseSensitiveCheckBox->isChecked()) {
		ignoreCase = QLatin1String("FALSE");
	}

	QString package = QStringLiteral(", package=");
	if (packagesList->currentIndex() == 0) {
		package.append(u"NULL"_s); // all installed packages; actually we could also use package.clear(), here.
	} else if (packagesList->currentIndex() == 1) {
		package.append(u".packages()"_s); // all loaded packages
	} else {
		package.append(u"\""_s + packagesList->currentText() + u"\""_s);
	}

	QString fields = fieldsList->itemData(fieldsList->currentIndex()).toString();

	QString s = u".rk.get.search.results ("_s + RObject::rQuote(field->currentText()) + u", agrep="_s + agrep + u", ignore.case="_s + ignoreCase + package +
	            u", fields="_s + fields + u')';

	auto c = new RCommand(s, RCommand::App | RCommand::Sync | RCommand::GetStringVector);
	c->whenFinished(this, [this](RCommand *command) {
		QStringList res;
		if (command->failed()) {
			RK_ASSERT(false);
		} else {
			RK_ASSERT(command->getDataType() == RData::StringVector);
			res = command->stringVector();
		}
		results->setResults(res);

		for (int i = 0; i < COL_COUNT; ++i)
			results_view->resizeColumnToContents(i);
		setEnabled(true);
	});
	RInterface::issueCommand(c);
	setEnabled(false);
	field->addItem(field->currentText());
}

void RKHelpSearchWindow::resultDoubleClicked(const QModelIndex &index) {
	RK_TRACE(APP);

	if (!index.isValid()) return;

	int row = proxy_model->mapToSource(index).row();
	QString topic = results->data(results->index(row, COL_TOPIC)).toString();
	if (topic.isEmpty()) return;
	QString package = results->data(results->index(row, COL_PACKAGE)).toString();
	QString type = results->resultsType(row);

	getFunctionHelp(topic, package, type);
}

void RKHelpSearchWindow::updateInstalledPackages() {
	RK_TRACE(APP);

	QString old_value = packagesList->currentText();

	packagesList->clear();
	packagesList->addItem(i18n("All installed packages"));
	packagesList->addItem(i18n("All loaded packages"));
	packagesList->insertSeparator(2);
	packagesList->addItems(RKSessionVars::instance()->installedPackages());

	int index = 0;
	if (!old_value.isEmpty()) index = packagesList->findText(old_value);
	if (index < 0) index = 0;
	packagesList->setCurrentIndex(index);
}

//////////////// RKHelpResultsModel ////////////////

RKHelpSearchResultsModel::RKHelpSearchResultsModel(QObject *parent) : QAbstractTableModel(parent) {
	RK_TRACE(APP);

	result_count = 0;
}

RKHelpSearchResultsModel::~RKHelpSearchResultsModel() {
	RK_TRACE(APP);
}

void RKHelpSearchResultsModel::setResults(const QStringList &results) {
	RK_TRACE(APP);

	RK_ASSERT((results.size() % 4) == 0);
	beginResetModel();

	result_count = results.size() / 4;
	topics = results.mid(0, result_count);
	titles = results.mid(result_count, result_count);
	packages = results.mid(result_count * 2, result_count);
	types = results.mid(result_count * 3, result_count);

	endResetModel();
}

int RKHelpSearchResultsModel::rowCount(const QModelIndex &parent) const {
	// don't trace
	if (parent.isValid()) return 0;
	return result_count;
}

int RKHelpSearchResultsModel::columnCount(const QModelIndex &parent) const {
	// don't trace
	if (parent.isValid()) return 0;
	return COL_COUNT;
}

QString RKHelpSearchResultsModel::resultsType(int row) {
	RK_TRACE(APP);

	if (row >= result_count) {
		RK_ASSERT(false);
		return QString();
	}
	return types[row];
}

QVariant RKHelpSearchResultsModel::data(const QModelIndex &index, int role) const {
	// don't trace

	// easier typing
	int row = index.row();
	int col = index.column();
	if (result_count && (row < result_count)) {
		if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
			if (col == COL_TOPIC) return topics[row];
			if (col == COL_TITLE) return titles[row];
			if (col == COL_PACKAGE) return packages[row];
			if (col == COL_TYPE) return types[row];
		} else if ((col == 0) && (role == Qt::DecorationRole)) {
			if (types[row] == QLatin1String("help")) return RKStandardIcons::getIcon(RKStandardIcons::WindowHelp);
			if (types[row] == QLatin1String("demo")) return RKStandardIcons::getIcon(RKStandardIcons::WindowCommandEditor);
			if (types[row] == QLatin1String("vignette")) return RKStandardIcons::getIcon(RKStandardIcons::DocumentPDF);
		}
	} else {
		RK_ASSERT(false);
	}

	return QVariant();
}

QVariant RKHelpSearchResultsModel::headerData(int section, Qt::Orientation orientation, int role) const {
	// don't trace

	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			if (section == COL_TOPIC) return (i18n("Topic"));
			if (section == COL_TITLE) return (i18n("Title"));
			if (section == COL_PACKAGE) return (i18n("Package"));
			if (section == COL_TYPE) return (i18n("Type"));
		}
	}

	return QVariant();
}
