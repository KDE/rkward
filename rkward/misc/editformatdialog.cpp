/*
editformatdialog - This file is part of RKWard (https://rkward.kde.org). Created: Thu Sep 30 2004
SPDX-FileCopyrightText: 2004 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "editformatdialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <qspinbox.h>
#include <qstringlist.h>

#include <KLocalizedString>

#include "../core/rkvariable.h"
#include "../misc/rkdialogbuttonbox.h"
#include "../misc/rkradiogroup.h"

#include "../debug.h"

EditFormatDialog::EditFormatDialog(QWidget *parent) : QDialog(parent) {
	RK_TRACE(EDITOR);

	QVBoxLayout *layout = new QVBoxLayout(this);

	auto alignment_box = new RKRadioGroup(i18n("Alignment"));
	alignment_group = alignment_box->group();
	layout->addWidget(alignment_box);
	alignment_box->addButton(i18n("Default"), (int)RKVariable::FormattingOptions::AlignDefault)->setChecked(true);
	alignment_box->addButton(i18n("Left"), (int)RKVariable::FormattingOptions::AlignLeft);
	alignment_box->addButton(i18n("Right"), (int)RKVariable::FormattingOptions::AlignRight);

	auto precision_box = new RKRadioGroup(i18n("Decimal Places"));
	precision_group = precision_box->group();
	layout->addWidget(precision_box);
	precision_box->addButton(i18n("Default setting"), (int)RKVariable::FormattingOptions::PrecisionDefault);
	precision_box->addButton(i18n("As required"), (int)RKVariable::FormattingOptions::PrecisionRequired);
	precision_field = new QSpinBox();
	precision_field->setRange(0, 10);
	precision_box->addButton(i18n("Fixed precision:"), (int)RKVariable::FormattingOptions::PrecisionFixed, precision_field);

	RKDialogButtonBox *buttons = new RKDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	layout->addWidget(buttons);
}

EditFormatDialog::~EditFormatDialog() {
	RK_TRACE(EDITOR);
}

void EditFormatDialog::initialize(const RKVariable::FormattingOptions &options, const QString &varname) {
	RK_TRACE(EDITOR);

	setWindowTitle(i18n("Formatting options for '%1'", varname));

	EditFormatDialog::options = options;

	alignment_group->button((int)options.alignment)->setChecked(true);
	precision_group->button((int)options.precision_mode)->setChecked(true);
	precision_field->setValue(options.precision);
}

void EditFormatDialog::accept() {
	RK_TRACE(EDITOR);

	options.alignment = (RKVariable::FormattingOptions::Alignment)alignment_group->checkedId();

	options.precision_mode = (RKVariable::FormattingOptions::Precision)precision_group->checkedId();
	if (options.precision_mode == RKVariable::FormattingOptions::PrecisionFixed) {
		options.precision = precision_field->value();
	} else {
		options.precision = 0;
	}

	QDialog::accept();
}

///////////// EditFormatDialogProxy ////////////////////

EditFormatDialogProxy::EditFormatDialogProxy(QWidget *parent) : QWidget(parent) {
	RK_TRACE(EDITOR);

	dialog = nullptr;
}

EditFormatDialogProxy::~EditFormatDialogProxy() {
	RK_TRACE(EDITOR);
}

void EditFormatDialogProxy::initialize(const RKVariable::FormattingOptions &options, const QString &varname) {
	RK_TRACE(EDITOR);

	if (dialog) return; // one dialog at a time, please!

	EditFormatDialogProxy::options = options;
	dialog = new EditFormatDialog(this);
	dialog->initialize(options, varname);

	connect(dialog, &QDialog::finished, this, &EditFormatDialogProxy::dialogDone);
	QTimer::singleShot(0, dialog, &EditFormatDialog::exec);
}

void EditFormatDialogProxy::dialogDone(int result) {
	RK_TRACE(EDITOR);
	RK_ASSERT(dialog);

	if (result == QDialog::Accepted) {
		options = dialog->options;
		Q_EMIT done(this, RKItemDelegate::EditorExit);
	} else {
		Q_EMIT done(this, RKItemDelegate::EditorReject);
	}
	dialog->deleteLater();
	dialog = nullptr;
}
