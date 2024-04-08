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

RKPasteSpecialAction::RKPasteSpecialAction(QObject* parent) : QAction(parent) {
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
	for(auto obj : objs) {
		if (qobject_cast<QWidget*>(obj)) {
			pwin = static_cast<QWidget*>(obj);
			break;
		}
	}
	RKPasteSpecialDialog* dialog = new RKPasteSpecialDialog(pwin);
	int res = dialog->exec();
	if (res == QDialog::Accepted) {
		Q_EMIT pasteText(dialog->resultingText());
	}
	dialog->deleteLater();
}

#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDialogButtonBox>
#include <QPushButton>

#include "../dataeditor/rktextmatrix.h"
#include "../core/robject.h"
#include "rksaveobjectchooser.h"
#include "../rbackend/rkrinterface.h"
#include "../misc/rkprogresscontrol.h"

RKPasteSpecialDialog::RKPasteSpecialDialog(QWidget* parent, bool standalone) : QDialog(parent) {
	RK_TRACE (MISC);

	setWindowTitle (i18n ("Paste Special..."));

	QVBoxLayout *pagelayout = new QVBoxLayout (this);
	objectname = standalone ? new RKSaveObjectChooser(this, QStringLiteral("pasted.data")) : nullptr;
	if (objectname) {
		connect(objectname, &RKSaveObjectChooser::changed, this, &RKPasteSpecialDialog::updateState);
		pagelayout->addWidget(objectname);
	}
	QHBoxLayout *rowlayout = new QHBoxLayout ();
	pagelayout->addLayout (rowlayout);

	QGroupBox* box;
	QVBoxLayout* group_layout;
	QHBoxLayout* h_layout;
	QRadioButton* rbutton;

	// Mode box
	box = new QGroupBox (i18n ("Paste Mode"), this);
	group_layout = new QVBoxLayout (box);
	dimensionality_group = new QButtonGroup (box);
	rbutton = new QRadioButton (i18n ("Single string"), box);
	dimensionality_group->addButton (rbutton, DimSingleString);
	group_layout->addWidget (rbutton);
	rbutton = new QRadioButton (i18n ("Vector"), box);
	dimensionality_group->addButton (rbutton, DimVector);
	group_layout->addWidget (rbutton);
	rbutton = new QRadioButton (i18n ("Matrix"), box);
	dimensionality_group->addButton (rbutton, DimMatrix);
	group_layout->addWidget (rbutton);
	rbutton = new QRadioButton (i18n ("data.frame"), box);
	dimensionality_group->addButton (rbutton, DimDataFrame);
	rbutton->setChecked (true);
	group_layout->addWidget (rbutton);
	connect (dimensionality_group, &QButtonGroup::idClicked, this, &RKPasteSpecialDialog::updateState);
	rowlayout->addWidget (box);

	const QMimeData* clipdata = QApplication::clipboard ()->mimeData ();

	// Separator box
	box = new QGroupBox (i18n ("Field Separator"), this);
	group_layout = new QVBoxLayout (box);
	separator_group = new QButtonGroup (box);
	rbutton = new QRadioButton (i18n ("Tab"), box);
	separator_group->addButton (rbutton, SepTab);
	group_layout->addWidget (rbutton);
	rbutton->setChecked (true);		// tab-separated is a reasonable fallback guess
	rbutton = new QRadioButton (i18n ("Comma"), box);
	separator_group->addButton (rbutton, SepComma);
	group_layout->addWidget (rbutton);
	if (clipdata->hasFormat ("text/comma-separated-values")) rbutton->setChecked (true);
	rbutton = new QRadioButton (i18n ("Single space"), box);
	separator_group->addButton (rbutton, SepSpace);
	group_layout->addWidget (rbutton);
	rbutton = new QRadioButton (i18n ("Any whitespace"), box);
	separator_group->addButton (rbutton, SepWhitespace);
	group_layout->addWidget (rbutton);
	h_layout = new QHBoxLayout ();
	rbutton = new QRadioButton (i18n ("Other:"), box);
	separator_group->addButton (rbutton, SepCustom);
	h_layout->addWidget (rbutton);
	separator_freefield = new QLineEdit (";", box);
	h_layout->addWidget (separator_freefield);
	group_layout->addLayout (h_layout);
	connect (separator_group, &QButtonGroup::idClicked, this, &RKPasteSpecialDialog::updateState);
	rowlayout->addWidget (box);

	rowlayout = new QHBoxLayout;
	pagelayout->addLayout (rowlayout);

	// Quoting box
	box = new QGroupBox (i18n ("Quoting"), this);
	group_layout = new QVBoxLayout (box);
	quoting_group = new QButtonGroup (box);
	rbutton = new QRadioButton (i18n ("Do not quote values"), box);
	quoting_group->addButton (rbutton, QuoteNone);
	group_layout->addWidget (rbutton);
	rbutton = new QRadioButton (i18n ("Automatic"), box);
	rbutton->setChecked (true);
	quoting_group->addButton (rbutton, QuoteAuto);
	group_layout->addWidget (rbutton);
	rbutton = new QRadioButton (i18n ("Quote all values"), box);
	quoting_group->addButton (rbutton, QuoteAll);
	group_layout->addWidget (rbutton);
	connect (quoting_group, &QButtonGroup::idClicked, this, &RKPasteSpecialDialog::updateState);
	rowlayout->addWidget (box);

	// Labels
	box = new QGroupBox(i18n("Labels"), this);
	group_layout = new QVBoxLayout (box);
	names_box = new QCheckBox(i18n("First row contains labels"), box);
	group_layout->addWidget(names_box);
	rownames_box = new QCheckBox(i18n("First column contains labels"), box);
	group_layout->addWidget(rownames_box);
	rowlayout->addWidget(box);

	// further controls
	box = new QGroupBox (i18n ("Transformations"), this);
	group_layout = new QVBoxLayout (box);
	reverse_h_box = new QCheckBox (i18n ("Reverse horizontally"), box);
	group_layout->addWidget (reverse_h_box);
	reverse_v_box = new QCheckBox (i18n ("Reverse vertically"), box);
	group_layout->addWidget (reverse_v_box);
	transpose_box = new QCheckBox (i18n ("Flip rows/columns"), box);
	group_layout->addWidget (transpose_box);
	insert_nas_box = new QCheckBox (i18n ("Insert NAs where needed"), box);
	insert_nas_box->setChecked (true);
	group_layout->addWidget (insert_nas_box);
	rowlayout->addWidget (box);

	QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	ok_button = buttons->button(QDialogButtonBox::Ok);
	connect(ok_button, &QPushButton::clicked, this, &QDialog::accept);
	connect(buttons->button (QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);
	pagelayout->addWidget (buttons);

	updateState ();		// initialize
}

RKPasteSpecialDialog::~RKPasteSpecialDialog () {
	RK_TRACE (MISC);
}

void RKPasteSpecialDialog::updateState () {
	RK_TRACE (MISC);

	int dimensionality = dimensionality_group->checkedId ();
	
	static_cast<QWidget*> (separator_group->parent ())->setEnabled (dimensionality != DimSingleString);
	reverse_h_box->setEnabled (dimensionality != DimSingleString);
	reverse_v_box->setEnabled (dimensionality >= DimMatrix);
	insert_nas_box->setEnabled (dimensionality != DimSingleString);
	transpose_box->setEnabled(dimensionality >= DimMatrix);
	names_box->setEnabled(dimensionality == DimDataFrame);
	rownames_box->setEnabled(dimensionality == DimDataFrame);

	separator_freefield->setEnabled ((dimensionality != DimSingleString) && (separator_group->checkedId () == SepCustom));
	ok_button->setEnabled((objectname == nullptr) || objectname->isOk());
}

QString RKPasteSpecialDialog::resultingText () {
	RK_TRACE (MISC);

	const int sep = separator_group->checkedId ();		// for easier typing
	const int dim = dimensionality_group->checkedId ();
	const bool reverse_h = reverse_h_box->isChecked () && (dim != DimSingleString);
	const bool reverse_v = reverse_v_box->isChecked () && (dim >= DimMatrix);
	const bool transpose = transpose_box->isChecked () && (dim >= DimMatrix);
	const bool names = names_box->isChecked() && (dim == DimDataFrame);
	const bool rownames = rownames_box->isChecked() && (dim == DimDataFrame);
	Quoting quot = (Quoting) quoting_group->checkedId();

	QString clip;

	const QMimeData* data = QApplication::clipboard ()->mimeData ();
	if ((dim != DimSingleString) && (sep == SepTab) && data->hasFormat ("text/tab-separated-values")) {
		clip = QString::fromLocal8Bit (data->data ("text/tab-separated-values"));
	} else if ((dim != DimSingleString) && (sep == SepComma) && data->hasFormat ("text/comma-separated-values")) {
		clip = QString::fromLocal8Bit (data->data ("text/comma-separated-values"));
	} else {
		clip = data->text ();
	}

	if (dim == DimSingleString) return prepString(clip, quot);

	QRegularExpression fieldsep;
	if (sep == SepCustom) fieldsep.setPattern (separator_freefield->text ());
	else if (sep == SepWhitespace) fieldsep.setPattern ("\\s+");
	else if (sep == SepSpace) fieldsep.setPattern (" ");
	else if (sep == SepTab) fieldsep.setPattern ("\t");
	else if (sep == SepComma) fieldsep.setPattern ("\\,");
	else RK_ASSERT (false);

	RKTextMatrix matrix = RKTextMatrix::matrixFromSeparatedValues (clip, fieldsep);
	if (dim == DimVector) {
		// transform list to single row matrix. This is wasteful on resources, but easy to code...
		QStringList list;
		for (int i = 0; i < matrix.numRows (); ++i) {
			list += matrix.getRow (i);
		}
		matrix = RKTextMatrix::matrixFromSeparatedValues (list.join ("\t"));
	}

	if (reverse_h || reverse_v || transpose) matrix = matrix.transformed (reverse_h, reverse_v, transpose);

	QString ret;
	if (dim == DimDataFrame) ret.append("data.frame(");
	if (dim >= DimMatrix) ret.append ("cbind(\n");
	else ret.append ("c(");	// DimVector

	int startcol = rownames ? 1 : 0;
	int startrow = names ? 1 : 0;
	for (int i = startcol; i < matrix.numColumns (); ++i) {
		if (dim >= DimMatrix) {
			if (i != startcol) ret.append ("),\n");
			if (names) {
				ret.append(prepString(matrix.getText(0, i), QuoteAll) + "=c(");
			} else {
				ret.append("c(");
			}
		} else if (i != 0) ret.append (",");

		for (int j = startrow; j < matrix.numRows (); ++j) {
			if (j != startrow) ret.append (",");
			ret.append(prepString(matrix.getText(j, i), quot));
		}
	}
	ret.append (")\n");
	if (dim == DimDataFrame) {
		ret.append(')');
		if (rownames) {
			ret.append(", rownames=c(\n");
			for (int row = startrow; row < matrix.numRows(); ++row) {
				if (row != startrow) ret.append (",");
				ret.append(prepString(matrix.getText(row, 0), QuoteAll));
			}
			ret.append(")\n");
		}
	}
	if (dim >= DimMatrix) ret.append (")\n");

	return (ret);
}

QString RKPasteSpecialDialog::prepString(const QString& src, const Quoting quot) const {
//	RK_TRACE (MISC);

	if (quot == QuoteAll) return (RObject::rQuote (src));
	if (src.isEmpty() && insert_nas_box->isChecked ()) return ("NA");
	if (quot == QuoteNone) return (src);
	RK_ASSERT (quot == QuoteAuto);

	bool numeric = false;
	src.toDouble (&numeric);	// side-effect of setting numeric to true, if number conversion succeeds
	if (!numeric) return (RObject::rQuote (src));
	return src;
}

void RKPasteSpecialDialog::accept() {
	RK_TRACE(MISC);
	if (objectname) {
		RCommand *command = new RCommand(objectname->currentFullName() + " <- " + resultingText(), RCommand::App | RCommand::ObjectListUpdate);
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
