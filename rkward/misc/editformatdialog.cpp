/*
editformatdialog - This file is part of RKWard (https://rkward.kde.org). Created: Thu Sep 30 2004
SPDX-FileCopyrightText: 2004 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "editformatdialog.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <QVBoxLayout>
#include <QTimer>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>

#include "../core/rkvariable.h"
#include "../misc/rkdialogbuttonbox.h"

#include "../debug.h"

EditFormatDialog::EditFormatDialog (QWidget *parent) : QDialog (parent) {
	RK_TRACE (EDITOR);

	QVBoxLayout *layout = new QVBoxLayout (this);

	alignment_group = new QButtonGroup (this);
	QGroupBox* alignment_box = new QGroupBox (i18n ("Alignment"), this);
	layout->addWidget (alignment_box);
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
	QGroupBox* precision_box = new QGroupBox (i18n ("Decimal Places"), this);
	layout->addWidget (precision_box);
	group_layout = new QVBoxLayout (precision_box);
	precision_group->addButton (button = new QRadioButton (i18n ("Default setting"), precision_box), (int) RKVariable::FormattingOptions::PrecisionDefault);
	group_layout->addWidget (button);
	precision_group->addButton (button = new QRadioButton (i18n ("As required"), precision_box), (int) RKVariable::FormattingOptions::PrecisionRequired);
	group_layout->addWidget (button);
	precision_group->addButton (button = new QRadioButton (i18n ("Fixed precision:"), precision_box), (int) RKVariable::FormattingOptions::PrecisionFixed);
	group_layout->addWidget (button);
	precision_field = new QSpinBox (precision_box);
	precision_field->setRange (0, 10);
	connect (precision_field, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &EditFormatDialog::precisionFieldChanged);
	group_layout->addWidget (precision_field);
	precision_group->button ((int) RKVariable::FormattingOptions::PrecisionDefault)->setChecked (true);

	RKDialogButtonBox *buttons = new RKDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	layout->addWidget (buttons);
}

EditFormatDialog::~EditFormatDialog () {
	RK_TRACE (EDITOR);
}

void EditFormatDialog::initialize (const RKVariable::FormattingOptions& options, const QString& varname) {
	RK_TRACE (EDITOR);

	setWindowTitle (i18n ("Formatting options for '%1'", varname));

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

	QDialog::accept ();
}

void EditFormatDialog::precisionFieldChanged (int) {
	RK_TRACE (EDITOR);

	precision_group->button ((int) RKVariable::FormattingOptions::PrecisionFixed)->setChecked (true);
}


///////////// EditFormatDialogProxy ////////////////////

EditFormatDialogProxy::EditFormatDialogProxy (QWidget* parent) : QWidget (parent) {
	RK_TRACE (EDITOR);

	dialog = nullptr;
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

	connect (dialog, &QDialog::finished, this, &EditFormatDialogProxy::dialogDone);
	QTimer::singleShot(0, dialog, &EditFormatDialog::exec);
}

void EditFormatDialogProxy::dialogDone (int result) {
	RK_TRACE (EDITOR);
	RK_ASSERT (dialog);

	if (result == QDialog::Accepted) {
		options = dialog->options;
		Q_EMIT done(this, RKItemDelegate::EditorExit);
	} else {
		Q_EMIT done(this, RKItemDelegate::EditorReject);
	}
	dialog->deleteLater ();
	dialog = nullptr;
}

