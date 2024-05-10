/*
editformatdialog - This file is part of the RKWard project. Created: Thu Sep 30 2004
SPDX-FileCopyrightText: 2004 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef EDITFORMATDIALOG_H
#define EDITFORMATDIALOG_H

#include <QDialog>

#include "../core/rkvariable.h"
#include "rktableview.h"

class QButtonGroup;
class QSpinBox;

/**
Allows editing of format-attributes for an RKVariable

@author Thomas Friedrichsmeier
*/
class EditFormatDialog : public QDialog {
	Q_OBJECT
public Q_SLOTS:
	void precisionFieldChanged (int);
protected:
/** reimplemented to make the newly selected options available */
	void accept () override;

friend class EditFormatDialogProxy;
/** ctor */
	explicit EditFormatDialog(QWidget *parent);
/** dtor */
	~EditFormatDialog ();

/** initializes the GUI-options from the settings for the variable */
	void initialize (const RKVariable::FormattingOptions& options, const QString& varname);
private:
	QButtonGroup *alignment_group;
	QButtonGroup *precision_group;
	QSpinBox *precision_field;
	RKVariable::FormattingOptions options;
};

/** Simple proxy wrapper to allow using a model EditFormatDialog in a QTableView */
class EditFormatDialogProxy : public QWidget {
	Q_OBJECT
public:
	explicit EditFormatDialogProxy (QWidget* parent);
	~EditFormatDialogProxy ();

	void initialize (const RKVariable::FormattingOptions& options, const QString& varname);
	RKVariable::FormattingOptions getOptions () const { return options; };
Q_SIGNALS:
	void done (QWidget* widget, RKItemDelegate::EditorDoneReason reason);
protected Q_SLOTS:
	void dialogDone (int result);
private:
	RKVariable::FormattingOptions options;
	EditFormatDialog* dialog;
};

#endif
