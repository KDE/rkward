/***************************************************************************
                          rkoptionset  -  description
                             -------------------
    begin                : Mon Oct 31 2011
    copyright            : (C) 2011, 2012 by Thomas Friedrichsmeier
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

#ifndef RKOPTIONSET_H
#define RKOPTIONSET_H

#include <rkcomponent.h>

#include <qmap.h>
#include <QDomElement>
#include <QTimer>
#include <QSet>

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

/** An RKOptionSet provides a group of options for an arbitrary number of "rows". E.g. different line colors for each of a group of variables.
 * 
 * TODO
 * - serialization / de-serialization. We will need to make RKComponentBase::fetchPropertyValuesRecursive() and RKComponent::setPropertyValues() virtual, and reimplement them.
 * 
  *@author Thomas Friedrichsmeier
  */
class RKOptionSet : public RKComponent {
	Q_OBJECT
public:
	RKOptionSet (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKOptionSet ();
	int type () { return ComponentOptionSet; };
	RKComponent *createDisplay (bool show_index, QWidget *parent);
	bool isValid ();
	/** reimplemented from RKComponent */
	ComponentStatus recursiveStatus ();
	/** reimplemented from RKComponent */
	void changed ();
private slots:
	void governingPropertyChanged (RKComponentPropertyBase *property);
	void columnPropertyChanged (RKComponentPropertyBase *property);
	void currentRowPropertyChanged (RKComponentPropertyBase *property);
	void addRow ();
	void removeRow ();
	void updateStatusAndDisplay ();
	void currentRowChanged (QTreeWidgetItem *item);
private:
	void initDisplay ();
	void updateVisuals ();
	int rowCount () const { return row_count->intValue (); };
	void setRowState (int row, bool finished, bool valid);
	void storeRowSerialization (int row);

	RKComponentPropertyInt *current_row;
	RKComponentPropertyInt *row_count;

/** for option sets which are "driven" (i.e. the user cannot simply add / remove rows, directly), this holds the key column, controlling addition / removal of rows in the set.
  * if this length (or order) is changed in this row, it will also be changed in the other rows. */
	RKComponentPropertyStringList *keycolumn;

	/** Map of properties (in the contents region) to columns which need to be updated, when the property changes. */
	QMultiMap<RKComponentPropertyBase *, RKComponentPropertyStringList *> columns_to_update;
	struct ColumnInfo {
		QString column_name;
		QString column_label;
		QString governor;
		QString governor_modifier;
		QString default_value;
		int display_index;
		bool restorable;
		QStringList old_values;
	};
	/** Map of all columns to their meta info */
	QMap<RKComponentPropertyStringList *, ColumnInfo> column_map;
	struct RowInfo {
		RowInfo () : valid (false), finished (false) {};
		bool valid;		/**< has finished processing and is known to be valid */
		bool finished;	/**< has finished processing */
		QMap<QString, QString> full_row_serialization;	/**< complete serialization of this row, (see RKComponent::fetchPropertyValuesRecursive()) */
	};
	QList<RowInfo> rows;
	QMap<QString, QString> default_row_state;
	int n_unfinished_rows, n_invalid_rows;
	int active_row;

	RKComponent *contents_container;
	QTreeWidget *display;
	QWidget *display_buttons;
	QPushButton *remove_button;
	QPushButton *add_button;
	bool display_show_index;
	QTimer update_timer;

	int min_rows;
	int min_rows_if_any;
	int max_rows;

	bool updating_from_contents;
	bool updating_from_storage;
/** When keys in the key column change, all other columns have to be updated, accordingly. */
	void handleKeycolumnUpdate ();
/** Sets the contents from the values in given row */
	void setContentsForRow (int row);
/** Columns which have already been updated before an update of the keycolumn, and thus do not need re-sorting, if the keycolumn is updated */
	QSet<RKComponentPropertyStringList *> columns_which_have_been_updated_externally;

/** get the default value for the given column, row. */
	friend QString getDefaultValue (const ColumnInfo& ci, int row);
};

#endif
