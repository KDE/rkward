/***************************************************************************
                          rkspecialactions  -  description
                             -------------------
    begin                : Mon Mar 15 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#include "rkspecialactions.h"

#include <klocale.h>

#include "../debug.h"

RKPasteSpecialAction::RKPasteSpecialAction (QObject* parent) : KPasteTextAction (parent) {
	RK_TRACE (MISC);

	setText (i18n ("Paste special..."));
	connect (this, SIGNAL (triggered(bool)), this, SLOT (doSpecialPaste()));
}

RKPasteSpecialAction::~RKPasteSpecialAction () {
	RK_TRACE (MISC);
}

void RKPasteSpecialAction::doSpecialPaste () {
	RK_TRACE (MISC);

	RKPasteSpecialDialog* dialog = new RKPasteSpecialDialog (associatedWidgets ().first ());
	int res = dialog->exec ();
	if (res == QDialog::Accepted) {
		emit (pasteText (dialog->resultingText()));
	}
	dialog->deleteLater ();
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

#include <kvbox.h>
#include <khbox.h>

#include "../dataeditor/rktextmatrix.h"
#include "../core/robject.h"

RKPasteSpecialDialog::RKPasteSpecialDialog (QWidget* parent) : KDialog (parent) {
	RK_TRACE (MISC);

	setCaption (i18n ("Paste special..."));
	setButtons (KDialog::Ok | KDialog::Cancel);

	KVBox* page = new KVBox (this);
	setMainWidget (page);
	KHBox* row = new KHBox (page);

	QGroupBox* box;
	QVBoxLayout* group_layout;
	QHBoxLayout* h_layout;
	QRadioButton* rbutton;

	// Mode box
	box = new QGroupBox (i18n ("Paste mode"), row);
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
	rbutton->setChecked (true);
	group_layout->addWidget (rbutton);
	connect (dimensionality_group, SIGNAL (buttonClicked(int)), this, SLOT (updateState()));

	// Separator box
	box = new QGroupBox (i18n ("Field separator"), row);
	group_layout = new QVBoxLayout (box);
	separator_group = new QButtonGroup (box);
	rbutton = new QRadioButton (i18n ("Tab"), box);
	rbutton->setChecked (true);
	separator_group->addButton (rbutton, SepTab);
#warning TODO: autodetection heuristics
	group_layout->addWidget (rbutton);
	rbutton = new QRadioButton (i18n ("Comma"), box);
	separator_group->addButton (rbutton, SepComma);
	group_layout->addWidget (rbutton);
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
	connect (separator_group, SIGNAL (buttonClicked(int)), this, SLOT (updateState()));

	row = new KHBox (page);

	// Quoting box
	box = new QGroupBox (i18n ("Quoting"), row);
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
	connect (quoting_group, SIGNAL (buttonClicked(int)), this, SLOT (updateState()));

	// further controls
	box = new QGroupBox (i18n ("Transformations"), row);
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
	reverse_v_box->setEnabled (dimensionality == DimMatrix);
	insert_nas_box->setEnabled (dimensionality != DimSingleString);
	transpose_box->setEnabled (dimensionality == DimMatrix);

	separator_freefield->setEnabled ((dimensionality != DimSingleString) && (separator_group->checkedId () == SepCustom));
}

QString RKPasteSpecialDialog::resultingText () {
	RK_TRACE (MISC);

	const int sep = separator_group->checkedId ();		// for easier typing
	const int dim = dimensionality_group->checkedId ();
	const bool reverse_h = reverse_h_box->isChecked () && (dim != DimSingleString);
	const bool reverse_v = reverse_v_box->isChecked () && (dim == DimMatrix);
	const bool transpose = transpose_box->isChecked () && (dim == DimMatrix);

	QString clip;

	const QMimeData* data = QApplication::clipboard ()->mimeData ();
	if ((dim != DimSingleString) && (sep == SepTab) && data->hasFormat ("text/tab-separated-values")) {
		clip = QString::fromLocal8Bit (data->data ("text/tab-separated-values"));
	} else if ((dim != DimSingleString) && (sep == SepComma) && data->hasFormat ("text/comma-separated-values")) {
		clip = QString::fromLocal8Bit (data->data ("text/comma-separated-values"));
	} else {
		clip = data->text ();
	}

	if (dim == DimSingleString) return prepString (clip);

	QRegExp fieldsep;
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
	if (dim == DimMatrix) {
		ret.append ("rbind (\n");
	}

	for (int i = 0; i < matrix.numRows (); ++i) {
		if (i != 0) ret.append ("),\n");
		ret.append ("c(");
		for (int j = 0; j < matrix.numColumns (); ++j) {
			if (j != 0) ret.append (",");
			ret.append (prepString (matrix.getText (i, j)));
		}
		if (i == (matrix.numRows () - 1)) ret.append (")\n");
	}

	if (dim == DimMatrix) {
		ret.append (")\n");
	}

	return (ret);
}

QString RKPasteSpecialDialog::prepString (const QString& src) const {
//	RK_TRACE (MISC);

	const int quot = quoting_group->checkedId ();
	if (quot == QuoteAll) return (RObject::rQuote (src));
	if (src.isEmpty() && insert_nas_box->isChecked ()) return ("NA");
	if (quot == QuoteNone) return (src);
	RK_ASSERT (quot == QuoteAuto);

	bool numeric = false;
	src.toDouble (&numeric);	// side-effect of setting numeric to true, if number conversion succeeds
	if (!numeric) return (RObject::rQuote (src));
	return src;
}

#include "rkspecialactions.moc"
