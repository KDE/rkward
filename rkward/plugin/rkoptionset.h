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
private slots:
	void governingPropertyChanged (RKComponentPropertyBase *property);
	void columnPropertyChanged (RKComponentPropertyBase *property);
	void currentRowPropertyChanged (RKComponentPropertyBase *property);
	void addRow ();
	void removeRow ();
	void updateContents ();
	void currentRowChanged (QTreeWidgetItem *item);
private:
	void initDisplay ();
	void updateVisuals ();

	QMap<QString, QString> content_defaults;
/** for option sets which are "driven" (i.e. the user cannot simply add / remove rows, directly), this holds the key column, controlling addition / removal of rows in the set.
  * if this length (or order) is changed in this row, it will also be changed in the other rows. */
	RKComponentPropertyStringList *keycolumn;
	QStringList old_keys;
	QMultiMap<RKComponentPropertyBase *, RKComponentPropertyStringList *> columns_to_update;
	struct ColumnInfo {
		QString column_name;
		QString column_label;
		QString governor;
		QString governor_modifier;
		QString default_value;
		int display_index;
		bool restorable;
	};
	QMap<RKComponentPropertyStringList *, ColumnInfo> column_map;
	RKComponent *contents_container;
	QTreeWidget *display;
	QWidget *display_buttons;
	QPushButton *remove_button;
	QPushButton *add_button;
	bool display_show_index;
	RKComponentPropertyInt *current_row;
	RKComponentPropertyInt *row_count;
	QTimer update_timer;

	int min_rows;
	int min_rows_if_any;
	int max_rows;

	bool updating_from_contents;
	bool updating_from_storage;
	QSet<RKComponentPropertyStringList *> columns_which_have_been_updated_externally;
};

#endif
