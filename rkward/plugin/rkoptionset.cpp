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

#include "rkoptionset.h"

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QPushButton>

#include <klocale.h>
#include <kvbox.h>

#include "rkstandardcomponent.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardicons.h"
#include "../misc/xmlhelper.h"

#include "../debug.h"

RKOptionSet::RKOptionSet (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = XMLHelper::getStaticHelper ();
	updating_from_contents = updating_from_storage = false;
	connect (&update_timer, SIGNAL (timeout()), this, SLOT (updateContents()));
	update_timer.setSingleShot (true);
	update_timer.setInterval (0);

	min_rows = xml->getIntAttribute (element, "min_rows", 0, DL_INFO);
	min_rows_if_any = xml->getIntAttribute (element, "min_rows_if_any", 1, DL_INFO);
	max_rows = xml->getIntAttribute (element, "max", INT_MAX, DL_INFO);

	// create some meta properties
	current_row = new RKComponentPropertyInt (this, false, -1);
	current_row->setInternal (true);
	addChild ("current_row", current_row);		// NOTE: read-write
	connect (current_row, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (currentRowPropertyChanged(RKComponentPropertyBase*)));
	row_count = new RKComponentPropertyInt (this, false, 0);
	row_count->setInternal (true);
	addChild ("row_count", row_count);		// NOTE: read-only

	// first build the contents, as we will need to refer to the elements inside, later
	QVBoxLayout *layout = new QVBoxLayout (this);
	KVBox *contents_box = new KVBox (this);
	layout->addWidget (contents_box);

	display = 0;	// will be created from the builder, on demand -> createDisplay ()
	contents_container = new RKComponent (this, contents_box);
	RKComponentBuilder *builder = new RKComponentBuilder (contents_container, QDomElement ());
	builder->buildElement (xml->getChildElement (element, "content", DL_ERROR), contents_box, false);	// NOTE that parent widget != parent component, here, by intention. The point is that the display should not be disabled along with the contents
	builder->makeConnections ();
	addChild ("contents", contents_container);

	// create columns
	XMLChildList options = xml->getChildElements (element, "option", DL_WARNING);

	QStringList visible_column_labels ("#");	// Optionally hidden first row for index
	for (int i = 0; i < options.size (); ++i) {
		const QDomElement &e = options.at (i);
		QString id = xml->getStringAttribute (e, "id", QString (), DL_ERROR);
		QString label = xml->getStringAttribute (e, "label", QString (), DL_DEBUG);
		QString governor = xml->getStringAttribute (e, "connect", QString (), DL_INFO);
		bool restorable = xml->getBoolAttribute (e, "restorable", true, DL_INFO);

		while (child_map.contains (id)) {
			RK_DO (qDebug ("optionset already contains a property named %s. Renaming to _%s", qPrintable (id), qPrintable (id)), PLUGIN, DL_ERROR);
			id = "_" + id;
		}

		ColumnInfo col_inf;
		col_inf.column_name = id;
		col_inf.restorable = restorable;
		col_inf.governor = governor;
#warning TODO: Do we have to wait for the parent component to settle before (re-)fetching defaults?
		if (e.hasAttribute ("default")) col_inf.default_value = xml->getStringAttribute (e, "default", QString (), DL_ERROR);
		else if (!governor.isEmpty ()) col_inf.default_value = contents_container->fetchStringValue (governor);
		if (!label.isEmpty ()) {
			col_inf.display_index = visible_column_labels.size ();
			col_inf.column_label = label;
			visible_column_labels.append (label);
		} else {
			col_inf.display_index = -1;
		}

		RKComponentPropertyStringList *column_property = new RKComponentPropertyStringList (this, false);
		addChild (id, column_property);
		connect (column_property, SIGNAL (valueChanged(RKComponentPropertyBase *)), this, SLOT (columnPropertyChanged(RKComponentPropertyBase *)));
		column_map.insert (column_property, col_inf);
	}

	keycolumn = 0;
	QString keycol = xml->getStringAttribute (element, "keycolumn", QString (), DL_DEBUG);
	if (!keycol.isEmpty ()) {
		keycolumn = static_cast<RKComponentPropertyStringList*> (child_map.value (keycol));
		if (!column_map.contains (keycolumn)) {
			RK_DO (qDebug ("optionset does not contain a column named %s. Falling back to manual insertion mode", qPrintable (keycol)), PLUGIN, DL_ERROR);
			keycolumn = 0;
		}
	}

	QMap<RKComponentPropertyStringList *, ColumnInfo>::iterator it = column_map.begin ();
	for (; it != column_map.end (); ++it) {
		ColumnInfo &ci = it.value ();
		if (!ci.governor.isEmpty ()) {		// there *can* be columns without governor for driven or connected option sets
			// Establish connections between columns and their respective governors. Since the format differs, the connection is done indirectly, through this component.
			// So, here, we set up a map of properties to columns, and connect to the change signals.
			RKComponentBase *governor = contents_container->lookupComponent (ci.governor, &ci.governor_modifier);
			if (governor && governor->isProperty ()) {
				RKComponentPropertyBase *gov_prop = static_cast<RKComponentPropertyBase*> (governor);
				if (ci.restorable) {
					if (!ci.governor_modifier.isEmpty ()) {
						RK_DO (qDebug ("Cannot connect restorable column '%s' in optionset to property with modifier (%s). Add an auxiliary non-restorable column, instead.", qPrintable (ci.column_name), qPrintable (ci.governor)), PLUGIN, DL_ERROR);
						continue;
					} else if (gov_prop->isInternal ()) {
						RK_DO (qDebug ("Cannot connect restorable column '%s' in optionset to property (%s), which is marked 'internal'. Add an auxiliary non-restorable column, instead.", qPrintable (ci.column_name), qPrintable (ci.governor)), PLUGIN, DL_ERROR);
						continue;
					}
				}
				columns_to_update.insertMulti (gov_prop, it.key ());
				connect (gov_prop, SIGNAL (valueChanged(RKComponentPropertyBase *)), this, SLOT (governingPropertyChanged(RKComponentPropertyBase *)));
			} else {
				RK_DO (qDebug ("did not find governing property %s for column %s of optionset", qPrintable (ci.governor), qPrintable (ci.column_name)), PLUGIN, DL_ERROR);
			}
		}
	}

	if (display) {		// may or may not have been created
		display->setColumnCount (visible_column_labels.size ());
		display->setHeaderLabels (visible_column_labels);
		display->setItemsExpandable (false);
		display->setRootIsDecorated (false);
		if (display_show_index) display->resizeColumnToContents (0);
		else display->setColumnHidden (0, true);
		connect (display, SIGNAL (currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT (currentRowChanged (QTreeWidgetItem*)));

		if (keycolumn) display_buttons->setVisible (false);
		else {
			connect (add_button, SIGNAL (clicked()), this, SLOT (addRow()));
			connect (remove_button, SIGNAL (clicked()), this, SLOT (removeRow()));
		}
	}

	update_timer.start ();
}

RKOptionSet::~RKOptionSet () {
	RK_TRACE (PLUGIN);
}

RKComponent *RKOptionSet::createDisplay (bool show_index, QWidget *parent) {
	RK_TRACE (PLUGIN);

	RKComponent* dummy = new RKComponent (this, parent);
	QVBoxLayout *layout = new QVBoxLayout (dummy);
	layout->setContentsMargins (0, 0, 0, 0);
	KHBox *box = new KHBox (dummy);
	layout->addWidget (box);

	if (display) {
		RK_DO (qDebug ("cannot create more than one optiondisplay per optionset"), PLUGIN, DL_ERROR);
	} else {
		display = new QTreeWidget (box);
		display_show_index = show_index;
	}

	display_buttons = new KHBox (dummy);
	layout->addWidget (display_buttons);
	add_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionInsertRow), QString (), display_buttons);
	RKCommonFunctions::setTips (i18n ("Add a row / element"), add_button);
	remove_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteRow), QString (), display_buttons);
	RKCommonFunctions::setTips (i18n ("Remove a row / element"), remove_button);

	return (dummy);
}

void RKOptionSet::addRow () {
	RK_TRACE (PLUGIN);

	int row = current_row->intValue () + 1;	// append feels more natural than insert, here
	if (row <= 0) row = row_count->intValue ();

	QMap<RKComponentPropertyStringList *, ColumnInfo>::const_iterator it = column_map.constBegin ();
	for (; it != column_map.constEnd (); ++it) {
		RKComponentPropertyStringList* col = it.key ();
		const ColumnInfo &column = it.value ();
		QStringList values = col->values ();
		values.insert (row, getDefaultValue (column, row));
		col->setValues (values);
	}

	current_row->setIntValue (row);
}

void RKOptionSet::removeRow () {
	RK_TRACE (PLUGIN);

	int row = current_row->intValue ();
	int nrow = row_count->intValue ();
	if (row < 0) {
		RK_ASSERT (false);
		return;
	}

	QMap<RKComponentPropertyStringList *, ColumnInfo>::const_iterator it = column_map.constBegin ();
	for (; it != column_map.constEnd (); ++it) {
		RKComponentPropertyStringList* col = it.key ();
		QStringList values = col->values ();
		values.removeAt (row);
		col->setValues (values);
	}

	--row;
	if ((row < 0) && (nrow > 1)) row = 0;
	current_row->setIntValue (row);
}

QString getDefaultValue (const RKOptionSet::ColumnInfo& ci, int row) {
	// let's not trace this simple helper fun
	Q_UNUSED (row);
#warning TODO: should also allow scripted defaults
	return ci.default_value;
}

// This function is called when a property of the current row of the optionset changes
void RKOptionSet::governingPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (updating_from_storage) return;
	updating_from_contents = true;

	int row = current_row->intValue ();
	if (row < 0) {
		RK_ASSERT (false);
		return;
	}

	QList<RKComponentPropertyStringList *> cols = columns_to_update.values (property);
	QTreeWidgetItem *display_item = 0;
	if (display) display_item = display->topLevelItem (row);
	for (int i = 0; i < cols.size (); ++i) {
		RKComponentPropertyStringList *target = cols.at (i);
		const ColumnInfo &inf = column_map[target];
		QString value = property->value (inf.governor_modifier);
		target->setValueAt (row, value);

		if (display_item && (inf.display_index >= 0)) {
			display_item->setText (inf.display_index, value);
		}

		if (target == keycolumn) old_keys = keycolumn->values ();
	}

	updating_from_contents = false;
}

// This function is called, when a column of the set is changed, typically from external logic
void RKOptionSet::columnPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (updating_from_contents) return;
	int activate_row = current_row->intValue ();
	
	RKComponentPropertyStringList *target = static_cast<RKComponentPropertyStringList *> (property);
	RK_ASSERT (column_map.contains (target));
	const ColumnInfo &inf = column_map[target];
	if (display && (inf.display_index >= 0)) {
		QStringList values = target->values ();
		while (display->topLevelItemCount () != values.size ()) {
			// row count has changed. Ultimately, this means, all values will be updated, so a crude adjustment
			// of row count is good enough, here
			if (values.size () > display->topLevelItemCount ()) {
				display->addTopLevelItem (new QTreeWidgetItem (QStringList ()));
				activate_row = values.size () - 1;
			} else {
				delete (display->takeTopLevelItem (0));
				activate_row = qMax (values.size (), activate_row);
			}
		}
		for (int row = 0; row < values.size (); ++row) {
			QTreeWidgetItem *item = display->topLevelItem (row);
			if (item) item->setText (inf.display_index, values[row]);
			else RK_ASSERT (false);
		}
	}
	if (inf.restorable) update_timer.start ();

	if (target == keycolumn) {
		QStringList new_keys = target->values ();;
		QMap<int, int> position_changes;

		for (int new_pos = 0; new_pos < new_keys.size (); ++new_pos) {
			QString key = new_keys[new_pos];
			if (old_keys.value (new_pos) != key) {	// NOTE: old_keys could be shorter than new_keys!
				int old_pos = old_keys.indexOf (key);	// NOTE: -1 for key no longer present
				position_changes.insert (new_pos, old_pos);
			}
		}

		if (position_changes.isEmpty () && (old_keys.size () == new_keys.size ())) {
			columns_which_have_been_updated_externally.clear ();
			return;	// no change
		}

		QMap<RKComponentPropertyStringList *, ColumnInfo>::const_iterator it = column_map.constBegin ();
		for (; it != column_map.constEnd (); ++it) {
			RKComponentPropertyStringList* col = it.key ();
			const ColumnInfo &column = it.value ();
			if (columns_which_have_been_updated_externally.contains (col) || col == keycolumn) {
				continue;
			}

			// Ok, we'll have to adjust this column. We start by copying the old values, and padding to the
			// new length (if that is greater than the old).
			QStringList old_values = col->values ();
			QStringList new_values = old_values;
			for (int i = (new_keys.size () - new_values.size ()); i > 0; --i) new_values.append (QString ());

			// adjust all positions that have changed
			for (int pos = 0; pos < new_keys.size (); ++pos) {
				QMap<int, int>::const_iterator pit = position_changes.find (pos);
				if (pit != position_changes.constEnd ()) {	// some change
					int old_pos = pit.value ();
					if (old_pos < 0) {	// a new key
						new_values[pos] = getDefaultValue (column, pos);
						activate_row = pos;
					} else {	// old key changed position
						new_values[pos] = old_values[old_pos];
					}
				}
			}

			// strip excess length (if any), and apply
			new_values = new_values.mid (0, new_keys.size ());
			col->setValues (new_values);
			// NOTE: this will recurse into the current function, triggering an update of display and contents
		}

		columns_which_have_been_updated_externally.clear ();
		old_keys = new_keys;

		int nrows = new_keys.size ();
		row_count->setIntValue (nrows);
	} else {
		if (!columns_which_have_been_updated_externally.isEmpty ()) {	// add clearing timer for the first entry, only
			update_timer.start ();		 // NOTE: only has an effect, if column is neither restorable nor shown in the display. Otherwise, an update has already been triggered
		}
		columns_which_have_been_updated_externally.insert (target);
	}

	current_row->setIntValue (activate_row);
}

void RKOptionSet::updateContents () {
	RK_TRACE (PLUGIN);
	columns_which_have_been_updated_externally.clear ();

	RK_ASSERT (!updating_from_contents);
	RK_ASSERT (!updating_from_storage);
	updating_from_storage = true;

	int count = -1;
	int row = current_row->intValue ();
	QMap<RKComponentPropertyStringList *, ColumnInfo>::const_iterator it = column_map.constBegin ();
	if (it != column_map.constEnd ()) count = it.key ()->values ().size ();
	for (; it != column_map.constEnd (); ++it) {
		RKComponentPropertyStringList* col = it.key ();
		const ColumnInfo &ci = it.value ();
		if (!ci.restorable) continue;
		QString dummy;
		RKComponentBase *governor = contents_container->lookupComponent (ci.governor, &dummy);
		if (governor && governor->isProperty ()) {
			RK_ASSERT (dummy.isEmpty ());

			QString value;
			if (row >= 0) value = col->valueAt (row);
			else value = getDefaultValue (ci, row);

			static_cast<RKComponentPropertyBase*> (governor)->setValue (value);
		} else {
			RK_DO (qDebug ("Lookup error with trying to restore row %d of optionset: %s. Remainder: %s", row, qPrintable (ci.governor), qPrintable (dummy)), PLUGIN, DL_WARNING);
			RK_ASSERT (false);
		}
	}

	row_count->setIntValue (count);
	contents_container->enablednessProperty ()->setBoolValue (row >= 0);
	updateVisuals ();
	changed ();	// needed, for the unlikely case that no change notification was triggered above, since isValid() returns false while updating

	updating_from_storage = false;
}

void RKOptionSet::updateVisuals () {
	RK_TRACE (PLUGIN);

	if (!display) return;

	if (display_show_index) {
		for (int row = display->topLevelItemCount () - 1; row >= 0; --row) {
			display->topLevelItem (row)->setText (0, QString::number (row + 1));
		}
	}

	QPalette palette = display->header ()->palette ();
	if (!isSatisfied ()) {		// implies that it is enabled
		palette.setColor (QPalette::Window, QColor (255, 0, 0));
	} else {
		if (isEnabled ()) {
			palette.setColor (QPalette::Window, QColor (255, 255, 255));
		} else {
			palette.setColor (QPalette::Window, QColor (200, 200, 200));
		}
	}
	display->header ()->setPalette (palette);
}

void RKOptionSet::currentRowPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (property == current_row);
	if (display) {
		QTreeWidgetItem *item = 0;
		int row = current_row->intValue ();
		if (row >= 0) item = display->topLevelItem (row);
		if (item != display->currentItem ()) display->setCurrentItem (item);
	}
#warning: What if the current row is invalid. Should we refuse to switch? Or simply keep track of the fact? What if it is still processing?
	update_timer.start ();
}

void RKOptionSet::currentRowChanged (QTreeWidgetItem *new_row) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (display);
	current_row->setIntValue (display->indexOfTopLevelItem (new_row));
	// --> currentRowPropertyChanged ()
}

bool RKOptionSet::isValid () {
	if (update_timer.isActive ()) return (false);	// actually, this signifies "processing" state
	int count = row_count->intValue ();
	if (count < min_rows) return false;
	if ((count > 0) && (count < min_rows_if_any)) return false;
	if (count > max_rows) return false;
	return true;
}

#include "rkoptionset.moc"
