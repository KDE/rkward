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

#include <qdialog.h>

#include "../core/rkvariable.h"

class Q3ButtonGroup;
class QSpinBox;

/**
Allows editing of format-attributes for an (edited) RKVariable

@author Thomas Friedrichsmeier
*/
class EditFormatDialog : public QDialog {
Q_OBJECT
public:
	EditFormatDialog (QWidget *parent, RKVariable *var, int mode=0);

	~EditFormatDialog ();
public slots:
	void precisionFieldChanged (int);
protected:
/// reimplemented to submit the changes to the backend
	void accept ();
private:
	Q3ButtonGroup *alignment_group;
	Q3ButtonGroup *precision_group;
	QSpinBox *precision_field;
	RKVariable::FormattingOptions *options;
	
	RKVariable *var;
	int mode;

/** initializes the GUI-options from the settings for the variable */
	void initialize ();
};

#endif
