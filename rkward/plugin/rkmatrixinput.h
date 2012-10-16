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

#include <QAbstractTableModel>
#include <QDomElement>

class QTableView;

/** Provides a table for editing one- or two-dimensional data
  *@author Thomas Friedrichsmeier
  */
class RKMatrixInput : public RKComponent, public QAbstractTableModel {
	Q_OBJECT
public:
	RKMatrixInput (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKMatrixInput ();
	int type () { return ComponentMatrixInput; };
	bool isValid () { return is_valid; };
	QString value (const QString &modifier);
private slots:
	void dimensionPropertyChanged (RKComponentPropertyBase *property);
	void tsvPropertyChanged (RKComponentPropertyBase *property);
private:
	RKComponentPropertyInt *row_count;
	RKComponentPropertyInt *column_count;
	RKComponentPropertyBase *tsv_data;

	int rowCount (const QModelIndex &parent = QModelIndex()) const; // implemented for QAbstractTableModel
	int columnCount (const QModelIndex &parent = QModelIndex()) const; // implemented for QAbstractTableModel
	QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const; // re-implemented for QAbstractTableModel
	bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole); // re-implemented for QAbstractTableModel

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

	bool isValueValid (const QString &value) const;
	void updateValidityFlag ();

	bool setCellValue (int row, int column, const QString& value);
	bool setColumnValue (int column, const QString& value);
	bool setTableValue (const QString& value);

	bool is_valid;

	// NOTE: The storage may contain more rows / columns than the current dimensions of the table. This is so that no data gets
	// lost, if a user shrinks a table, accidentally, then re-grows it.
	struct Column {
		int valid_up_to_row;	// to save validizing the entire table on each change, we keep track of validity per column
		QStringList storage;
		QString cached_joined_string;
	};
	QList<Column> columns;

	QTableView *display;
};

#endif
