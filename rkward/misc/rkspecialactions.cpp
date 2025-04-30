/*
rkspecialactions - This file is part of RKWard (https://rkward.kde.org). Created: Mon Mar 15 2010
SPDX-FileCopyrightText: 2010-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkspecialactions.h"

#include <KLocalizedString>
#include <KMessageBox>

#include "../rkward.h"

#include "../debug.h"

RKPasteSpecialAction::RKPasteSpecialAction(QObject *parent) : QAction(parent) {
	RK_TRACE(MISC);

	setText(i18n("Paste special..."));
	connect(this, &QAction::triggered, this, &RKPasteSpecialAction::doSpecialPaste);
}

RKPasteSpecialAction::~RKPasteSpecialAction() {
	RK_TRACE(MISC);
}

void RKPasteSpecialAction::doSpecialPaste() {
	RK_TRACE(MISC);

	QWidget *pwin = nullptr;
	const auto objs = associatedObjects();
	for (auto obj : objs) {
		if (qobject_cast<QWidget *>(obj)) {
			pwin = static_cast<QWidget *>(obj);
			break;
		}
	}
	RKPasteSpecialDialog *dialog = new RKPasteSpecialDialog(pwin);
	int res = dialog->exec();
	if (res == QDialog::Accepted) {
		Q_EMIT pasteText(dialog->resultingText());
	}
	dialog->deleteLater();
}

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QVBoxLayout>

#include "../core/robject.h"
#include "../dataeditor/rktextmatrix.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkradiogroup.h"
#include "../rbackend/rkrinterface.h"
#include "rksaveobjectchooser.h"

RKPasteSpecialDialog::RKPasteSpecialDialog(QWidget *parent, bool standalone) : QDialog(parent) {
	RK_TRACE(MISC);

	setWindowTitle(i18n("Paste Special..."));

	QVBoxLayout *pagelayout = new QVBoxLayout(this);
	objectname = standalone ? new RKSaveObjectChooser(this, QStringLiteral("pasted.data")) : nullptr;
	if (objectname) {
		connect(objectname, &RKSaveObjectChooser::changed, this, &RKPasteSpecialDialog::updateState);
		pagelayout->addWidget(objectname);
	}
	QHBoxLayout *rowlayout = new QHBoxLayout();
	pagelayout->addLayout(rowlayout);

	// Mode box
	auto box = new RKRadioGroup(i18n("Paste Mode"));
	dimensionality_group = box->group();
	box->addButton(i18n("Single string"), DimSingleString);
	box->addButton(i18n("Vector"), DimVector);
	box->addButton(i18n("Matrix"), DimMatrix);
	box->addButton(i18n("data.frame"), DimDataFrame)->setChecked(true);
	;
	connect(dimensionality_group, &QButtonGroup::idClicked, this, &RKPasteSpecialDialog::updateState);
	rowlayout->addWidget(box);

	const QMimeData *clipdata = QApplication::clipboard()->mimeData();

	// Separator box
	box = new RKRadioGroup(i18n("Field Separator"));
	separator_group = box->group();
	box->addButton(i18n("Tab"), SepTab)->setChecked(true); // tab-separated is a reasonable fallback guess
	box->addButton(i18n("Comma"), SepComma)->setChecked(clipdata->hasFormat(QStringLiteral("text/comma-separated-values")));
	box->addButton(i18n("Single space"), SepSpace);
	box->addButton(i18n("Any whitespace"), SepWhitespace);
	separator_freefield = new QLineEdit(QStringLiteral(";"), box);
	box->addButton(i18n("Other:"), SepCustom, separator_freefield, QBoxLayout::LeftToRight);
	connect(separator_group, &QButtonGroup::idClicked, this, &RKPasteSpecialDialog::updateState);
	rowlayout->addWidget(box);

	rowlayout = new QHBoxLayout;
	pagelayout->addLayout(rowlayout);

	// Quoting box
	box = new RKRadioGroup(i18n("Quoting"));
	quoting_group = box->group();
	box->addButton(i18n("Do not quote values"), QuoteNone);
	box->addButton(i18n("Automatic"), QuoteAuto)->setChecked(true);
	box->addButton(i18n("Quote all values"), QuoteAll);
	connect(quoting_group, &QButtonGroup::idClicked, this, &RKPasteSpecialDialog::updateState);
	rowlayout->addWidget(box);

	// Labels
	auto gbox = new QGroupBox(i18n("Labels"));
	auto group_layout = new QVBoxLayout(gbox);
	names_box = new QCheckBox(i18n("First row contains labels"));
	group_layout->addWidget(names_box);
	rownames_box = new QCheckBox(i18n("First column contains labels"));
	group_layout->addWidget(rownames_box);
	rowlayout->addWidget(gbox);

	// further controls
	gbox = new QGroupBox(i18n("Transformations"));
	group_layout = new QVBoxLayout(gbox);
	reverse_h_box = new QCheckBox(i18n("Reverse horizontally"));
	group_layout->addWidget(reverse_h_box);
	reverse_v_box = new QCheckBox(i18n("Reverse vertically"));
	group_layout->addWidget(reverse_v_box);
	transpose_box = new QCheckBox(i18n("Flip rows/columns"));
	group_layout->addWidget(transpose_box);
	insert_nas_box = new QCheckBox(i18n("Insert NAs where needed"));
	insert_nas_box->setChecked(true);
	group_layout->addWidget(insert_nas_box);
	rowlayout->addWidget(box);

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	ok_button = buttons->button(QDialogButtonBox::Ok);
	connect(ok_button, &QPushButton::clicked, this, &QDialog::accept);
	connect(buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);
	pagelayout->addWidget(buttons);

	updateState(); // initialize
}

RKPasteSpecialDialog::~RKPasteSpecialDialog() {
	RK_TRACE(MISC);
}

void RKPasteSpecialDialog::updateState() {
	RK_TRACE(MISC);

	int dimensionality = dimensionality_group->checkedId();

	static_cast<QWidget *>(separator_group->parent())->setEnabled(dimensionality != DimSingleString);
	reverse_h_box->setEnabled(dimensionality != DimSingleString);
	reverse_v_box->setEnabled(dimensionality >= DimMatrix);
	insert_nas_box->setEnabled(dimensionality != DimSingleString);
	transpose_box->setEnabled(dimensionality >= DimMatrix);
	names_box->setEnabled(dimensionality == DimDataFrame);
	rownames_box->setEnabled(dimensionality == DimDataFrame);

	ok_button->setEnabled((objectname == nullptr) || objectname->isOk());
}

QString RKPasteSpecialDialog::resultingText() {
	RK_TRACE(MISC);

	const int sep = separator_group->checkedId(); // for easier typing
	const int dim = dimensionality_group->checkedId();
	const bool reverse_h = reverse_h_box->isChecked() && (dim != DimSingleString);
	const bool reverse_v = reverse_v_box->isChecked() && (dim >= DimMatrix);
	const bool transpose = transpose_box->isChecked() && (dim >= DimMatrix);
	const bool names = names_box->isChecked() && (dim == DimDataFrame);
	const bool rownames = rownames_box->isChecked() && (dim == DimDataFrame);
	Quoting quot = (Quoting)quoting_group->checkedId();

	QString clip;

	const QMimeData *data = QApplication::clipboard()->mimeData();
	if ((dim != DimSingleString) && (sep == SepTab) && data->hasFormat(QStringLiteral("text/tab-separated-values"))) {
		clip = QString::fromLocal8Bit(data->data(QStringLiteral("text/tab-separated-values")));
	} else if ((dim != DimSingleString) && (sep == SepComma) && data->hasFormat(QStringLiteral("text/comma-separated-values"))) {
		clip = QString::fromLocal8Bit(data->data(QStringLiteral("text/comma-separated-values")));
	} else {
		clip = data->text();
	}

	if (dim == DimSingleString) return prepString(clip, quot);

	QRegularExpression fieldsep;
	if (sep == SepCustom) fieldsep.setPattern(separator_freefield->text());
	else if (sep == SepWhitespace) fieldsep.setPattern(QStringLiteral("\\s+"));
	else if (sep == SepSpace) fieldsep.setPattern(QStringLiteral(" "));
	else if (sep == SepTab) fieldsep.setPattern(QStringLiteral("\t"));
	else if (sep == SepComma) fieldsep.setPattern(QStringLiteral("\\,"));
	else RK_ASSERT(false);

	RKTextMatrix matrix = RKTextMatrix::matrixFromSeparatedValues(clip, fieldsep);
	if (dim == DimVector) {
		// transform list to single row matrix. This is wasteful on resources, but easy to code...
		QStringList list;
		for (int i = 0; i < matrix.numRows(); ++i) {
			list += matrix.getRow(i);
		}
		matrix = RKTextMatrix::matrixFromSeparatedValues(list.join(QStringLiteral("\t")));
	}

	if (reverse_h || reverse_v || transpose) matrix = matrix.transformed(reverse_h, reverse_v, transpose);

	QString ret;
	if (dim == DimDataFrame) ret.append(u"data.frame("_s);
	if (dim >= DimMatrix) ret.append(u"cbind(\n"_s);
	else ret.append(u"c("_s); // DimVector

	int startcol = rownames ? 1 : 0;
	int startrow = names ? 1 : 0;
	for (int i = startcol; i < matrix.numColumns(); ++i) {
		if (dim >= DimMatrix) {
			if (i != startcol) ret.append(u"),\n"_s);
			if (names) {
				ret.append(prepString(matrix.getText(0, i), QuoteAll) + u"=c("_s);
			} else {
				ret.append(u"c("_s);
			}
		} else if (i != 0) ret.append(u',');

		for (int j = startrow; j < matrix.numRows(); ++j) {
			if (j != startrow) ret.append(u',');
			ret.append(prepString(matrix.getText(j, i), quot));
		}
	}
	ret.append(u")\n"_s);
	if (dim == DimDataFrame) {
		ret.append(u')');
		if (rownames) {
			ret.append(u", rownames=c(\n"_s);
			for (int row = startrow; row < matrix.numRows(); ++row) {
				if (row != startrow) ret.append(u',');
				ret.append(prepString(matrix.getText(row, 0), QuoteAll));
			}
			ret.append(u")\n"_s);
		}
	}
	if (dim >= DimMatrix) ret.append(u")\n"_s);

	return (ret);
}

QString RKPasteSpecialDialog::prepString(const QString &src, const Quoting quot) const {
	//	RK_TRACE (MISC);

	if (quot == QuoteAll) return (RObject::rQuote(src));
	if (src.isEmpty() && insert_nas_box->isChecked()) return (u"NA"_s);
	if (quot == QuoteNone) return (src);
	RK_ASSERT(quot == QuoteAuto);

	bool numeric = false;
	src.toDouble(&numeric); // side-effect of setting numeric to true, if number conversion succeeds
	if (!numeric) return (RObject::rQuote(src));
	return src;
}

void RKPasteSpecialDialog::accept() {
	RK_TRACE(MISC);
	if (objectname) {
		RCommand *command = new RCommand(objectname->currentFullName() + u" <- "_s + resultingText(), RCommand::App | RCommand::ObjectListUpdate);
		connect(command->notifier(), &RCommandNotifier::commandFinished, [](RCommand *c) {
			if (c->failed()) {
				QString msg = c->fullOutput();
				if (msg.isEmpty()) msg = i18n("Command failed to parse. Try using <i>Edit->Paste special...</i> in the R Console window for better diagnostics.");
				KMessageBox::detailedError(RKWardMainWindow::getMain(), i18n("Pasting object from clipboard data failed."), msg, i18n("Paste failed"));
			}
		});
		RInterface::issueCommand(command);
	}
	QDialog::accept();
}
