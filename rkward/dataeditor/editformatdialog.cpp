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

#include <q3buttongroup.h>
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
	
	alignment_group = new Q3ButtonGroup (i18n ("Alignment"), vbox);
	alignment_group->setColumnLayout (0, Qt::Vertical);
	alignment_group->layout()->setSpacing (RKGlobals::spacingHint ());
	alignment_group->layout()->setMargin (RKGlobals::marginHint ());
	QVBoxLayout *group_layout = new QVBoxLayout (alignment_group->layout());
	group_layout->setContentsMargins (0, 0, 0, 0);
	group_layout->addWidget (new QRadioButton (i18n ("Default"), alignment_group));
	group_layout->addWidget (new QRadioButton (i18n ("Left"), alignment_group));
	group_layout->addWidget (new QRadioButton (i18n ("Right"), alignment_group));
	alignment_group->setButton ((int) RKVariable::FormattingOptions::AlignDefault);

	precision_group = new Q3ButtonGroup (i18n ("Decimal Places"), vbox);
	precision_group->setColumnLayout (0, Qt::Vertical);
	precision_group->layout()->setSpacing (RKGlobals::spacingHint ());
	precision_group->layout()->setMargin (RKGlobals::marginHint ());
	group_layout = new QVBoxLayout (precision_group->layout());
	group_layout->setContentsMargins (0, 0, 0, 0);
	group_layout->addWidget (new QRadioButton (i18n ("Default setting"), precision_group));
	group_layout->addWidget (new QRadioButton (i18n ("As required"), precision_group));
	group_layout->addWidget (new QRadioButton (i18n ("Fixed precision:"), precision_group));
	precision_field = new QSpinBox (0, 10, 1, precision_group);
	connect (precision_field, SIGNAL (valueChanged (int)), this, SLOT (precisionFieldChanged (int)));
	group_layout->addWidget (precision_field);
	precision_group->setButton ((int) RKVariable::FormattingOptions::PrecisionDefault);

	setButtons (KDialog::Ok | KDialog::Cancel);
}

EditFormatDialog::~EditFormatDialog () {
	RK_TRACE (EDITOR);
}

void EditFormatDialog::initialize (const RKVariable::FormattingOptions& options, const QString& varname) {
	RK_TRACE (EDITOR);

	setCaption (i18n ("Formatting options for '%1'", varname));

	EditFormatDialog::options = options;

	alignment_group->setButton ((int) options.alignment);
	precision_group->setButton ((int) options.precision_mode);
	precision_field->setValue (options.precision);
}

void EditFormatDialog::accept () {
	RK_TRACE (EDITOR);

	options.alignment = (RKVariable::FormattingOptions::Alignment) alignment_group->selectedId ();

	options.precision_mode = (RKVariable::FormattingOptions::Precision) precision_group->selectedId ();
	if (options.precision_mode == RKVariable::FormattingOptions::PrecisionFixed) {
		options.precision = precision_field->value ();
	} else {
		options.precision = 0;
	}

	KDialog::accept ();
}

void EditFormatDialog::precisionFieldChanged (int) {
	RK_TRACE (EDITOR);

	precision_group->setButton ((int) RKVariable::FormattingOptions::PrecisionFixed);
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
