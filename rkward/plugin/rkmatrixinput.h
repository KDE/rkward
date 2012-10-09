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

#ifndef RKMATRIXINPUT_H
#define RKMATRIXINPUT_H

#include <rkcomponent.h>

#include <QDomElement>

/** Provides a table for editing one- or two-dimensional data
  *@author Thomas Friedrichsmeier
  */
class RKMatrixInput : public RKComponent {
	Q_OBJECT
public:
	RKMatrixInput (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKMatrixInput ();
	int type () { return ComponentMatrixInput; };
	bool isValid ();
private slots:
	void dimensionPropertyChanged (RKComponentPropertyBase *property);
private:
	RKComponentPropertyInt *row_count;
	RKComponentPropertyInt *column_count;

	enum Mode {
		Integer,
		Real,
		String
	};

	double min;
	double max;

	bool allow_missings;
	bool allow_user_resize_rows;
	bool allow_user_resize_columns;
};

#endif
