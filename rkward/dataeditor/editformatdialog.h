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
#ifndef EDITFORMATDIALOG_H
#define EDITFORMATDIALOG_H

#include <kdialog.h>

#include "../core/rkvariable.h"
#include "twintablemember.h"

class Q3ButtonGroup;
class QSpinBox;

/**
Allows editing of format-attributes for an RKVariable

@author Thomas Friedrichsmeier
*/
class EditFormatDialog : public KDialog {
	Q_OBJECT
public slots:
	void precisionFieldChanged (int);
protected:
/** reimplemented to make the newly selected options available */
	void accept ();

friend class EditFormatDialogProxy;
/** ctor */
	EditFormatDialog (QWidget *parent);
/** dtor */
	~EditFormatDialog ();

/** initializes the GUI-options from the settings for the variable */
	void initialize (const RKVariable::FormattingOptions& options, const QString& varname);
private:
	Q3ButtonGroup *alignment_group;
	Q3ButtonGroup *precision_group;
	QSpinBox *precision_field;
	RKVariable::FormattingOptions options;
};

/** Simple proxy wrapper to allow using a model EditFormatDialog in a QTableView */
class EditFormatDialogProxy : public QWidget {
	Q_OBJECT
public:
	EditFormatDialogProxy (QWidget* parent);
	~EditFormatDialogProxy ();

	void initialize (const RKVariable::FormattingOptions& options, const QString& varname);
	RKVariable::FormattingOptions getOptions () const { return options; };
signals:
	void done (QWidget* widget, RKItemDelegate::EditorDoneReason reason);
protected slots:
	void dialogDone (int result);
private:
	RKVariable::FormattingOptions options;
	EditFormatDialog* dialog;
};

#endif
