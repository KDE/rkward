/***************************************************************************
                          rkmatrixinput  -  description
                             -------------------
    begin                : Tue Oct 09 2012
    copyright            : (C) 2012 by Thomas Friedrichsmeier
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

#include "rkmatrixinput.h"

#include <QVBoxLayout>

#include "klocale.h"

#include "../misc/xmlhelper.h"

#include "../debug.h"

RKMatrixInput::RKMatrixInput (const QDomElement& element, RKComponent* parent_component, QWidget* parent_widget) : RKComponent (parent_component, parent_widget), QAbstractTableModel () {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Enter data:"), DL_INFO), this);
	vbox->addWidget (label);

	display = new QTableView (this);
	vbox->addWidget (display);

	mode = static_cast<Mode> (xml->getMultiChoiceAttribute (element, "mode", "integer;real;string", 1, DL_WARNING));
	if (mode == Integer) {
		min = xml->getIntAttribute (element, "min", INT_MIN, DL_INFO) - .1;	// we'll only allow ints anyway. Make sure not to run into floating point precision issues.
		max = xml->getIntAttribute (element, "max", INT_MAX, DL_INFO) + .1;
	} else if (mode == Real) {
		min = xml->getDoubleAttribute (element, "min", -FLT_MAX, DL_INFO);
		max = xml->getDoubleAttribute (element, "min", FLT_MAX, DL_INFO);
	} else {
		min = -FLT_MAX;
		max = FLT_MAX;
	}

	allow_missings = xml->getBoolAttribute (element, "allow_missings", false, DL_INFO);
	allow_user_resize_columns = xml->getBoolAttribute (element, "allow_user_resize_columns", true, DL_INFO);
	allow_user_resize_rows = xml->getBoolAttribute (element, "allow_user_resize_rows", true, DL_INFO);
	trailing_rows = allow_user_resize_rows ? 1 :0;
	trailing_columns = allow_user_resize_columns ? 1 :0;

	row_count = new RKComponentPropertyInt (this, false, xml->getIntAttribute (element, "rows", 2, DL_INFO));
	column_count = new RKComponentPropertyInt (this, false, xml->getIntAttribute (element, "columns", 2, DL_INFO));
	tsv_data = new RKComponentPropertyBase (this);
	row_count->setInternal (true);
	addChild (row_count);
	column_count->setInternal (true);
	addChild (column_count);
	addChild (tsv_data);
	connect (row_count, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (dimensionPropertyChanged()));
	connect (column_count, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (dimensionPropertyChanged()));
	connect (tsv_data, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (tsvPropertyChanged()));
	updating_tsv_data = false;
	updating_dimensions = false;

	display->setModel (this);
}


RKMatrixInput::~RKMatrixInput () {
	RK_TRACE (PLUGIN);
}

int RKMatrixInput::columnCount (const QModelIndex& parent) const {
	if (parent.isValid ()) return 0;
	return column_count->intValue () + trailing_columns;
}

int RKMatrixInput::rowCount (const QModelIndex& parent) const {
	if (parent.isValid ()) return 0;
	return row_count->intValue () + trailing_rows;
}

QVariant RKMatrixInput::data (const QModelIndex& index, int role) const {
	if (!index.isValid ()) return QVariant ();

	int row = index.row ();
	int column = index.column ();

	bool trailing = false;
	if (row > row_count->intValue ()) {
		if ((!allow_user_resize_rows) || (row > row_count->intValue () + TRAILING_ROWS)) {
			RK_ASSERT (false);
			return QVariant ();
		}
		trailing = true;
	}
	if (column > column_count->intValue ()) {
		if ((!allow_user_resize_columns) || (column > column_count->intValue () + TRAILING_COLS)) {
			RK_ASSERT (false);
			return QVariant ();
		}
		trailing = true;
	};
	if (trailing) {
		if (role == Qt::BackgroundRole) return QVariant (QBrush (Qt::lightGray));
		if (role == Qt::ToolTipRole || role == Qt::StatusTipRole) return QVariant (i18n ("Type on these cells to expand the table"));
		return QVariant ();
	}
#error TODO
}


#error TODO

#include "rkmatrixinput.moc"
