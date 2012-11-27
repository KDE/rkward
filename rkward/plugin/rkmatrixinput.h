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

#include "../dataeditor/twintablemember.h"

#include <QDomElement>
#include <QStringList>

class RKTableView;
class RKMatrixInputModel;

/** Provides a table for editing one- or two-dimensional data
  *@author Thomas Friedrichsmeier
  */
class RKMatrixInput : public RKComponent {
	Q_OBJECT
public:
	RKMatrixInput (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKMatrixInput ();
	int type () { return ComponentMatrixInput; };
	bool isValid () { return is_valid; };
	QVariant value (const QString &modifier=QString ());
public slots:
	void cut ();
	void copy ();
	void paste ();
	void clearSelectedCells ();
private slots:
	void dimensionPropertyChanged (RKComponentPropertyBase *property);
	void tsvPropertyChanged ();
private:
	friend class RKMatrixInputModel;
	RKComponentPropertyInt *row_count;
	RKComponentPropertyInt *column_count;
	RKComponentPropertyBase *tsv_data;

	enum Mode {
		Integer=0,
		Real,
		String
	} mode;

	double min;
	double max;

	bool allow_missings;
	bool allow_user_resize_rows;
	bool allow_user_resize_columns;
	int trailing_rows;
	int trailing_columns;

	bool isValueValid (const QString &value) const;
	void updateAll ();

	void setCellValue (int row, int column, const QString& value);
	QString cellValue (int row, int column) const;
	void setColumnValue (int column, const QString& value);
	void updateColumn (int column);
	bool expandStorageForColumn (int column);
	QString makeColumnString (int column, const QString& sep, bool r_pasteable = true);
	bool isColumnValid (int column);

	bool is_valid;

	// NOTE: The storage may contain more rows / columns than the current dimensions of the table. This is so that no data gets
	// lost, if a user shrinks a table, accidentally, then re-grows it.
	struct Column {
		int last_valid_row;	// to save validizing the entire table on each change, we keep track of validity per column
		QStringList storage;
		QString cached_tab_joined_string;
	};
	QList<Column> columns;

	RKTableView *display;
	RKMatrixInputModel *model;

	// to avoid recursion:
	bool updating_tsv_data;
};

#include <QAbstractTableModel>

class RKMatrixInputModel : public QAbstractTableModel {
private:
friend class RKMatrixInput;
	RKMatrixInputModel (RKMatrixInput *matrix);
	virtual ~RKMatrixInputModel ();
	int rowCount (const QModelIndex &parent = QModelIndex()) const; // implemented for QAbstractTableModel
	int columnCount (const QModelIndex &parent = QModelIndex()) const; // implemented for QAbstractTableModel
	QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const; // re-implemented for QAbstractTableModel
	bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole); // re-implemented for QAbstractTableModel
	Qt::ItemFlags flags (const QModelIndex &index) const; // re-implemented for QAbstractTableModel
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;  // re-implemented for QAbstractTableModel

	RKMatrixInput *matrix;
	QStringList horiz_header;
	QStringList vert_header;
};

#endif
