/***************************************************************************
                          editformatdialog  -  description
                             -------------------
    begin                : Thu Sep 30 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "editformatdialog.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <QVBoxLayout>
#include <QTimer>

#include <klocale.h>
#include <kvbox.h>

#include "../core/rkvariable.h"
#include "../rkglobals.h"
#include "../debug.h"

EditFormatDialog::EditFormatDialog (QWidget *parent) : KDialog (parent) {
	RK_TRACE (EDITOR);

	KVBox *vbox = new KVBox ();
	setMainWidget (vbox);

	alignment_group = new QButtonGroup (this);
	QGroupBox* alignment_box = new QGroupBox (i18n ("Alignment"), vbox);
	QVBoxLayout* group_layout = new QVBoxLayout (alignment_box);
	group_layout->setContentsMargins (0, 0, 0, 0);
	QRadioButton* button;
	alignment_group->addButton (button = new QRadioButton (i18n ("Default"), alignment_box), (int) RKVariable::FormattingOptions::AlignDefault);
	group_layout->addWidget (button);
	alignment_group->addButton (button = new QRadioButton (i18n ("Left"), alignment_box), (int) RKVariable::FormattingOptions::AlignLeft);
	group_layout->addWidget (button);
	alignment_group->addButton (button = new QRadioButton (i18n ("Right"), alignment_box), (int) RKVariable::FormattingOptions::AlignRight);
	group_layout->addWidget (button);
	alignment_group->button ((int) RKVariable::FormattingOptions::AlignDefault)->setChecked (true);

	precision_group = new QButtonGroup (this);
	QGroupBox* precision_box = new QGroupBox (i18n ("Decimal Places"), vbox);
	group_layout = new QVBoxLayout (precision_box);
	precision_group->addButton (button = new QRadioButton (i18n ("Default setting"), precision_box), (int) RKVariable::FormattingOptions::PrecisionDefault);
	group_layout->addWidget (button);
	precision_group->addButton (button = new QRadioButton (i18n ("As required"), precision_box), (int) RKVariable::FormattingOptions::PrecisionRequired);
	group_layout->addWidget (button);
	precision_group->addButton (button = new QRadioButton (i18n ("Fixed precision:"), precision_box), (int) RKVariable::FormattingOptions::PrecisionFixed);
	group_layout->addWidget (button);
	precision_field = new QSpinBox (precision_box);
	precision_field->setRange (0, 10);
	connect (precision_field, SIGNAL (valueChanged (int)), this, SLOT (precisionFieldChanged (int)));
	group_layout->addWidget (precision_field);
	precision_group->button ((int) RKVariable::FormattingOptions::PrecisionDefault)->setChecked (true);

	setButtons (KDialog::Ok | KDialog::Cancel);
}

EditFormatDialog::~EditFormatDialog () {
	RK_TRACE (EDITOR);
}

void EditFormatDialog::initialize (const RKVariable::FormattingOptions& options, const QString& varname) {
	RK_TRACE (EDITOR);

	setCaption (i18n ("Formatting options for '%1'", varname));

	EditFormatDialog::options = options;

	alignment_group->button ((int) options.alignment)->setChecked (true);
	precision_group->button ((int) options.precision_mode)->setChecked (true);
	precision_field->setValue (options.precision);
}

void EditFormatDialog::accept () {
	RK_TRACE (EDITOR);

	options.alignment = (RKVariable::FormattingOptions::Alignment) alignment_group->checkedId ();

	options.precision_mode = (RKVariable::FormattingOptions::Precision) precision_group->checkedId ();
	if (options.precision_mode == RKVariable::FormattingOptions::PrecisionFixed) {
		options.precision = precision_field->value ();
	} else {
		options.precision = 0;
	}

	KDialog::accept ();
}

void EditFormatDialog::precisionFieldChanged (int) {
	RK_TRACE (EDITOR);

	precision_group->button ((int) RKVariable::FormattingOptions::PrecisionFixed)->setChecked (true);
}


///////////// EditFormatDialogProxy ////////////////////

EditFormatDialogProxy::EditFormatDialogProxy (QWidget* parent) : QWidget (parent) {
	RK_TRACE (EDITOR);

	dialog = 0;
}

EditFormatDialogProxy::~EditFormatDialogProxy () {
	RK_TRACE (EDITOR);
}

void EditFormatDialogProxy::initialize (const RKVariable::FormattingOptions& options, const QString& varname) {
	RK_TRACE (EDITOR);

	if (dialog) return;	// one dialog at a time, please!

	EditFormatDialogProxy::options = options;
	dialog = new EditFormatDialog (this);
	dialog->initialize (options, varname);

	connect (dialog, SIGNAL (finished(int)), this, SLOT (dialogDone(int)));
	QTimer::singleShot (0, dialog, SLOT (exec()));
}

void EditFormatDialogProxy::dialogDone (int result) {
	RK_TRACE (EDITOR);
	RK_ASSERT (dialog);

	if (result == QDialog::Accepted) {
		options = dialog->options;
		emit (done (this, RKItemDelegate::EditorExit));
	} else {
		emit (done (this, RKItemDelegate::EditorReject));
	}
	dialog->deleteLater ();
	dialog = 0;
}

#include "editformatdialog.moc"
