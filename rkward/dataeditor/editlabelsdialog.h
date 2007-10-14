/***************************************************************************
                          editlabelsdialog  -  description
                             -------------------
    begin                : Tue Sep 21 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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
#ifndef EDITLABELSDIALOG_H
#define EDITLABELSDIALOG_H

#include <kdialog.h>
//Added by qt3to4:
#include <QResizeEvent>

#include "../core/robject.h"

class RKVariable;

#include "twintablemember.h"

/** special mini class provides the table in EditLabelsDialog
TODO: make copy/paste work

@author Thomas Friedrichsmeier
*/
class LevelsTable : public TwinTableMember {
	Q_OBJECT
public:
	LevelsTable (QWidget *parent, RObject::ValueLabels *labels);
	~LevelsTable ();
/** reimplemented form QTable not to add trailing rows/cols if needed */
	QWidget *beginEdit (int row, int col, bool replace);
/** reimplemented form QTable  to work on RObject::ValueLabels instead of QTableItems */
	void paintCell (QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg);
/** reimplemented form QTable to work on RObject::ValueLabels instead of QTableItems */
	void setText (int row, int col, const QString &text);
/** reimplemented form QTable to work on RObject::ValueLabels instead of QTableItems */
	QString text (int row, int col) const;
public slots:
/** cut */
	void cut();
/** copy */
	void copy();
/** paste */
	void paste();
protected:
/** reimplemented to resize the table columns so that there's no unused space to the right */
	void resizeEvent (QResizeEvent *e);
/** reimplemented to resize the table columns so that there's no unused space to the right */
	void columnWidthChanged (int col);
private:
friend class EditLabelsDialog;
	RObject::ValueLabels *storage;
	bool updating_size;
};

/**
Allows editing of value labels / factor levels for an (edited) RKVariable

@author Thomas Friedrichsmeier
*/
class EditLabelsDialog : public KDialog {
	Q_OBJECT
public:
/** constuctor., the variable to work on.
@param parent a QWidget parent (usually 0)
@param var the variable/factor to set labels for
@param mode not yet used */
	EditLabelsDialog (QWidget *parent, RKVariable *var, int mode=0);

	~EditLabelsDialog ();
protected:
/// reimplemented to submit the changes to the backend
	void accept ();
private:
	LevelsTable *table;
	RKVariable *var;
	int mode;
};

#endif
