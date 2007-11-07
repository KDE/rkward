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

#include "../core/rkvariable.h"
#include "../rkglobals.h"
#include "../debug.h"

#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <klocale.h>

EditFormatDialog::EditFormatDialog (QWidget *parent, RKVariable *var, int mode) : QDialog (parent) {
	RK_TRACE (EDITOR);
	RK_ASSERT (var);
//	RK_ASSERT (var->objectOpened ());

	EditFormatDialog::var = var;
	EditFormatDialog::mode = mode;
	EditFormatDialog::options = var->getFormattingOptions ();

	Q3VBoxLayout *vbox = new Q3VBoxLayout (this, RKGlobals::marginHint (), RKGlobals::spacingHint ());
	
	alignment_group = new Q3ButtonGroup (i18n ("Alignment"), this);
	alignment_group->setColumnLayout (0, Qt::Vertical);
	alignment_group->layout()->setSpacing (RKGlobals::spacingHint ());
	alignment_group->layout()->setMargin (RKGlobals::marginHint ());
	Q3VBoxLayout *group_layout = new Q3VBoxLayout (alignment_group->layout());
	group_layout->addWidget (new QRadioButton (i18n ("Default for type '%1'", RObject::typeToText (var->getDataType ())), alignment_group));
	group_layout->addWidget (new QRadioButton (i18n ("Left"), alignment_group));
	group_layout->addWidget (new QRadioButton (i18n ("Right"), alignment_group));
	alignment_group->setButton ((int) RKVariable::FormattingOptions::AlignDefault);
	vbox->addWidget (alignment_group);

	precision_group = new Q3ButtonGroup (i18n ("Decimal Places"), this);
	precision_group->setColumnLayout (0, Qt::Vertical);
	precision_group->layout()->setSpacing (RKGlobals::spacingHint ());
	precision_group->layout()->setMargin (RKGlobals::marginHint ());
	group_layout = new Q3VBoxLayout (precision_group->layout());
	group_layout->addWidget (new QRadioButton (i18n ("Default setting"), precision_group));
	group_layout->addWidget (new QRadioButton (i18n ("As required"), precision_group));
	group_layout->addWidget (new QRadioButton (i18n ("Fixed precision:"), precision_group));
	precision_field = new QSpinBox (0, 10, 1, precision_group);
	connect (precision_field, SIGNAL (valueChanged (int)), this, SLOT (precisionFieldChanged (int)));
	group_layout->addWidget (precision_field);
	precision_group->setButton ((int) RKVariable::FormattingOptions::PrecisionDefault);
	vbox->addWidget (precision_group);

	QPushButton *ok_button = new QPushButton (i18n ("Ok"), this);
	connect (ok_button, SIGNAL (clicked ()), this, SLOT (accept ()));
	vbox->addWidget (ok_button);

	initialize ();

	setCaption (i18n ("Formatting options for '%1'", var->getShortName ()));
}

EditFormatDialog::~EditFormatDialog () {
	RK_TRACE (EDITOR);
}

void EditFormatDialog::initialize () {
	RK_TRACE (EDITOR);

	if (!options) return;
	
	alignment_group->setButton ((int) options->alignment);
	precision_group->setButton ((int) options->precision_mode);
	precision_field->setValue (options->precision);
}

void EditFormatDialog::accept () {
	RK_TRACE (EDITOR);

	RKVariable::FormattingOptions *new_options = new RKVariable::FormattingOptions;
	new_options->alignment = RKVariable::FormattingOptions::AlignDefault;
	new_options->precision_mode = RKVariable::FormattingOptions::PrecisionDefault;
	new_options->precision = 0;
	bool empty = true;
#if QT_VERSION < 0x030200
	int al = alignment_group->id (button_group->selected ());
#else
	int al = alignment_group->selectedId ();
#endif
	if (al != (int) RKVariable::FormattingOptions::AlignDefault) {
		new_options->alignment = (RKVariable::FormattingOptions::Alignment) al;
		empty = false;
	}

#if QT_VERSION < 0x030200
	int prec = precision_group->id (button_group->selected ());
#else
	int prec = precision_group->selectedId ();
#endif
	if (prec != (int) RKVariable::FormattingOptions::PrecisionDefault) {
		empty = false;
		if (prec == (int) RKVariable::FormattingOptions::PrecisionRequired) {
			new_options->precision_mode = RKVariable::FormattingOptions::PrecisionRequired;
		} else {
			new_options->precision_mode = RKVariable::FormattingOptions::PrecisionFixed;
			new_options->precision = precision_field->value ();
		}
	}

	if (empty) {
		delete new_options;
		var->setFormattingOptions (0);
	} else {
		var->setFormattingOptions (new_options);
	}

	QDialog::accept ();
}

void EditFormatDialog::precisionFieldChanged (int) {
	RK_TRACE (EDITOR);

	precision_group->setButton ((int) RKVariable::FormattingOptions::PrecisionFixed);
}

#include "editformatdialog.moc"
